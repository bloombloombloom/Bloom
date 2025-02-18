#pragma once

#include <QSize>
#include <QPoint>
#include <QPainter>
#include <vector>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PinoutWidgets/PinoutItem.hpp"

#include "src/Targets/TargetPinoutDescriptor.hpp"
#include "src/Targets/TargetDescriptor.hpp"
#include "src/Targets/TargetPadDescriptor.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PinoutWidgets/PinoutState.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PinoutWidgets/Pin.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PinoutWidgets/LabelGroupInterface.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PinoutWidgets/VerticalLabelGroup.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PinoutWidgets/VerticalLabelGroupSet.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PinoutWidgets/VerticalLabelGroupPinPair.hpp"

#include "src/Helpers/Pair.hpp"

namespace Widgets::PinoutWidgets
{
    class DipPinout: public PinoutItem
    {
    public:
        static constexpr int PACKAGE_PADDING = 15;
        static constexpr int MAX_PACKAGE_HEIGHT = 88;
        static constexpr int MIN_PACKAGE_HEIGHT = 65;
        static constexpr int PIN_MARGIN = 2;
        static constexpr int PIN_LINE_A_SPACING = 15;
        static constexpr int PIN_LINE_MARGIN = 5;
        static constexpr int MIN_LINE_A_LENGTH = 10;
        static constexpr int P1_INDICATOR_DIAMETER = 12;
        static constexpr int P1_INDICATOR_MARGIN = 8;
        static constexpr int P1_EDGE_INDICATOR_DIAMETER = 18;

        QSize size = {};

        QSize packageBodySize;
        QPoint packageBodyPosition;

        DipPinout(
            const Targets::TargetPinoutDescriptor& pinoutDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            const PinoutState& pinoutState
        );

        std::vector<Pair<const Targets::TargetPadDescriptor&, LabelGroupInterface*>> padDescriptorLabelGroupPairs();
        void refreshGeometry() override;
        [[nodiscard]] QRectF boundingRect() const override;
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    private:
        const Targets::TargetPinoutDescriptor& pinoutDescriptor;
        const PinoutState& pinoutState;

        VerticalLabelGroupSet* topQuadrantLabelGroupSet = new VerticalLabelGroupSet{
            VerticalLabelGroupSet::Position::TOP
        };
        VerticalLabelGroupSet* bottomQuadrantLabelGroupSet = new VerticalLabelGroupSet{
            VerticalLabelGroupSet::Position::BOTTOM
        };

        std::vector<VerticalLabelGroupPinPair> topLabelGroupPinPairs = {};
        std::vector<VerticalLabelGroupPinPair> bottomLabelGroupPinPairs = {};
    };
}
