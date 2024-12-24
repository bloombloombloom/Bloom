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
#include "Widgets/TargetWidgets/TargetPackageWidgetContainer.hpp"
#include "Widgets/TargetWidgets/TargetPackageWidget.hpp"
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
        InsightProjectSettings& insightProjectSettings,
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
    InsightProjectSettings& insightProjectSettings;

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

    QWidget* leftMenuBar = nullptr;
    Widgets::PanelWidget* leftPanel = nullptr;
    Widgets::TargetRegistersPaneWidget* targetRegistersSidePane = nullptr;
    QToolButton* targetRegistersButton = nullptr;

    Widgets::InsightTargetWidgets::TargetPackageWidgetContainer* ioContainerWidget = nullptr;
    Widgets::InsightTargetWidgets::TargetPackageWidget* targetPackageWidget = nullptr;

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

    static bool isPinoutSupported(const Targets::TargetPinoutDescriptor& pinoutDescriptor);

    void setUiDisabled(bool disable);

    void populateVariantMenu();
    void selectDefaultVariant();
    void selectVariant(const Targets::TargetVariantDescriptor* variantDescriptor);
    void createPanes();

    void adjustPanels();
    void adjustMinimumSize();

    void onTargetStateUpdate(Targets::TargetState newState, Targets::TargetState previousState);
    void refresh();
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
