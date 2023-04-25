#include "PlainTextEdit.hpp"

#include <QMenu>
#include <QAction>

namespace Bloom::Widgets
{
    PlainTextEdit::PlainTextEdit(QWidget* parent)
        : QPlainTextEdit(parent)
    {}

    void PlainTextEdit::contextMenuEvent(QContextMenuEvent* event) {
        if (QMenu* menu = this->createStandardContextMenu()) {
            menu->setAttribute(Qt::WA_DeleteOnClose);

            // Remove default icons
            for (auto& action : menu->actions()) {
                action->setIcon(QIcon());
            }

            menu->popup(event->globalPos());
        }
    }
}
