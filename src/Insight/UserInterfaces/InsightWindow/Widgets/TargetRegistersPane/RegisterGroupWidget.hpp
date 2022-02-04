#pragma once

#include <QWidget>
#include <unordered_set>
#include <QVBoxLayout>
#include <QLabel>
#include <QSize>
#include <set>

#include "TargetRegistersPaneWidget.hpp"
#include "ItemWidget.hpp"
#include "src/Insight/InsightWorker/InsightWorker.hpp"

#include "../ClickableWidget.hpp"
#include "../SvgWidget.hpp"

namespace Bloom::Widgets
{
    class RegisterWidget;
    class RegisterGroupWidget: public ItemWidget
    {
        Q_OBJECT

    public:
        QString name;
        bool collapsed = true;

        std::set<RegisterWidget*> registerWidgets;
        std::map<Targets::TargetRegisterDescriptor, RegisterWidget*> registerWidgetsMappedByDescriptor;

        RegisterGroupWidget(
            QString name,
            const std::set<Targets::TargetRegisterDescriptor>& registerDescriptors,
            InsightWorker& insightWorker,
            TargetRegistersPaneWidget* parent
        );

        void collapse();
        void expand();
        void setAllRegistersVisible(bool visible);

        void filterRegisters(const QString& keyword);

    private:
        QVBoxLayout* layout = new QVBoxLayout(this);
        ItemWidget* headerWidget = new ItemWidget(this);
        SvgWidget* arrowIcon = new SvgWidget(this->headerWidget);
        SvgWidget* registerGroupIcon = new SvgWidget(this->headerWidget);
        QLabel* label = new QLabel(this->headerWidget);
        QWidget* bodyWidget = new QWidget(this);
    };
}
