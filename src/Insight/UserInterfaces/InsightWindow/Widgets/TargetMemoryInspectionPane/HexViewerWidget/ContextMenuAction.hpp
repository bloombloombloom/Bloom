#pragma once

#include <QAction>
#include <QWidget>
#include <functional>
#include <QString>
#include <optional>

#include "src/Targets/TargetMemory.hpp"
#include "ByteItem.hpp"

namespace Bloom::Widgets
{
    class ContextMenuAction: public QAction
    {
        Q_OBJECT

        /*
         * The ContextMenuAction::isEnabledCallback member is just a callback function that implements any specific
         * rules which determine if the action should be enabled (in the context menu).
         *
         * If the callback returns false, the menu action will be disabled.
         *
         * The callback is called just before the context menu is presented to the user, everytime the context menu
         * is requested.
         *
         * If no callback is specified (ContextMenuAction::isEnabledCallback == std::nullopt), the menu action will
         * always be enabled.
         */
        using IsEnabledCallbackType = std::function<
            bool(const std::unordered_map<Targets::TargetMemoryAddress, ByteItem*>&)
        >;

    public:
        std::optional<IsEnabledCallbackType> isEnabledCallback;

        ContextMenuAction(
            const QString& text,
            std::optional<IsEnabledCallbackType> isEnabledCallback,
            QWidget* parent
        );

    signals:
        void invoked(const std::unordered_map<Targets::TargetMemoryAddress, ByteItem*>& selectedByteItemsByAddress);
    };
}
