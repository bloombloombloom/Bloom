#pragma once

#include <cstdint>
#include <QGraphicsItem>
#include <map>

#include "ByteItem.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/AddressType.hpp"

namespace Bloom::Widgets
{
    class ByteAddressItem: public QGraphicsItem
    {
    public:
        static constexpr int WIDTH = 75;
        static constexpr int HEIGHT = ByteItem::HEIGHT;

        std::size_t rowIndex = 0;

        explicit ByteAddressItem(
            std::size_t rowIndex,
            const std::map<std::size_t, std::vector<ByteItem*>>& byteItemsByRowIndex,
            const AddressType& addressType,
            QGraphicsItem* parent
        );

        [[nodiscard]] QRectF boundingRect() const override {
            return {
                0,
                0,
                ByteAddressItem::WIDTH,
                ByteAddressItem::HEIGHT
            };
        }

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    private:
        const std::map<std::size_t, std::vector<ByteItem*>>& byteItemsByRowIndex;
        const AddressType& addressType;
    };
}
