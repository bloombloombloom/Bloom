#pragma once

#include <QtCore>
#include <QMainWindow>
#include <QEvent>
#include <memory>
#include <optional>

#include "src/ProjectSettings.hpp"
#include "src/ProjectConfig.hpp"
#include "src/Targets/TargetState.hpp"

#include "src/Targets/TargetDescriptor.hpp"
#include "src/Targets/TargetVariant.hpp"

#include "Widgets/PanelWidget.hpp"

#include "Widgets/Label.hpp"
#include "Widgets/SvgToolButton.hpp"
#include "Widgets/TargetWidgets/TargetPackageWidgetContainer.hpp"
#include "Widgets/TargetWidgets/TargetPackageWidget.hpp"
#include "Widgets/TargetRegistersPane/TargetRegistersPaneWidget.hpp"
#include "Widgets/TargetMemoryInspectionPane/TargetMemoryInspectionPane.hpp"
#include "Widgets/TargetMemoryInspectionPane/TargetMemoryInspectionPaneSettings.hpp"
#include "AboutWindow.hpp"

namespace Bloom
{
    class InsightWindow: public QMainWindow
    {
        Q_OBJECT

    public:
        InsightWindow(
            const EnvironmentConfig& environmentConfig,
            const InsightConfig& insightConfig,
            InsightProjectSettings& insightProjectSettings
        );

        void setEnvironmentConfig(const EnvironmentConfig& environmentConfig) {
            this->environmentConfig = environmentConfig;
            this->targetConfig = environmentConfig.targetConfig;
        }

        void setInsightConfig(const InsightConfig& insightConfig) {
            this->insightConfig = insightConfig;
        }

        void init(Targets::TargetDescriptor targetDescriptor);

    signals:
        void activatedSignal();

    protected:
        void resizeEvent(QResizeEvent* event) override;
        void showEvent(QShowEvent* event) override;
        void closeEvent(QCloseEvent* event) override;

    private:
        InsightProjectSettings& insightProjectSettings;

        InsightConfig insightConfig;
        EnvironmentConfig environmentConfig;
        TargetConfig targetConfig;

        bool activated = false;

        Targets::TargetDescriptor targetDescriptor;
        Targets::TargetState targetState = Targets::TargetState::UNKNOWN;

        QWidget* windowContainer = nullptr;
        QMenuBar* mainMenuBar = nullptr;
        QWidget* layoutContainer = nullptr;
        QWidget* container = nullptr;
        QMenu* variantMenu = nullptr;
        Widgets::Label* targetNameLabel = nullptr;
        Widgets::Label* targetIdLabel = nullptr;
        AboutWindow* aboutWindowWidget = nullptr;

        QWidget* header = nullptr;
        Widgets::SvgToolButton* refreshIoInspectionButton = nullptr;

        QWidget* leftMenuBar = nullptr;
        Widgets::PanelWidget* leftPanel = nullptr;
        Widgets::TargetRegistersPaneWidget* targetRegistersSidePane = nullptr;
        QToolButton* targetRegistersButton = nullptr;

        Widgets::Label* ioUnavailableWidget = nullptr;
        Widgets::InsightTargetWidgets::TargetPackageWidgetContainer* ioContainerWidget = nullptr;
        Widgets::InsightTargetWidgets::TargetPackageWidget* targetPackageWidget = nullptr;

        QWidget* bottomMenuBar = nullptr;
        Widgets::PanelWidget* bottomPanel = nullptr;
        Widgets::TargetMemoryInspectionPane* ramInspectionPane = nullptr;
        Widgets::TargetMemoryInspectionPane* eepromInspectionPane = nullptr;
        std::map<
            Targets::TargetMemoryType,
            Widgets::TargetMemoryInspectionPaneSettings
        > memoryInspectionPaneSettingsByMemoryType;
        QToolButton* ramInspectionButton = nullptr;
        QToolButton* eepromInspectionButton = nullptr;

        QWidget* footer = nullptr;
        Widgets::Label* targetStatusLabel = nullptr;
        Widgets::Label* programCounterValueLabel = nullptr;

        std::map<QString, Targets::TargetVariant> supportedVariantsByName;
        const Targets::TargetVariant* selectedVariant = nullptr;
        std::optional<Targets::TargetVariant> previouslySelectedVariant;
        bool uiDisabled = false;

        static bool isVariantSupported(const Targets::TargetVariant& variant);

        void setUiDisabled(bool disable);

        void activate();
        void populateVariantMenu();
        void selectDefaultVariant();
        void selectVariant(const Targets::TargetVariant* variant);
        void createPanes();
        void destroyPanes();
        void deactivate();

        void adjustPanels();
        void adjustMinimumSize();

        void onTargetControllerSuspended();
        void onTargetControllerResumed(const Bloom::Targets::TargetDescriptor& targetDescriptor);
        void onTargetStateUpdate(Targets::TargetState newState);
        void refresh();
        void refreshPinStates();
        void refreshProgramCounter(std::optional<std::function<void(void)>> callback = std::nullopt);
        void openReportIssuesUrl();
        void openGettingStartedUrl();
        void openAboutWindow();
        void toggleTargetRegistersPane();
        void toggleRamInspectionPane();
        void toggleEepromInspectionPane();
        void onRegistersPaneStateChanged();
        void onRamInspectionPaneStateChanged();
        void onEepromInspectionPaneStateChanged();
        void onProgrammingModeEnabled();
        void onProgrammingModeDisabled();
    };
}
