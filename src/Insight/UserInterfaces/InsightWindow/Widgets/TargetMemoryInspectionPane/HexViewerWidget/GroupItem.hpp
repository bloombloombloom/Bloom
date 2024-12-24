#pragma once

#include <QPainter>
#include <QSize>
#include <QMargins>
#include <vector>

#include "HexViewerItem.hpp"
#include "ByteItem.hpp"

#include "src/Targets/TargetMemory.hpp"
#include "HexViewerSharedState.hpp"

namespace Widgets
{
    class GroupItem: public HexViewerItem
    {
    public:
        std::vector<HexViewerItem*> items;
        QSize groupSize = {};
        bool multiLine = false;

        ~GroupItem();

        [[nodiscard]] QSize size() const override {
            return this->groupSize;
        }

        virtual void adjustItemPositions(int maximumWidth, const HexViewerSharedState* hexViewerState);

        [[nodiscard]] std::vector<HexViewerItem*> flattenedItems() const;

    protected:
        explicit GroupItem(
            Targets::TargetMemoryAddress startAddress,
            HexViewerItem* parent = nullptr
        );

        virtual QMargins groupMargins(const HexViewerSharedState* hexViewerState, int maximumWidth) const {
            return {0, 0, 0, 0};
        }

        virtual bool positionOnNewLine(const int maximumWidth) {
            return this->multiLine;
        }

        [[nodiscard]] ByteItem* firstByteItem() const;

        void sortItems();
    };
}
