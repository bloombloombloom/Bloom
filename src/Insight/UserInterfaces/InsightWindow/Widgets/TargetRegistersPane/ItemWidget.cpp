#include "ItemWidget.hpp"

#include <QStyle>

namespace Bloom::Widgets
{
    ItemWidget::ItemWidget(QWidget* parent): ClickableWidget(parent) {
        auto onClick = [this] {
            this->setSelected(true);
        };

        QObject::connect(this, &ClickableWidget::clicked, this, onClick);
        QObject::connect(this, &ClickableWidget::rightClicked, this, onClick);

        this->setSelected(false);
    }

    void ItemWidget::setSelected(bool selected) {
        this->setProperty("selected", selected);
        this->style()->unpolish(this);
        this->style()->polish(this);

        if (selected) {
            emit this->selected(this);
        }

        this->postSetSelected(selected);
    }
}
