#include "SvgToolButton.hpp"

#include <QMenu>

namespace Bloom::Widgets
{
    SvgToolButton::SvgToolButton(QWidget* parent): QToolButton(parent) {
        this->setButtonWidth(10);
        this->setButtonHeight(10);
    }

    void SvgToolButton::childEvent(QChildEvent* childEvent) {
        if ((childEvent->added() || childEvent->polished()) && childEvent->child()->isWidgetType()) {
            /*
             * If a menu widget has been added as a child to this SvgToolButton, associate the menu with the button
             * via QToolButton::setMenu().
             */
            auto* menuWidget = qobject_cast<QMenu*>(childEvent->child());
            if (menuWidget != nullptr && menuWidget != this->menu()) {
                this->setMenu(menuWidget);
            }
        }
    }
}
