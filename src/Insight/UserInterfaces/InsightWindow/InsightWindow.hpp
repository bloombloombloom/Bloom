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
#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetVariantDescriptor.hpp"
#include "src/Targets/TargetPinoutDescriptor.hpp"

#include "Widgets/Label.hpp"
#include "Widgets/SvgToolButton.hpp"
#include "Widgets/PinoutWidgets/PinoutContainer.hpp"
#include "Widgets/PinoutWidgets/PinoutScene.hpp"
#include "Widgets/PanelWidget.hpp"
#include "Widgets/TargetRegistersPane/TargetRegistersPaneWidget.hpp"
#include "Widgets/TargetMemoryInspectionPane/TargetMemoryInspectionPane.hpp"
#include "Widgets/TargetMemoryInspectionPane/ToolButton.hpp"
#include "Widgets/TargetMemoryInspectionPane/TargetMemoryInspectionPaneSettings.hpp"
#include "Widgets/TaskIndicator/TaskIndicator.hpp"
#include "AboutWindow.hpp"

class InsightWindow: public QMainWindow
{
    Q_OBJECT

public:
    InsightWindow(
        InsightProjectSettings& settings,
        const InsightConfig& insightConfig,
        const EnvironmentConfig& environmentConfig,
        const Targets::TargetDescriptor& targetDescriptor,
        const Targets::TargetState& targetState
    );

protected:
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    InsightProjectSettings& settings;

    InsightConfig insightConfig;
    EnvironmentConfig environmentConfig;
    TargetConfig targetConfig;

    const Targets::TargetDescriptor& targetDescriptor;
    const Targets::TargetState& targetState;

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
    QAction* refreshRegistersOnTargetStopAction = nullptr;
    QAction* refreshGpioOnTargetStopAction = nullptr;

    QWidget* leftMenuBar = nullptr;
    Widgets::PanelWidget* leftPanel = nullptr;
    Widgets::TargetRegistersPaneWidget* targetRegistersSidePane = nullptr;
    QToolButton* targetRegistersButton = nullptr;

    Widgets::PinoutWidgets::PinoutContainer* pinoutContainerWidget = nullptr;
    Widgets::PinoutWidgets::PinoutScene* pinoutScene = nullptr;

    QWidget* bottomMenuBar = nullptr;
    QHBoxLayout* bottomMenuBarLayout = nullptr;
    Widgets::PanelWidget* bottomPanel = nullptr;
    std::vector<Widgets::TargetMemoryInspectionPane*> memoryInspectionPaneWidgets = {};

    QWidget* footer = nullptr;
    Widgets::Label* targetStatusLabel = nullptr;
    Widgets::Label* programCounterValueLabel = nullptr;
    Widgets::TaskIndicator* taskIndicator = nullptr;

    const Targets::TargetVariantDescriptor* selectedVariantDescriptor = nullptr;
    bool uiDisabled = false;

    void setUiDisabled(bool disable);

    void populateVariantMenu();
    void selectDefaultVariant();
    void selectVariant(const Targets::TargetVariantDescriptor* variantDescriptor);
    void createPanes();

    void adjustPanels();
    void adjustMinimumSize();

    void onTargetStateUpdate(Targets::TargetState newState, Targets::TargetState previousState);
    void setRefreshRegistersOnTargetStopped(bool enabled);
    void setRefreshGpioOnTargetStopped(bool enabled);
    void refresh(bool refreshRegisters, bool refreshGpio);
    void refreshPadStates();
    void openReportIssuesUrl();
    void openGettingStartedUrl();
    void openAboutWindow();
    void toggleTargetRegistersPane();
    void toggleMemoryInspectionPane(Widgets::TargetMemoryInspectionPane* pane);
    void toggleEepromInspectionPane();
    void toggleFlashInspectionPane();
    void onRegistersPaneStateChanged();
    void onMemoryInspectionPaneStateChanged(Widgets::TargetMemoryInspectionPane* pane, Widgets::ToolButton* toolBtn);
    void onProgrammingModeEnabled();
    void onProgrammingModeDisabled();
};
