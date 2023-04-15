#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QScrollArea>
#include <set>
#include <unordered_map>
#include <QSize>
#include <QString>
#include <QEvent>
#include <QAction>
#include <optional>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PaneWidget.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PanelWidget.hpp"

#include "RegisterItem.hpp"
#include "RegisterGroupItem.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetRegisterInspector/TargetRegisterInspectorWindow.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ListView/ListView.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/SvgToolButton.hpp"
#include "src/Targets/TargetState.hpp"
#include "src/Targets/TargetDescriptor.hpp"

namespace Bloom::Widgets
{
    class TargetRegistersPaneWidget: public PaneWidget
    {
        Q_OBJECT

    public:
        TargetRegistersPaneWidget(
            const Targets::TargetDescriptor& targetDescriptor,
            PaneState& paneState,
            PanelWidget *parent
        );

        void filterRegisters(const QString& keyword);
        void collapseAllRegisterGroups();
        void expandAllRegisterGroups();

        void refreshRegisterValues(
            std::optional<Targets::TargetRegisterDescriptor> registerDescriptor = std::nullopt,
            std::optional<std::function<void(void)>> callback = std::nullopt
        );

    protected:
        void resizeEvent(QResizeEvent* event) override;

    private:
        const Targets::TargetDescriptor& targetDescriptor;

        QWidget* container = nullptr;

        QWidget* toolBar = nullptr;
        SvgToolButton* collapseAllButton = nullptr;
        SvgToolButton* expandAllButton = nullptr;

        QLineEdit* searchInput = nullptr;
        ListView* registerListView = nullptr;
        ListScene* registerListScene = nullptr;

        Targets::TargetRegisterDescriptors registerDescriptors;
        std::vector<RegisterGroupItem*> registerGroupItems;
        std::unordered_map<Targets::TargetRegisterDescriptor, RegisterItem*> registerItemsByDescriptor;
        std::unordered_map<Targets::TargetRegisterDescriptor, TargetRegisterInspectorWindow*> inspectionWindowsByDescriptor;
        std::unordered_map<Targets::TargetRegisterDescriptor, Targets::TargetMemoryBuffer> currentRegisterValues;

        Targets::TargetState targetState = Targets::TargetState::UNKNOWN;

        // Context-menu actions
        QAction* openInspectionWindowAction = new QAction("Inspect", this);
        QAction* refreshValueAction = new QAction("Refresh Value", this);
        QAction* copyNameAction = new QAction("Register Name", this);
        QAction* copyValueDecimalAction = new QAction("Value as Decimal", this);
        QAction* copyValueHexAction = new QAction("...as Hex String", this);
        QAction* copyValueBinaryAction = new QAction("...as Binary Bit String", this);

        RegisterItem* contextMenuRegisterItem = nullptr;

        void onItemDoubleClicked(ListItem* clickedItem);
        void onItemContextMenu(ListItem* item, QPoint sourcePosition);
        void onTargetStateChanged(Targets::TargetState newState);
        void onRegistersRead(const Targets::TargetRegisters& registers);
        void clearInlineRegisterValues();
        void openInspectionWindow(const Targets::TargetRegisterDescriptor& registerDescriptor);
        void copyRegisterName(const Targets::TargetRegisterDescriptor& registerDescriptor);
        void copyRegisterValueHex(const Targets::TargetRegisterDescriptor& registerDescriptor);
        void copyRegisterValueDecimal(const Targets::TargetRegisterDescriptor& registerDescriptor);
        void copyRegisterValueBinary(const Targets::TargetRegisterDescriptor& registerDescriptor);
    };
}
