#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QScrollArea>
#include <set>
#include <QSize>
#include <QString>
#include <QEvent>
#include <optional>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PaneWidget.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PanelWidget.hpp"

#include "ItemWidget.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/SvgToolButton.hpp"
#include "src/Targets/TargetState.hpp"
#include "src/Targets/TargetDescriptor.hpp"

namespace Bloom::Widgets
{
    class RegisterGroupWidget;
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

        void refreshRegisterValues(std::optional<std::function<void(void)>> callback = std::nullopt);

        void onItemSelectionChange(Bloom::Widgets::ItemWidget* newlySelectedWidget);

    protected:
        void resizeEvent(QResizeEvent* event) override;

    private:
        const Targets::TargetDescriptor& targetDescriptor;

        QWidget* container = nullptr;

        QWidget* toolBar = nullptr;
        SvgToolButton* collapseAllButton = nullptr;
        SvgToolButton* expandAllButton = nullptr;

        QLineEdit* searchInput = nullptr;
        QScrollArea* itemScrollArea = nullptr;
        QWidget* itemContainer = nullptr;

        ItemWidget* selectedItemWidget = nullptr;

        std::set<RegisterGroupWidget*> registerGroupWidgets;
        Targets::TargetRegisterDescriptors renderedDescriptors;

        Targets::TargetState targetState = Targets::TargetState::UNKNOWN;

        void onTargetStateChanged(Targets::TargetState newState);
        void onRegistersRead(const Targets::TargetRegisters& registers);
        void clearInlineRegisterValues();
    };
}
