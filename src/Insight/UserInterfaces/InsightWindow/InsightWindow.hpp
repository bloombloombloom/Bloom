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

#include "Widgets/PanelWidget.hpp"

#include "Widgets/TargetWidgets/TargetPackageWidgetContainer.hpp"
#include "Widgets/TargetWidgets/TargetPackageWidget.hpp"
#include "Widgets/TargetRegistersPane/TargetRegistersPaneWidget.hpp"
#include "Widgets/TargetMemoryInspectionPane/TargetMemoryInspectionPane.hpp"
#include "AboutWindow.hpp"

namespace Bloom
{
    class InsightWindow: public QMainWindow
    {
        Q_OBJECT

    public:
        InsightWindow(InsightWorker& insightWorker);

        void setEnvironmentConfig(const EnvironmentConfig& environmentConfig) {
            this->environmentConfig = environmentConfig;
            this->targetConfig = environmentConfig.targetConfig;
        }

        void setInsightConfig(const InsightConfig& insightConfig) {
            this->insightConfig = insightConfig;
        }

        void init(Targets::TargetDescriptor targetDescriptor);

    public slots:
        void onTargetControllerSuspended();
        void onTargetControllerResumed(const Bloom::Targets::TargetDescriptor& targetDescriptor);
        void onTargetStateUpdate(Targets::TargetState newState);
        void onTargetProgramCounterUpdate(quint32 programCounter);
        void openReportIssuesUrl();
        void openGettingStartedUrl();
        void openAboutWindow();
        void toggleTargetRegistersPane();
        void toggleRamInspectionPane();
        void toggleEepromInspectionPane();

    signals:
        void refreshTargetPinStates(int variantId);

    protected:
        void resizeEvent(QResizeEvent* event) override;
        void showEvent(QShowEvent* event) override;

    private:
        InsightConfig insightConfig;
        EnvironmentConfig environmentConfig;
        TargetConfig targetConfig;

        InsightWorker& insightWorker;

        bool activated = false;

        Targets::TargetDescriptor targetDescriptor;
        Targets::TargetState targetState = Targets::TargetState::UNKNOWN;

        QWidget* windowContainer = nullptr;
        QMenuBar* mainMenuBar = nullptr;
        QWidget* layoutContainer = nullptr;
        QWidget* container = nullptr;
        QMenu* variantMenu = nullptr;
        AboutWindow* aboutWindowWidget = nullptr;

        QWidget* header = nullptr;
        QToolButton* refreshIoInspectionButton = nullptr;

        QWidget* leftMenuBar = nullptr;
        Widgets::PanelWidget* leftPanel = nullptr;
        Widgets::TargetRegistersPaneWidget* targetRegistersSidePane = nullptr;
        QToolButton* targetRegistersButton = nullptr;

        QLabel* ioUnavailableWidget = nullptr;
        Widgets::InsightTargetWidgets::TargetPackageWidgetContainer* ioContainerWidget = nullptr;
        Widgets::InsightTargetWidgets::TargetPackageWidget* targetPackageWidget = nullptr;

        QWidget* bottomMenuBar = nullptr;
        Widgets::PanelWidget* bottomPanel = nullptr;
        Widgets::TargetMemoryInspectionPane* ramInspectionPane = nullptr;
        Widgets::TargetMemoryInspectionPane* eepromInspectionPane = nullptr;
        QToolButton* ramInspectionButton = nullptr;
        QToolButton* eepromInspectionButton = nullptr;

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

        void adjustPanels();
    };
}
