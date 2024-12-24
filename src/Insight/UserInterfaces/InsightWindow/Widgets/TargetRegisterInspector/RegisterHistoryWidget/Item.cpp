#include "Item.hpp"

#include <QStyle>

namespace Widgets
{
    Item::Item(const Targets::TargetMemoryBuffer& registerValue, QWidget* parent)
        : ClickableWidget(parent)
        , registerValue(registerValue)
    {
        auto onClick = [this] {
            this->setSelected(true);
        };

        QObject::connect(this, &ClickableWidget::clicked, this, onClick);
        QObject::connect(this, &ClickableWidget::rightClicked, this, onClick);

        this->setSelected(false);
    }

    void Item::setSelected(bool selected) {
        this->setProperty("selected", selected);

        // TODO: This is a horrible hack. It will be binned when I rewrite this widget to use ListView
        this->setStyleSheet(this->styleSheet());

        if (selected) {
            emit this->selected(this);
        }
    }
}
