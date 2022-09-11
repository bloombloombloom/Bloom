#pragma once

#include <QGraphicsItem>
#include <cstdint>
#include <QPainter>
#include <QGraphicsScene>
#include <map>
#include <vector>

#include "ByteItem.hpp"
#include "ByteAddressItem.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/HexViewerWidgetSettings.hpp"

namespace Bloom::Widgets
{
    class ByteAddressContainer: public QGraphicsItem
    {
    public:
        static constexpr int WIDTH = 88;

        ByteAddressContainer(const HexViewerWidgetSettings& settings);

        [[nodiscard]] QRectF boundingRect() const override {
            return QRectF(
                0,
                0,
                ByteAddressContainer::WIDTH,
                this->scene()->height()
            );
        }

        void adjustAddressLabels(const std::map<std::size_t, std::vector<ByteItem*>>& byteItemsByRowIndex);
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    private:
        const HexViewerWidgetSettings& settings;
        std::map<std::size_t, ByteAddressItem*> addressItemsByRowIndex;
    };
}
