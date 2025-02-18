#pragma once

#include <QGraphicsItem>
#include <QSize>
#include <QRectF>
#include <functional>
#include <memory>
#include <vector>
#include <map>

#include "src/Targets/TargetPinDescriptor.hpp"
#include "src/Targets/TargetPadDescriptor.hpp"

#include "LabelGroupInterface.hpp"
#include "PinoutState.hpp"
#include "Label.hpp"
#include "PinNumberLabel.hpp"
#include "PadNameLabel.hpp"

namespace Widgets::PinoutWidgets
{
    class VerticalLabelGroup
        : public LabelGroupInterface
        , public QGraphicsItem
    {
    public:
        static constexpr int VERTICAL_PADDING = 6;
        static constexpr int HORIZ_PADDING = 6;
        static constexpr int LABEL_MARGIN = 4;
        static constexpr int MIN_LABEL_WIDTH = 150;
        static constexpr int SECONDARY_LABEL_SEPARATOR_SPACING = 10;

        const Targets::TargetPinDescriptor& pinDescriptor;
        const std::optional<std::reference_wrapper<const Targets::TargetPadDescriptor>> padDescriptor;
        std::uint16_t pinNumber;
        QSize size = {};

        VerticalLabelGroup(
            const Targets::TargetPinDescriptor& pinDescriptor,
            std::optional<std::reference_wrapper<const Targets::TargetPadDescriptor>> padDescriptor,
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
        std::map<std::size_t, std::vector<Label*>> secondaryLabelsByRowIndex;
    };
}
