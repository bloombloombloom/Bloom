#include "Insight.hpp"

#include <typeindex>
#include <QTimer>

#include "InsightWorker.hpp"
#include "src/Helpers/Paths.hpp"
#include "src/Logger/Logger.hpp"
#include "src/Exceptions/InvalidConfig.hpp"
#include "src/Targets/TargetState.hpp"

using namespace Bloom;
using namespace Bloom::Exceptions;

void Insight::run() {
    try {
        this->startup();

        this->workerThread->start();
        this->setThreadState(ThreadState::READY);
        Logger::info("Insight ready");
        this->application.exec();

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
    this->setThreadState(ThreadState::STARTING);
    this->eventManager.registerListener(this->eventListener);

    this->eventListener->registerCallbackForEventType<Events::ShutdownApplication>(
        std::bind(&Insight::onShutdownApplicationEvent, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<Events::TargetControllerThreadStateChanged>(
        std::bind(&Insight::onTargetControllerThreadStateChangedEvent, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<Events::DebugServerThreadStateChanged>(
        std::bind(&Insight::onDebugServerThreadStateChangedEvent, this, std::placeholders::_1)
    );

    auto targetDescriptor = this->targetControllerConsole.getTargetDescriptor();

#ifndef BLOOM_DEBUG_BUILD
    QCoreApplication::addLibraryPath(QString::fromStdString(Paths::applicationDirPath() + "/plugins"));
#endif
    this->application.setQuitOnLastWindowClosed(true);
    qRegisterMetaType<Bloom::Targets::TargetDescriptor>();
    qRegisterMetaType<Bloom::Targets::TargetPinDescriptor>();
    qRegisterMetaType<Bloom::Targets::TargetPinState>();
    qRegisterMetaType<Bloom::Targets::TargetState>();
    qRegisterMetaType<std::map<int, Bloom::Targets::TargetPinState>>();

    this->mainWindow.setInsightConfig(this->insightConfig);
    this->mainWindow.setEnvironmentConfig(this->environmentConfig);

    this->mainWindow.init(
        this->application,
        targetDescriptor
    );

    this->mainWindow.show();

    /*
     * We can't run our own event loop here - we have to use Qt's event loop. But we still need to be able to
     * process our events. To address this, we use a QTimer to dispatch our events on an interval.
     *
     * This allows us to use Qt's event loop whilst still being able to process our own events.
     */
    auto eventDispatchTimer = new QTimer(&(this->application));
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

    connect(worker, &InsightWorker::targetControllerSuspended, &(this->mainWindow), &InsightWindow::onTargetControllerSuspended);
    connect(worker, &InsightWorker::targetControllerResumed, &(this->mainWindow), &InsightWindow::onTargetControllerResumed);
    connect(worker, &InsightWorker::targetStateUpdated, &(this->mainWindow), &InsightWindow::onTargetStateUpdate);
    connect(worker, &InsightWorker::targetProgramCounterUpdated, &(this->mainWindow), &InsightWindow::onTargetProgramCounterUpdate);
    connect(worker, &InsightWorker::targetPinStatesUpdated, &(this->mainWindow), &InsightWindow::onTargetPinStatesUpdate);
    connect(worker, &InsightWorker::targetIoPortsUpdated, &(this->mainWindow), &InsightWindow::onTargetIoPortsUpdate);
    connect(&(this->mainWindow), &InsightWindow::refreshTargetPinStates, worker, &InsightWorker::requestPinStates);
    connect(&(this->mainWindow), &InsightWindow::setTargetPinState, worker, &InsightWorker::requestPinStateUpdate);
}

void Insight::shutdown() {
    if (this->getThreadState() == ThreadState::STOPPED) {
        return;
    }

    Logger::info("Shutting down Insight");
    this->mainWindow.close();

    if (this->workerThread != nullptr && this->workerThread->isRunning()) {
        this->workerThread->quit();
    }

    this->application.exit(0);

    this->setThreadState(ThreadState::STOPPED);
}

void Insight::onShutdownApplicationEvent(const Events::ShutdownApplication&) {
    /*
     * Once Insight shuts down, control of the main thread will be returned to Application::run(), which
     * will pickup the ShutdownApplication event and proceed with the shutdown.
     */
    this->shutdown();
}

void Insight::onTargetControllerThreadStateChangedEvent(const Events::TargetControllerThreadStateChanged& event) {
    if (event.getState() == ThreadState::STOPPED) {
        // Something horrible has happened with the TargetController - Insight is useless without the TargetController
        this->shutdown();
    }
}

void Insight::onDebugServerThreadStateChangedEvent(const Events::DebugServerThreadStateChanged& event) {
    if (event.getState() == ThreadState::STOPPED) {
        // Something horrible has happened with the DebugServer
        this->shutdown();
    }
}
