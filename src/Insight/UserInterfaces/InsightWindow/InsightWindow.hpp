#pragma once

#include <QtCore>
#include <QMainWindow>
#include <QtUiTools/QtUiTools>
#include <memory>
#include <optional>

#include "src/ApplicationConfig.hpp"
#include "src/Insight/InsightWorker/InsightWorker.hpp"
#include "src/Targets/TargetState.hpp"

#include "src/Targets/TargetDescriptor.hpp"
#include "src/Targets/TargetVariant.hpp"

#include "Widgets/TargetWidgets/TargetPackageWidget.hpp"
#include "AboutWindow.hpp"

namespace Bloom
{
    class InsightWindow: public QObject
    {
    Q_OBJECT
    private:
        InsightConfig insightConfig;
        EnvironmentConfig environmentConfig;
        TargetConfig targetConfig;

        InsightWorker& insightWorker;

        bool activated = false;

        Targets::TargetDescriptor targetDescriptor;
        Targets::TargetState targetState = Targets::TargetState::UNKNOWN;

        QWidget* mainWindowWidget = nullptr;
        AboutWindow* aboutWindowWidget = nullptr;
        QMenuBar* mainMenuBar = nullptr;
        QMenu* variantMenu = nullptr;

        QWidget* header = nullptr;
        QToolButton* refreshIoInspectionButton = nullptr;

        QWidget* ioContainerWidget = nullptr;
        QLabel* ioUnavailableWidget = nullptr;
        Widgets::InsightTargetWidgets::TargetPackageWidget* targetPackageWidget = nullptr;

        QWidget* footer = nullptr;
        QLabel* targetStatusLabel = nullptr;
        QLabel* programCounterValueLabel = nullptr;

        std::map<QString, Targets::TargetVariant> supportedVariantsByName;
        const Targets::TargetVariant* selectedVariant = nullptr;
        bool uiDisabled = false;

        static bool isVariantSupported(const Targets::TargetVariant& variant);

        void selectVariant(const Targets::TargetVariant* variant);

        void toggleUi(bool disable);
        void activate();
        void deactivate();

    public:
        InsightWindow(QApplication& application, InsightWorker& insightWorker);

        void setEnvironmentConfig(const EnvironmentConfig& environmentConfig) {
            this->environmentConfig = environmentConfig;
            this->targetConfig = environmentConfig.targetConfig;
        }

        void setInsightConfig(const InsightConfig& insightConfig) {
            this->insightConfig = insightConfig;
        }

        void init(Targets::TargetDescriptor targetDescriptor);

        void show();

    public slots:
        void onTargetControllerSuspended();
        void onTargetControllerResumed(const Bloom::Targets::TargetDescriptor& targetDescriptor);
        void onTargetStateUpdate(Targets::TargetState newState);
        void onTargetProgramCounterUpdate(quint32 programCounter);
        void onTargetIoPortsUpdate();
        void close();
        void openReportIssuesUrl();
        static void openGettingStartedUrl();
        void openAboutWindow();
        void togglePinIoState(Widgets::InsightTargetWidgets::TargetPinWidget* pinWidget);

    signals:
        void refreshTargetPinStates(int variantId);
        void setTargetPinState(
            int variantId,
            Bloom::Targets::TargetPinDescriptor pinDescriptor,
            Bloom::Targets::TargetPinState pinState
        );
    };
}
