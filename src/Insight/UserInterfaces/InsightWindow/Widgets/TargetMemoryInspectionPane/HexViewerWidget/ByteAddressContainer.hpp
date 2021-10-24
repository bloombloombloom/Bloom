#pragma once

#include <QGraphicsItem>
#include <cstdint>
#include <QEvent>
#include <QGraphicsScene>
#include <optional>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ClickableWidget.hpp"

namespace Bloom::Widgets
{
    class ByteAddressContainer: public QGraphicsItem
    {
    public:
        static constexpr int WIDTH = 85;

        static constexpr int RIGHT_MARGIN = 5;
        static constexpr int BOTTOM_MARGIN = 5;

        std::size_t byteIndex;
        unsigned char value = 0x00;
        std::uint32_t address = 0x00;
        QString addressHex;
        QString relativeAddressHex;
        bool valueInitialised = false;

        std::size_t currentRowIndex = 0;
        std::size_t currentColumnIndex = 0;

        ByteAddressContainer();

        [[nodiscard]] QRectF boundingRect() const override {
            return QRectF(
                0,
                0,
                ByteAddressContainer::WIDTH,
                this->scene()->height()
            );
        }


    private:

    };
}
