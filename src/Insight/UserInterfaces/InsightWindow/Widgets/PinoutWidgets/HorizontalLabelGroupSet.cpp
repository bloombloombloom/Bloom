#include "HorizontalLabelGroupSet.hpp"

#include <algorithm>
#include <ranges>
#include <QPoint>

namespace Widgets::PinoutWidgets
{
    HorizontalLabelGroupSet::HorizontalLabelGroupSet(Position position)
        : labelGroups({})
        , size({0, 0})
        , position(position)
    {}

    void HorizontalLabelGroupSet::refreshGeometry() {
        auto maxWidth = int{0};

        for (auto* labelGroup : this->labelGroups) {
            labelGroup->refreshGeometry();
            maxWidth = std::max(labelGroup->size.width(), maxWidth);
        }

        auto yPos = int{0};

        const auto positionLabelGroup = [&] (HorizontalLabelGroup* labelGroup) {
            labelGroup->setPos(
                this->position == HorizontalLabelGroupSet::Position::LEFT ? maxWidth - labelGroup->size.width() : 0,
                yPos
            );
            yPos += labelGroup->size.height() + HorizontalLabelGroupSet::GROUP_VERTICAL_MARGIN;
        };

        if (this->position == Position::RIGHT) {
            for (auto groupIt = this->labelGroups.rbegin(); groupIt < this->labelGroups.rend(); ++groupIt) {
                positionLabelGroup(*groupIt);
            }

        } else {
            for (auto groupIt = this->labelGroups.begin(); groupIt < this->labelGroups.end(); ++groupIt) {
                positionLabelGroup(*groupIt);
            }
        }

        this->size.setWidth(maxWidth);
        this->size.setHeight(yPos - HorizontalLabelGroupSet::GROUP_VERTICAL_MARGIN);
    }

    QRectF HorizontalLabelGroupSet::boundingRect() const {
        return QRectF{QPointF{0, 0}, this->size};
    }

    void HorizontalLabelGroupSet::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
        // Nothing to do here...
    }
}
