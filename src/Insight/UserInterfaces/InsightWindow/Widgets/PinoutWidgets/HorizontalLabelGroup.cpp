#include "HorizontalLabelGroup.hpp"

#include "src/Services/StringService.hpp"

namespace Widgets::PinoutWidgets
{
    HorizontalLabelGroup::HorizontalLabelGroup(
        const Targets::TargetPinDescriptor& pinDescriptor,
        std::optional<std::reference_wrapper<const Targets::TargetPadDescriptor>> padDescriptor,
        Direction direction,
        const PinoutState& pinoutState
    )
        : pinDescriptor(pinDescriptor)
        , padDescriptor(padDescriptor)
        , pinNumber(Services::StringService::toUint16(this->pinDescriptor.position, 10))
        , size({0, 0})
        , direction(direction)
        , pinoutState(pinoutState)
        , pinNumberLabel(PinNumberLabel{this->pinDescriptor})
        , padNameLabel(PadNameLabel{this->padDescriptor})
    {}

    void HorizontalLabelGroup::insertLabel(std::unique_ptr<Label>&& label) {
        this->secondaryLabels.emplace_back(std::move(label));
    }

    void HorizontalLabelGroup::refreshGeometry() {
        this->enabledSecondaryLabels.clear();

        this->pinNumberLabel.refreshGeometry();
        this->padNameLabel.refreshGeometry();

        if (this->pinNumberLabel.width < HorizontalLabelGroup::MIN_PIN_NUMBER_LABEL_WIDTH) {
            this->pinNumberLabel.width = HorizontalLabelGroup::MIN_PIN_NUMBER_LABEL_WIDTH;
        }

        if (this->padNameLabel.width < HorizontalLabelGroup::MIN_LABEL_WIDTH) {
            this->padNameLabel.width = HorizontalLabelGroup::MIN_LABEL_WIDTH;
        }

        auto totalWidth = this->pinNumberLabel.width + HorizontalLabelGroup::LABEL_MARGIN + this->padNameLabel.width
            + (!this->secondaryLabels.empty() ? HorizontalLabelGroup::SECONDARY_LABEL_SEPARATOR_SPACING : 0);

        for (const auto& secondaryLabel : this->secondaryLabels) {
            if (!secondaryLabel->enabled) {
                continue;
            }

            this->enabledSecondaryLabels.emplace_back(secondaryLabel.get());
            secondaryLabel->refreshGeometry();

            if (secondaryLabel->width < HorizontalLabelGroup::MIN_LABEL_WIDTH) {
                secondaryLabel->width = HorizontalLabelGroup::MIN_LABEL_WIDTH;
            }

            totalWidth += HorizontalLabelGroup::LABEL_MARGIN + secondaryLabel->width;
        }

        this->size.setWidth(totalWidth);
        this->size.setHeight(Label::HEIGHT);
    }

    QRectF HorizontalLabelGroup::boundingRect() const {
        return QRectF{QPointF{0, 0}, this->size};
    }

    void HorizontalLabelGroup::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
        static constexpr auto LINE_COLOR = QColor{0x5E, 0x5C, 0x59};

        painter->setOpacity(this->isEnabled() ? 1 : 0.6);
        painter->setPen(Qt::PenStyle::NoPen);
        auto xPos = int{0};

        this->pinNumberLabel.paint(painter, this->transformLabelXPos(xPos, this->pinNumberLabel), 0);
        xPos += this->pinNumberLabel.width + HorizontalLabelGroup::LABEL_MARGIN;

        this->padNameLabel.paint(painter, this->transformLabelXPos(xPos, this->padNameLabel), 0);
        xPos += this->padNameLabel.width + HorizontalLabelGroup::LABEL_MARGIN;

        if (!this->enabledSecondaryLabels.empty()) {
            painter->setPen(LINE_COLOR);
            static constexpr auto centerY = Label::HEIGHT / 2;
            const auto separatorLineXStart = this->direction == Direction::LEFT ? this->size.width() - xPos : xPos;
            painter->drawLine(
                separatorLineXStart,
                centerY,
                separatorLineXStart + (
                    this->direction == Direction::LEFT
                        ? (HorizontalLabelGroup::SECONDARY_LABEL_SEPARATOR_SPACING * -1) + HorizontalLabelGroup::LABEL_MARGIN
                        : HorizontalLabelGroup::SECONDARY_LABEL_SEPARATOR_SPACING - HorizontalLabelGroup::LABEL_MARGIN
                ),
                centerY
            );
            painter->setPen(Qt::PenStyle::NoPen);

            xPos += HorizontalLabelGroup::SECONDARY_LABEL_SEPARATOR_SPACING;

            for (const auto& label : this->enabledSecondaryLabels) {
                label->paint(painter, this->transformLabelXPos(xPos, *label), 0);
                xPos += label->width + HorizontalLabelGroup::LABEL_MARGIN;
            }
        }
    }

    int HorizontalLabelGroup::transformLabelXPos(int xPos, const Label& label) const {
        return this->direction == Direction::LEFT ? this->size.width() - xPos - label.width : xPos;
    }
}
