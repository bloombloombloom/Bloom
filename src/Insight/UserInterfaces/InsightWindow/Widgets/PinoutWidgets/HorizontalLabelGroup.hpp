#pragma once

#include <QGraphicsItem>
#include <QSize>
#include <QRectF>
#include <QPainter>
#include <functional>
#include <vector>
#include <memory>

#include "src/Targets/TargetPinDescriptor.hpp"
#include "src/Targets/TargetPadDescriptor.hpp"

#include "LabelGroupInterface.hpp"
#include "PinoutState.hpp"
#include "Label.hpp"
#include "PinNumberLabel.hpp"
#include "PadNameLabel.hpp"

namespace Widgets::PinoutWidgets
{
    class HorizontalLabelGroup
        : public LabelGroupInterface
        , public QGraphicsItem
    {
    public:
        static constexpr int LABEL_MARGIN = 4;
        static constexpr int MIN_PIN_NUMBER_LABEL_WIDTH = 45;
        static constexpr int MIN_LABEL_WIDTH = 55;
        static constexpr int SECONDARY_LABEL_SEPARATOR_SPACING = 10;

        enum Direction: std::uint8_t
        {
            LEFT,
            RIGHT,
        };

        const Targets::TargetPinDescriptor& pinDescriptor;
        const std::optional<std::reference_wrapper<const Targets::TargetPadDescriptor>> padDescriptor;
        std::uint16_t pinNumber;
        QSize size = {};
        const Direction direction;

        HorizontalLabelGroup(
            const Targets::TargetPinDescriptor& pinDescriptor,
            std::optional<std::reference_wrapper<const Targets::TargetPadDescriptor>> padDescriptor,
            Direction direction,
            const PinoutState& pinoutState
        );

        void insertLabel(std::unique_ptr<Label>&& label) override;
        void refreshGeometry();
        [[nodiscard]] QRectF boundingRect() const override;
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    private:
        const PinoutState& pinoutState;
        PinNumberLabel pinNumberLabel;
        PadNameLabel padNameLabel;

        std::vector<std::unique_ptr<Label>> secondaryLabels;
        std::vector<Label*> enabledSecondaryLabels;

        [[nodiscard]] inline int transformLabelXPos(
            int xPos,
            const Label& label
        ) const __attribute__((__always_inline__));
    };
}
