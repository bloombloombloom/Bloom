#pragma once

#include <QGraphicsItem>
#include <QPainter>
#include <atomic>
#include <mutex>
#include <QPixmap>

#include "HexViewerItem.hpp"

namespace Widgets
{
#pragma pack(push, 1)
    class ByteItem: public HexViewerItem
    {
    public:
        static constexpr int WIDTH = 28;
        static constexpr int HEIGHT = 22;

        static constexpr int RIGHT_MARGIN = 6;
        static constexpr int BOTTOM_MARGIN = 6;

        bool selected:1 = false;
        bool excluded:1 = false;
        bool grouped:1 = false;
        bool stackMemory:1 = false;
        bool changed:1 = false;
        bool highlighted:1 = false;

        explicit ByteItem(Targets::TargetMemoryAddress address);

        QSize size() const override {
            return QSize(ByteItem::WIDTH, ByteItem::HEIGHT);
        }
    };
#pragma pack(pop)
}
