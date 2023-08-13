#include "Insight.hpp"

#include <QTimer>
#include <QFontDatabase>
#include <QJsonDocument>

#include "src/Services/PathService.hpp"
#include "src/Logger/Logger.hpp"
#include "src/EventManager/EventManager.hpp"
#include "UserInterfaces/InsightWindow/BloomProxyStyle.hpp"

#include "src/Application.hpp"

#include "InsightWorker/Tasks/GetTargetState.hpp"
#include "InsightWorker/Tasks/GetTargetDescriptor.hpp"

using namespace Exceptions;
using Targets::TargetState;

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
{
    Logger::info("Starting Insight");

    this->eventListener.registerCallbackForEventType<Events::TargetExecutionStopped>(
        std::bind(&Insight::onTargetStoppedEvent, this, std::placeholders::_1)
    );

    this->eventListener.registerCallbackForEventType<Events::TargetExecutionResumed>(
        std::bind(&Insight::onTargetResumedEvent, this, std::placeholders::_1)
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
    QApplication::setStyle(new BloomProxyStyle());

    qRegisterMetaType<Targets::TargetDescriptor>();
    qRegisterMetaType<Targets::TargetPinDescriptor>();
    qRegisterMetaType<Targets::TargetPinState>();
    qRegisterMetaType<Targets::TargetState>();
    qRegisterMetaType<std::map<int, Targets::TargetPinState>>();

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

    auto globalStylesheet = QFile(
        QString::fromStdString(
            Services::PathService::compiledResourcesPath() + "/src/Insight/UserInterfaces/InsightWindow/Stylesheets/Global.qss"
        )
    );

    if (!globalStylesheet.open(QFile::ReadOnly)) {
        throw Exception("Failed to open global stylesheet file");
    }

    this->globalStylesheet = globalStylesheet.readAll();

    // Construct and start worker threads
    for (std::uint8_t i = 0; i < Insight::INSIGHT_WORKER_COUNT; ++i) {
        auto* insightWorker = new InsightWorker();
        auto* workerThread = new QThread();

        workerThread->setObjectName("IW" + QString::number(insightWorker->id));
        insightWorker->moveToThread(workerThread);
        QObject::connect(workerThread, &QThread::started, insightWorker, &InsightWorker::startup);
        QObject::connect(workerThread, &QThread::finished, insightWorker, &QObject::deleteLater);
        QObject::connect(workerThread, &QThread::finished, workerThread, &QThread::deleteLater);

        this->insightWorkersById[insightWorker->id] = std::pair(insightWorker, workerThread);

        Logger::debug("Starting InsightWorker" + std::to_string(insightWorker->id));
        workerThread->start();
    }

    this->activateMainWindow();
}

void Insight::activateMainWindow() {
    if (this->mainWindow == nullptr) {
        this->mainWindow = new InsightWindow(
            this->environmentConfig,
            this->insightConfig,
            this->insightProjectSettings,
            this->targetDescriptor
        );

        this->mainWindow->setStyleSheet(this->globalStylesheet);

        QObject::connect(this->mainWindow, &QObject::destroyed, this, &Insight::onInsightWindowDestroyed);

        this->refreshTargetState();
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

void Insight::refreshTargetState() {
    const auto getTargetStateTask = QSharedPointer<GetTargetState>(new GetTargetState(), &QObject::deleteLater);
    QObject::connect(
        getTargetStateTask.get(),
        &GetTargetState::targetState,
        this,
        [this] (Targets::TargetState targetState) {
            this->lastTargetState = targetState;
            emit this->insightSignals->targetStateUpdated(this->lastTargetState);
        }
    );

    InsightWorker::queueTask(getTargetStateTask);
}

void Insight::onInsightWindowDestroyed() {
    this->mainWindow = nullptr;
    EventManager::triggerEvent(std::make_shared<Events::InsightMainWindowClosed>());
}

void Insight::onTargetStoppedEvent(const Events::TargetExecutionStopped& event) {
    if (this->lastTargetState == TargetState::STOPPED) {
        return;
    }

    this->lastTargetState = TargetState::STOPPED;

    if (this->targetStepping) {
        if (this->targetResumeTimer == nullptr) {
            this->targetResumeTimer = new QTimer(this);
            this->targetResumeTimer->setSingleShot(true);

            this->targetResumeTimer->callOnTimeout(this, [this] {
                if (this->lastTargetState != TargetState::STOPPED) {
                    return;
                }

                emit this->insightSignals->targetStateUpdated(TargetState::STOPPED);
            });
        }

        this->targetResumeTimer->start(1500);
        return;
    }

    if (this->targetResumeTimer != nullptr && this->targetResumeTimer->isActive()) {
        this->targetResumeTimer->stop();
    }

    emit this->insightSignals->targetStateUpdated(TargetState::STOPPED);
}

void Insight::onTargetResumedEvent(const Events::TargetExecutionResumed& event) {
    this->targetStepping = event.stepping;

    if (this->lastTargetState != TargetState::RUNNING) {
        this->lastTargetState = TargetState::RUNNING;
        emit this->insightSignals->targetStateUpdated(TargetState::RUNNING);
    }
}

void Insight::onTargetResetEvent(const Events::TargetReset& event) {
    try {
        if (this->lastTargetState != TargetState::STOPPED) {
            this->lastTargetState = TargetState::STOPPED;
            emit this->insightSignals->targetStateUpdated(TargetState::STOPPED);
        }

        emit this->insightSignals->targetReset();

    } catch (const Exceptions::Exception& exception) {
        Logger::debug("Error handling TargetReset event - " + exception.getMessage());
    }
}

void Insight::onTargetRegistersWrittenEvent(const Events::RegistersWrittenToTarget& event) {
    emit this->insightSignals->targetRegistersWritten(event.registers, event.createdTimestamp);
}

void Insight::onTargetMemoryWrittenEvent(const Events::MemoryWrittenToTarget& event) {
    emit this->insightSignals->targetMemoryWritten(
        event.memoryType,
        Targets::TargetMemoryAddressRange(event.startAddress, event.startAddress + (event.size - 1))
    );
}

void Insight::onProgrammingModeEnabledEvent(const Events::ProgrammingModeEnabled& event) {
    emit this->insightSignals->programmingModeEnabled();
}

void Insight::onProgrammingModeDisabledEvent(const Events::ProgrammingModeDisabled& event) {
    emit this->insightSignals->programmingModeDisabled();
}
