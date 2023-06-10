#include "TextInput.hpp"

#include <QMenu>
#include <QAction>

namespace Bloom::Widgets
{
    TextInput::TextInput(QWidget* parent)
        : QLineEdit(parent)
    {}

    void TextInput::contextMenuEvent(QContextMenuEvent* event) {
        if (QMenu* menu = this->createStandardContextMenu()) {
            menu->setAttribute(Qt::WA_DeleteOnClose, true);

            // Remove default icons
            for (auto& action : menu->actions()) {
                action->setIcon(QIcon());
            }

            menu->popup(event->globalPos());
        }
    }

    void TextInput::focusInEvent(QFocusEvent* event) {
        QLineEdit::focusInEvent(event);
        emit this->focusChanged();
    }

    void TextInput::focusOutEvent(QFocusEvent* event) {
        QLineEdit::focusOutEvent(event);
        emit this->focusChanged();
    }
}
