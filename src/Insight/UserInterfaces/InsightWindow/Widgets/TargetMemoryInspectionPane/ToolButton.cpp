#include "ToolButton.hpp"

#include <QFont>
#include <QFontMetrics>

#include "src/Services/PathService.hpp"
#include "src/Logger/Logger.hpp"

namespace Widgets
{
    ToolButton::ToolButton(QString&& label, QWidget* parent)
        : QToolButton(parent)
        , layout(new QHBoxLayout{this})
        , icon(new SvgWidget{this})
        , label(new Label{label, this})
    {
        this->setCheckable(true);
        this->setToolTip(label);
        this->setLayout(this->layout);

        this->icon->setSvgFilePath(
            QString::fromStdString(Services::PathService::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/Images/memory-inspection-icon.svg"
            )
        );
        this->icon->setContainerHeight(22);
        this->icon->setContainerWidth(14);

        this->layout->addWidget(this->icon);
        this->layout->addWidget(this->label);

        this->layout->setContentsMargins(10, 0, 10, 0);
        this->layout->setSpacing(8);

        this->label->setContentsMargins(0, 0, 0, 0);
        this->icon->setContentsMargins(0, 1, 0, 0);

        this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    }

    QSize ToolButton::minimumSizeHint() const {
        return this->layout->minimumSize();
    }
}
