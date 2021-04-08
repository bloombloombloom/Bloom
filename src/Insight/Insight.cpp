#include <thread>
#include <typeindex>
#include <QTimer>

#include "Insight.hpp"
#include "InsightWorker.hpp"
#include "src/Application.hpp"
#include "src/Logger/Logger.hpp"
#include "src/Exceptions/InvalidConfig.hpp"
#include "src/Targets/TargetState.hpp"

using namespace Bloom;
using namespace Exceptions;

void Insight::run() {
    try {
        this->startup();

        this->workerThread->start();
        this->setState(ThreadState::READY);
        Logger::info("Insight ready");
        this->application->exec();

    } catch (const Exception& exception) {
        Logger::error("Insight encountered a fatal error. See below for errors:");
        Logger::error(exception.getMessage());

    } catch (const std::exception& exception) {
        Logger::error("Insight encountered a fatal error. See below for errors:");
        Logger::error(std::string(exception.what()));
    }

    this->shutdown();
}

void Insight::startup() {
    Logger::info("Starting Insight");
    this->setState(ThreadState::STARTING);
    this->eventManager.registerListener(this->eventListener);

    this->eventListener->registerCallbackForEventType<ShutdownApplication>(
        std::bind(&Insight::onShutdownApplicationEvent, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<TargetControllerStateChanged>(
        std::bind(&Insight::onTargetControllerStateChangedEvent, this, std::placeholders::_1)
    );

    auto targetDescriptor = this->getTargetDescriptor();

    std::string qtAppName = "Bloom";
    char* appArguments[] = {qtAppName.data()};
    auto appArgCount = 1;
//        QCoreApplication::addLibraryPath(QString::fromStdString(Application::getApplicationDirPath() + "/plugins"));
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);

    this->application = new QApplication(appArgCount, appArguments);
    this->application->setQuitOnLastWindowClosed(true);
    qRegisterMetaType<Bloom::Targets::TargetPinDescriptor>();
    qRegisterMetaType<Bloom::Targets::TargetPinState>();
    qRegisterMetaType<Bloom::Targets::TargetState>();
    qRegisterMetaType<std::map<int, Bloom::Targets::TargetPinState>>();

    this->mainWindow.init(*(this->application), targetDescriptor, this->insightConfig, this->environmentConfig.targetConfig);
    this->mainWindow.show();

    /*
     * We can't run our own event loop here - we have to use Qt's event loop. But we still need to be able to
     * process our events. To address this, we use a QTimer to dispatch our events on an interval.
     *
     * This allows us to use Qt's event loop whilst still being able to process our own events.
     */
    auto eventDispatchTimer = new QTimer(this->application);
    connect(eventDispatchTimer, &QTimer::timeout, this, &Insight::dispatchEvents);
    eventDispatchTimer->start(100);

    // Prepare worker thread
    auto worker = new InsightWorker(this->eventManager);
    this->workerThread = new QThread();
    this->workerThread->setObjectName("IW");
    worker->moveToThread(this->workerThread);
    connect(this->workerThread, &QThread::started, worker, &InsightWorker::startup);
    connect(this->workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this->workerThread, &QThread::finished, this->workerThread, &QThread::deleteLater);

    connect(worker, &InsightWorker::targetStateUpdated, &(this->mainWindow), &InsightWindow::onTargetStateUpdate);
    connect(worker, &InsightWorker::targetProgramCounterUpdated, &(this->mainWindow), &InsightWindow::onTargetProgramCounterUpdate);
    connect(worker, &InsightWorker::targetPinStatesUpdated, &(this->mainWindow), &InsightWindow::onTargetPinStatesUpdate);
    connect(worker, &InsightWorker::targetIoPortsUpdated, &(this->mainWindow), &InsightWindow::onTargetIoPortsUpdate);
    connect(&(this->mainWindow), &InsightWindow::refreshTargetPinStates, worker, &InsightWorker::requestPinStates);
    connect(&(this->mainWindow), &InsightWindow::setTargetPinState, worker, &InsightWorker::requestPinStateUpdate);
}

Targets::TargetDescriptor Insight::getTargetDescriptor() {
    auto extractEvent = std::make_shared<Events::ExtractTargetDescriptor>();
    this->eventManager.triggerEvent(extractEvent);
    auto responseEvent = this->eventListener->waitForEvent<
        Events::TargetDescriptorExtracted,
        Events::TargetControllerErrorOccurred
    >(std::chrono::milliseconds(5000), extractEvent->id);

    if (!responseEvent.has_value()
        || !std::holds_alternative<EventPointer<Events::TargetDescriptorExtracted>>(responseEvent.value())
        ) {
        throw Exception("Unexpected response from TargetController");
    }

    auto descriptorExtracted = std::get<EventPointer<Events::TargetDescriptorExtracted>>(responseEvent.value());
    return descriptorExtracted->targetDescriptor;
}

void Insight::shutdown() {
    if (this->getState() == ThreadState::STOPPED) {
        return;
    }

    Logger::info("Shutting down Insight");
    this->mainWindow.close();

    if (this->workerThread != nullptr && this->workerThread->isRunning()) {
        this->workerThread->quit();
    }

    if (this->application != nullptr) {
        this->application->exit(0);
    }

    this->setState(ThreadState::STOPPED);
}

void Insight::onShutdownApplicationEvent(EventPointer<ShutdownApplication>) {
    /*
     * Once Insight shuts down, control of the main thread will be returned to Application::run(), which
     * will pickup the ShutdownApplication event and proceed with the shutdown.
     */
    this->shutdown();
}

void Insight::onTargetControllerStateChangedEvent(EventPointer<TargetControllerStateChanged> event) {
    if (event->getState() == ThreadState::STOPPED) {
        // Something horrible has happened with the TargetController - Insight is useless without the TargetController
        this->shutdown();
    }
}
