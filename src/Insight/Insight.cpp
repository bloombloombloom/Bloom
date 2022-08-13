#include "Insight.hpp"

#include <QTimer>
#include <QFontDatabase>

#include "src/Helpers/Paths.hpp"
#include "src/Logger/Logger.hpp"
#include "UserInterfaces/InsightWindow/BloomProxyStyle.hpp"

#include "src/Application.hpp"
#include "InsightWorker/Tasks/QueryLatestVersionNumber.hpp"

namespace Bloom
{
    using namespace Bloom::Exceptions;

    Insight::Insight(
        EventListener& eventListener,
        const ProjectConfig& projectConfig,
        const EnvironmentConfig& environmentConfig,
        const InsightConfig& insightConfig,
        InsightProjectSettings& insightProjectSettings
    )
        : eventListener(eventListener)
        , projectConfig(projectConfig)
        , environmentConfig(environmentConfig)
        , insightConfig(insightConfig)
        , insightProjectSettings(insightProjectSettings)
        , application(
            (
                QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true),
#ifndef BLOOM_DEBUG_BUILD
                QCoreApplication::addLibraryPath(QString::fromStdString(Paths::applicationDirPath() + "/plugins")),
#endif
                QApplication(this->qtApplicationArgc, this->qtApplicationArgv.data())
            )
        )
    {}

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

        auto targetDescriptor = this->targetControllerConsole.getTargetDescriptor();

        QApplication::setQuitOnLastWindowClosed(true);
        QApplication::setStyle(new BloomProxyStyle());

        auto globalStylesheet = QFile(
            QString::fromStdString(
                Paths::compiledResourcesPath() + "/src/Insight/UserInterfaces/InsightWindow/Stylesheets/Global.qss"
            )
        );

        if (!globalStylesheet.open(QFile::ReadOnly)) {
            throw Exception("Failed to open global stylesheet file");
        }

        this->application.setStyleSheet(globalStylesheet.readAll());

        qRegisterMetaType<Bloom::Targets::TargetDescriptor>();
        qRegisterMetaType<Bloom::Targets::TargetPinDescriptor>();
        qRegisterMetaType<Bloom::Targets::TargetPinState>();
        qRegisterMetaType<Bloom::Targets::TargetState>();
        qRegisterMetaType<std::map<int, Bloom::Targets::TargetPinState>>();

        // Load Ubuntu fonts
        QFontDatabase::addApplicationFont(
            QString::fromStdString(Paths::resourcesDirPath() + "/Fonts/Ubuntu/Ubuntu-B.ttf")
        );
        QFontDatabase::addApplicationFont(
            QString::fromStdString(Paths::resourcesDirPath() + "/Fonts/Ubuntu/Ubuntu-BI.ttf")
        );
        QFontDatabase::addApplicationFont(
            QString::fromStdString(Paths::resourcesDirPath() + "/Fonts/Ubuntu/Ubuntu-C.ttf")
        );
        QFontDatabase::addApplicationFont(
            QString::fromStdString(Paths::resourcesDirPath() + "/Fonts/Ubuntu/Ubuntu-L.ttf")
        );
        QFontDatabase::addApplicationFont(
            QString::fromStdString(Paths::resourcesDirPath() + "/Fonts/Ubuntu/Ubuntu-LI.ttf")
        );
        QFontDatabase::addApplicationFont(
            QString::fromStdString(Paths::resourcesDirPath() + "/Fonts/Ubuntu/Ubuntu-M.ttf")
        );
        QFontDatabase::addApplicationFont(
            QString::fromStdString(Paths::resourcesDirPath() + "/Fonts/Ubuntu/Ubuntu-MI.ttf")
        );
        QFontDatabase::addApplicationFont(
            QString::fromStdString(Paths::resourcesDirPath() + "/Fonts/Ubuntu/UbuntuMono-B.ttf")
        );
        QFontDatabase::addApplicationFont(
            QString::fromStdString(Paths::resourcesDirPath() + "/Fonts/Ubuntu/UbuntuMono-BI.ttf")
        );
        QFontDatabase::addApplicationFont(
            QString::fromStdString(Paths::resourcesDirPath() + "/Fonts/Ubuntu/UbuntuMono-R.ttf")
        );
        QFontDatabase::addApplicationFont(
            QString::fromStdString(Paths::resourcesDirPath() + "/Fonts/Ubuntu/UbuntuMono-RI.ttf")
        );
        QFontDatabase::addApplicationFont(
            QString::fromStdString(Paths::resourcesDirPath() + "/Fonts/Ubuntu/Ubuntu-R.ttf")
        );
        QFontDatabase::addApplicationFont(
            QString::fromStdString(Paths::resourcesDirPath() + "/Fonts/Ubuntu/Ubuntu-RI.ttf")
        );
        QFontDatabase::addApplicationFont(
            QString::fromStdString(Paths::resourcesDirPath() + "/Fonts/Ubuntu/Ubuntu-Th.ttf")
        );

        /*
         * We can't run our own event loop here - we have to use Qt's event loop. But we still need to be able to
         * process our events. To address this, we use a QTimer to dispatch our events on an interval.
         *
         * This allows us to use Qt's event loop whilst still being able to process our own events.
         */
        auto* eventDispatchTimer = new QTimer(&(this->application));
        QObject::connect(eventDispatchTimer, &QTimer::timeout, this, &Insight::dispatchEvents);
        eventDispatchTimer->start(100);

        QObject::connect(
            this->mainWindow,
            &InsightWindow::activatedSignal,
            this->insightWorker,
            &InsightWorker::onInsightWindowActivated
        );

        this->mainWindow->setInsightConfig(this->insightConfig);
        this->mainWindow->setEnvironmentConfig(this->environmentConfig);

        // Prepare worker thread
        this->workerThread = new QThread();
        this->workerThread->setObjectName("IW");
        this->insightWorker->moveToThread(this->workerThread);
        QObject::connect(this->workerThread, &QThread::started, this->insightWorker, &InsightWorker::startup);
        QObject::connect(this->workerThread, &QThread::finished, this->insightWorker, &QObject::deleteLater);
        QObject::connect(this->workerThread, &QThread::finished, this->workerThread, &QThread::deleteLater);

        this->mainWindow->init(targetDescriptor);

        QObject::connect(this->insightWorker, &InsightWorker::ready, this, [this] {
            this->checkBloomVersion();
        });

        this->mainWindow->show();
    }

    void Insight::shutdown() {
        if (this->getThreadState() == ThreadState::STOPPED) {
            return;
        }

        Logger::info("Shutting down Insight");

        this->mainWindow->close();

        if (this->workerThread != nullptr && this->workerThread->isRunning()) {
            Logger::debug("Stopping InsightWorker thread");
            this->workerThread->quit();
            Logger::debug("Waiting for InsightWorker thread to stop");
            this->workerThread->wait();
        }

        this->application.exit(0);
        this->setThreadState(ThreadState::STOPPED);
    }

    void Insight::checkBloomVersion() {
        auto currentVersionNumber = Application::VERSION;

        auto* versionQueryTask = new QueryLatestVersionNumber(
            currentVersionNumber
        );

        QObject::connect(
            versionQueryTask,
            &QueryLatestVersionNumber::latestVersionNumberRetrieved,
            this,
            [this, currentVersionNumber] (const VersionNumber& latestVersionNumber) {
                if (latestVersionNumber > currentVersionNumber) {
                    Logger::warning(
                        "Bloom v" + latestVersionNumber.toString()
                            + " is available to download - upgrade via " + Paths::homeDomainName()
                    );
                }
            }
        );

        this->insightWorker->queueTask(versionQueryTask);
    }
}
