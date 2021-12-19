#include "TextInput.hpp"

#include <QMenu>
#include <QAction>

using namespace Bloom::Widgets;

TextInput::TextInput(QWidget* parent): QLineEdit(parent) {}

void TextInput::contextMenuEvent(QContextMenuEvent* event) {
    if (QMenu *menu = createStandardContextMenu()) {
        menu->setAttribute(Qt::WA_DeleteOnClose);

        // Remove default icons
        for (auto& action : menu->actions()) {
            action->setIcon(QIcon());
        }

        menu->popup(event->globalPos());
    }
}
