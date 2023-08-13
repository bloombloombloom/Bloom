#include "SvgToolButton.hpp"

namespace Widgets
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
                if (this->contextMenuEnabled) {
                    this->contextMenu = menuWidget;
                    this->setContextMenuPolicy(Qt::ContextMenuPolicy::DefaultContextMenu);

                } else {
                    this->setMenu(menuWidget);
                }
            }
        }
    }

    void SvgToolButton::contextMenuEvent(QContextMenuEvent* event) {
        if (this->contextMenu != nullptr) {
            this->contextMenu->exec(this->mapToGlobal(QPoint(0, this->height())));
        }
    }
}
