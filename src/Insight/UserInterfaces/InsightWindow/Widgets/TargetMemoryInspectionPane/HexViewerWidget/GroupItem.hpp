#pragma once

#include <QPainter>
#include <QSize>
#include <QMargins>
#include <vector>

#include "HexViewerItem.hpp"
#include "ByteItem.hpp"

#include "src/Targets/TargetMemory.hpp"
#include "HexViewerSharedState.hpp"

namespace Bloom::Widgets
{
    class GroupItem: public HexViewerItem
    {
    public:
        std::vector<HexViewerItem*> items;

        ~GroupItem();

        QSize size() const override {
            return this->groupSize;
        }

        virtual void adjustItemPositions(const int maximumWidth, const HexViewerSharedState* hexViewerState);

        [[nodiscard]] std::vector<HexViewerItem*> flattenedItems() const;

        void paint(
            QPainter* painter,
            const HexViewerSharedState* hexViewerState,
            const QGraphicsItem* graphicsItem
        ) const override {
            return;
        }

    protected:
        QSize groupSize = {};
        bool multiLine = false;

        GroupItem(
            Targets::TargetMemoryAddress startAddress,
            HexViewerItem* parent = nullptr
        );

        virtual QMargins groupMargins(const HexViewerSharedState* hexViewerState, const int maximumWidth) const {
            return QMargins(0, 0, 0, 0);
        }

        virtual bool positionOnNewLine(const int maximumWidth) {
            return this->multiLine;
        }

        ByteItem* firstByteItem() const;

        void sortItems();
    };
}
