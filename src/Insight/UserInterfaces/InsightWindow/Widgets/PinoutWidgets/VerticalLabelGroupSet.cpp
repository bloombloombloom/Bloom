#include "VerticalLabelGroupSet.hpp"

#include <algorithm>
#include <ranges>
#include <QPoint>

namespace Widgets::PinoutWidgets
{
    VerticalLabelGroupSet::VerticalLabelGroupSet(Position position)
        : labelGroups({})
        , size({0, 0})
        , position(position)
    {}

    void VerticalLabelGroupSet::refreshGeometry() {
        for (auto* labelGroup : this->labelGroups) {
            labelGroup->refreshGeometry();
        }

        const auto firstRowHeight = this->getFirstRowHeight();

        auto maxHeight = int{0};
        auto firstRowXPos = int{0};
        auto secondRowXPos = int{0};

        const auto positionLabelGroup = [&] (VerticalLabelGroup* labelGroup) {

            if (this->isGroupOnFirstRow(labelGroup)) {
                auto position = QPoint{
                    firstRowXPos + VerticalLabelGroupSet::GROUP_HORIZONTAL_MARGIN,
                    0
                };

                if (
                    this->position == Position::TOP
                    || (this->labelGroups.size() % 2 != 0 && firstRowXPos == 0 && secondRowXPos > 0)
                ) {
                    const auto groupCenterX = position.x() + (labelGroup->size.width() / 2);
                    const auto secondRowMinX = secondRowXPos + (VerticalLabelGroupSet::GROUP_HORIZONTAL_MARGIN / 2);

                    if (groupCenterX < secondRowMinX) {
                        position.setX(position.x() + (secondRowMinX - groupCenterX));

                    } else {
                        secondRowXPos = groupCenterX - (VerticalLabelGroupSet::GROUP_HORIZONTAL_MARGIN / 2);
                    }
                }

                labelGroup->setPos(position);
                firstRowXPos = position.x() + labelGroup->size.width();

            } else {
                auto position = QPoint{
                    secondRowXPos + VerticalLabelGroupSet::GROUP_HORIZONTAL_MARGIN,
                    firstRowHeight + VerticalLabelGroupSet::GROUP_VERTICAL_MARGIN
                };

                if (
                    this->position == Position::BOTTOM
                    || (this->labelGroups.size() % 2 != 0 && secondRowXPos == 0 && firstRowXPos > 0)
                ) {
                    const auto groupCenterX = position.x() + (labelGroup->size.width() / 2);
                    const auto firstRowMinX = firstRowXPos + (VerticalLabelGroupSet::GROUP_HORIZONTAL_MARGIN / 2);

                    if (groupCenterX < firstRowMinX) {
                        position.setX(position.x() + (firstRowMinX - groupCenterX));

                    } else {
                        firstRowXPos = groupCenterX - (VerticalLabelGroupSet::GROUP_HORIZONTAL_MARGIN / 2);
                    }
                }

                labelGroup->setPos(position);
                secondRowXPos = position.x() + labelGroup->size.width();
            }

            maxHeight = std::max(static_cast<int>(labelGroup->pos().y() + labelGroup->size.height()), maxHeight);
        };

        if (this->position == Position::TOP) {
            for (auto groupIt = this->labelGroups.rbegin(); groupIt < this->labelGroups.rend(); ++groupIt) {
                positionLabelGroup(*groupIt);
            }

        } else {
            for (auto groupIt = this->labelGroups.begin(); groupIt < this->labelGroups.end(); ++groupIt) {
                positionLabelGroup(*groupIt);
            }
        }

        this->size.setWidth(std::max(firstRowXPos, secondRowXPos));
        this->size.setHeight(maxHeight);
    }

    QRectF VerticalLabelGroupSet::boundingRect() const {
        return QRectF{QPointF{0, 0}, this->size};
    }

    void VerticalLabelGroupSet::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
        // Nothing to do here...
    }

    bool VerticalLabelGroupSet::isGroupOnFirstRow(const VerticalLabelGroup* group) const {
        return (group->pinNumber % 2) != 0;
    }

    int VerticalLabelGroupSet::getFirstRowHeight() const {
        auto height = int{0};

        for (const auto* labelGroup : this->labelGroups) {
            if (!this->isGroupOnFirstRow(labelGroup)) {
                continue;
            }

            height = std::max(labelGroup->size.height(), height);
        }

        return height;
    }
}
