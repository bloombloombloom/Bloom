#include "Insight.hpp"

#include <QTimer>
#include <QFontDatabase>
#include <QJsonDocument>

#include "src/Services/PathService.hpp"
#include "src/EventManager/EventManager.hpp"
#include "UserInterfaces/InsightWindow/BloomProxyStyle.hpp"
#include "src/Logger/Logger.hpp"

#include "src/Application.hpp"

using namespace Exceptions;
using Targets::TargetExecutionState;

Insight::Insight(
    EventListener& eventListener,
    const ProjectConfig& projectConfig,
    const EnvironmentConfig& environmentConfig,
    const InsightConfig& insightConfig,
    InsightProjectSettings& insightProjectSettings,
    QApplication* parent
)
    : QObject(parent)
    , eventListener(eventListener)
    , projectConfig(projectConfig)
    , environmentConfig(environmentConfig)
    , insightConfig(insightConfig)
    , insightProjectSettings(insightProjectSettings)
    , targetDescriptor(this->targetControllerService.getTargetDescriptor())
    , targetState(this->targetControllerService.getTargetState())
{
    Logger::info("Starting Insight");

    this->eventListener.registerCallbackForEventType<Events::TargetStateChanged>(
        std::bind(&Insight::onTargetStateChangedEvent, this, std::placeholders::_1)
    );

    this->eventListener.registerCallbackForEventType<Events::TargetReset>(
        std::bind(&Insight::onTargetResetEvent, this, std::placeholders::_1)
    );

    this->eventListener.registerCallbackForEventType<Events::RegistersWrittenToTarget>(
        std::bind(&Insight::onTargetRegistersWrittenEvent, this, std::placeholders::_1)
    );

    this->eventListener.registerCallbackForEventType<Events::MemoryWrittenToTarget>(
        std::bind(&Insight::onTargetMemoryWrittenEvent, this, std::placeholders::_1)
    );

    this->eventListener.registerCallbackForEventType<Events::ProgrammingModeEnabled>(
        std::bind(&Insight::onProgrammingModeEnabledEvent, this, std::placeholders::_1)
    );

    this->eventListener.registerCallbackForEventType<Events::ProgrammingModeDisabled>(
        std::bind(&Insight::onProgrammingModeDisabledEvent, this, std::placeholders::_1)
    );

    QApplication::setQuitOnLastWindowClosed(false);
    QApplication::setStyle(new BloomProxyStyle{});

    qRegisterMetaType<Targets::TargetDescriptor>();
    qRegisterMetaType<Targets::TargetPinDescriptor>();
    qRegisterMetaType<Targets::TargetState>();

    // Load Ubuntu fonts
    QFontDatabase::addApplicationFont(
        QString::fromStdString(Services::PathService::resourcesDirPath() + "/fonts/Ubuntu/Ubuntu-B.ttf")
    );
    QFontDatabase::addApplicationFont(
        QString::fromStdString(Services::PathService::resourcesDirPath() + "/fonts/Ubuntu/Ubuntu-BI.ttf")
    );
    QFontDatabase::addApplicationFont(
        QString::fromStdString(Services::PathService::resourcesDirPath() + "/fonts/Ubuntu/Ubuntu-C.ttf")
    );
    QFontDatabase::addApplicationFont(
        QString::fromStdString(Services::PathService::resourcesDirPath() + "/fonts/Ubuntu/Ubuntu-L.ttf")
    );
    QFontDatabase::addApplicationFont(
        QString::fromStdString(Services::PathService::resourcesDirPath() + "/fonts/Ubuntu/Ubuntu-LI.ttf")
    );
    QFontDatabase::addApplicationFont(
        QString::fromStdString(Services::PathService::resourcesDirPath() + "/fonts/Ubuntu/Ubuntu-M.ttf")
    );
    QFontDatabase::addApplicationFont(
        QString::fromStdString(Services::PathService::resourcesDirPath() + "/fonts/Ubuntu/Ubuntu-MI.ttf")
    );
    QFontDatabase::addApplicationFont(
        QString::fromStdString(Services::PathService::resourcesDirPath() + "/fonts/Ubuntu/UbuntuMono-B.ttf")
    );
    QFontDatabase::addApplicationFont(
        QString::fromStdString(Services::PathService::resourcesDirPath() + "/fonts/Ubuntu/UbuntuMono-BI.ttf")
    );
    QFontDatabase::addApplicationFont(
        QString::fromStdString(Services::PathService::resourcesDirPath() + "/fonts/Ubuntu/UbuntuMono-R.ttf")
    );
    QFontDatabase::addApplicationFont(
        QString::fromStdString(Services::PathService::resourcesDirPath() + "/fonts/Ubuntu/UbuntuMono-RI.ttf")
    );
    QFontDatabase::addApplicationFont(
        QString::fromStdString(Services::PathService::resourcesDirPath() + "/fonts/Ubuntu/Ubuntu-R.ttf")
    );
    QFontDatabase::addApplicationFont(
        QString::fromStdString(Services::PathService::resourcesDirPath() + "/fonts/Ubuntu/Ubuntu-RI.ttf")
    );
    QFontDatabase::addApplicationFont(
        QString::fromStdString(Services::PathService::resourcesDirPath() + "/fonts/Ubuntu/Ubuntu-Th.ttf")
    );

    auto globalStylesheet = QFile{
        QString::fromStdString(
            Services::PathService::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Stylesheets/Global.qss"
        )
    };

    if (!globalStylesheet.open(QFile::ReadOnly)) {
        throw Exception{"Failed to open global stylesheet file"};
    }

    this->globalStylesheet = globalStylesheet.readAll();

    // Construct and start worker threads
    for (auto i = std::uint8_t{0}; i < Insight::INSIGHT_WORKER_COUNT; ++i) {
        auto* insightWorker = new InsightWorker{};
        auto* workerThread = new QThread{};

        workerThread->setObjectName("IW" + QString::number(insightWorker->id));
        insightWorker->moveToThread(workerThread);
        QObject::connect(workerThread, &QThread::started, insightWorker, &InsightWorker::startup);
        QObject::connect(workerThread, &QThread::finished, insightWorker, &QObject::deleteLater);
        QObject::connect(workerThread, &QThread::finished, workerThread, &QThread::deleteLater);

        this->insightWorkersById[insightWorker->id] = std::pair{insightWorker, workerThread};

        Logger::debug("Starting InsightWorker" + std::to_string(insightWorker->id));
        workerThread->start();
    }

    this->activateMainWindow();
}

