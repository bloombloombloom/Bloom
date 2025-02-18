#include "Pin.hpp"

#include "src/Services/StringService.hpp"

namespace Widgets::PinoutWidgets
{
    Pin::Pin(
        const Targets::TargetPinDescriptor& pinDescriptor,
        std::optional<std::reference_wrapper<const Targets::TargetPadDescriptor>> padDescriptor,
        Orientation orientation,
        const PinoutState& pinoutState
    )
        : pinDescriptor(pinDescriptor)
        , padDescriptor(padDescriptor)
        , orientation(orientation)
        , number(Services::StringService::toUint16(this->pinDescriptor.position, 10))
        , pinoutState(pinoutState)
    {}

    QRectF Pin::boundingRect() const {
        return QRectF{
            QPointF{0, 0},
            this->orientation == Orientation::VERTICAL
                ? QSize{Pin::WIDTH, Pin::HEIGHT}
                : QSize{Pin::HEIGHT, Pin::WIDTH}
        };
    }

    void Pin::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
        using Targets::TargetPadType;

        static constexpr auto DEFAULT_COLOR = QColor{0x7D, 0x7D, 0x7D};
        static constexpr auto HIGHLIGHT_COLOR = QColor{0x53, 0x67, 0x5D};

        painter->setBrush(
            this->pinoutState.hoveredPinNumber.has_value() && this->pinoutState.hoveredPinNumber == this->number
                ? HIGHLIGHT_COLOR
                : DEFAULT_COLOR
        );
        painter->setPen(Qt::PenStyle::NoPen);
        painter->setOpacity(this->isEnabled() ? 1 : 0.6);


        if (this->orientation == Orientation::VERTICAL) {
            painter->drawRect(0, 0, Pin::WIDTH, Pin::HEIGHT);

        } else {
            painter->drawRect(0, 0, Pin::HEIGHT, Pin::WIDTH);
        }
    }
}
