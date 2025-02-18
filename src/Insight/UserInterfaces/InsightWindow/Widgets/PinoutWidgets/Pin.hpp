#pragma once

#include <QGraphicsItem>
#include <QSize>
#include <QRectF>
#include <QPainter>
#include <functional>

#include "src/Targets/TargetPinDescriptor.hpp"
#include "src/Targets/TargetPadDescriptor.hpp"

#include "PinoutState.hpp"

namespace Widgets::PinoutWidgets
{
    class Pin: public QGraphicsItem
    {
    public:
        static constexpr int WIDTH = 11;
        static constexpr int HEIGHT = 17;

        enum Orientation: std::uint8_t
        {
            VERTICAL,
            HORIZONTAL
        };

        const Targets::TargetPinDescriptor& pinDescriptor;
        const std::optional<std::reference_wrapper<const Targets::TargetPadDescriptor>> padDescriptor;
        const Orientation orientation;
        std::uint16_t number;

        Pin(
            const Targets::TargetPinDescriptor& pinDescriptor,
            std::optional<std::reference_wrapper<const Targets::TargetPadDescriptor>> padDescriptor,
            Orientation orientation,
            const PinoutState& pinoutState
        );

        [[nodiscard]] QRectF boundingRect() const override;

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    protected:
        const PinoutState& pinoutState;
    };
}
