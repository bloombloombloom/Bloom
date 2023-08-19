#include "ContextMenuAction.hpp"

namespace Widgets
{
    ContextMenuAction::ContextMenuAction(
        const QString& text,
        std::optional<IsEnabledCallbackType> isEnabledCallback,
        QWidget* parent
    )
        : QAction(text, parent)
        , isEnabledCallback(isEnabledCallback)
    {}
}
