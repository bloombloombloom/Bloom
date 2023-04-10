#include "ContextMenuAction.hpp"

namespace Bloom::Widgets
{
    ContextMenuAction::ContextMenuAction(
        const QString& text,
        std::optional<IsEnabledCallbackType> isEnabledCallback,
        QWidget* parent
    )
        : isEnabledCallback(isEnabledCallback)
        , QAction(text, parent)
    {}
}