void Insight::activateMainWindow() {
    if (this->mainWindow == nullptr) {
        this->mainWindow = new InsightWindow{
            this->insightProjectSettings,
            this->insightConfig,
            this->environmentConfig,
            this->targetDescriptor,
            this->targetState
        };

        this->mainWindow->setStyleSheet(this->globalStylesheet);
        QObject::connect(this->mainWindow, &QObject::destroyed, this, &Insight::onInsightWindowDestroyed);
    }

    this->mainWindow->show();
    this->mainWindow->activateWindow();
}

void Insight::shutdown() {
    Logger::info("Shutting down Insight");

    if (this->mainWindow != nullptr) {
        this->mainWindow->close();
    }

    for (auto& [workerId, workerPair] : this->insightWorkersById) {
        auto* workerThread = workerPair.second;

        if (workerThread != nullptr && workerThread->isRunning()) {
            Logger::debug("Stopping InsightWorker" + std::to_string(workerId));
            workerThread->quit();
            Logger::debug("Waiting for InsightWorker" + std::to_string(workerId) + " to stop");
            workerThread->wait();
        }
    }
}

void Insight::onInsightWindowDestroyed() {
    this->mainWindow = nullptr;
    EventManager::triggerEvent(std::make_shared<Events::InsightMainWindowClosed>());
}

void Insight::onTargetStateChangedEvent(const Events::TargetStateChanged& event) {
    if (event.previousState.mode != event.newState.mode) {
        if (event.newState.mode == Targets::TargetMode::PROGRAMMING) {
            emit this->insightSignals->programmingModeEnabled();
        }

        if (event.newState.mode == Targets::TargetMode::DEBUGGING) {
            emit this->insightSignals->programmingModeDisabled();
        }
    }

    if (event.previousState.executionState == event.newState.executionState) {
        return;
    }

    emit this->insightSignals->targetStateUpdated(event.newState, event.previousState);
}

void Insight::onTargetResetEvent(const Events::TargetReset& event) {
    try {
        if (this->targetState.executionState != TargetExecutionState::STOPPED) {
            // Reset event came in too late, target has already resumed execution. Ignore
            return;
        }

        emit this->insightSignals->targetReset();

    } catch (const Exception& exception) {
        Logger::debug("Error handling TargetReset event - " + exception.getMessage());
    }
}

void Insight::onTargetRegistersWrittenEvent(const Events::RegistersWrittenToTarget& event) {
    emit this->insightSignals->targetRegistersWritten(event.registers, event.createdTimestamp);
}

void Insight::onTargetMemoryWrittenEvent(const Events::MemoryWrittenToTarget& event) {
    emit this->insightSignals->targetMemoryWritten(
        event.addressSpaceDescriptor,
        event.memorySegmentDescriptor,
        Targets::TargetMemoryAddressRange{event.startAddress, event.startAddress + (event.size - 1)}
    );
}

void Insight::onProgrammingModeEnabledEvent(const Events::ProgrammingModeEnabled& event) {
    emit this->insightSignals->programmingModeEnabled();
}

void Insight::onProgrammingModeDisabledEvent(const Events::ProgrammingModeDisabled& event) {
    emit this->insightSignals->programmingModeDisabled();
}
