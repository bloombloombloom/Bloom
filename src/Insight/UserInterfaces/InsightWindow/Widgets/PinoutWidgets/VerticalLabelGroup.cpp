#include "VerticalLabelGroup.hpp"

#include <algorithm>
#include <ranges>

#include "src/Services/StringService.hpp"

namespace Widgets::PinoutWidgets
{
    VerticalLabelGroup::VerticalLabelGroup(
        const Targets::TargetPinDescriptor& pinDescriptor,
        std::optional<std::reference_wrapper<const Targets::TargetPadDescriptor>> padDescriptor,
        const PinoutState& pinoutState
    )
        : pinDescriptor(pinDescriptor)
        , padDescriptor(padDescriptor)
        , pinNumber(Services::StringService::toUint16(this->pinDescriptor.position, 10))
        , pinoutState(pinoutState)
        , pinNumberLabel(PinNumberLabel{this->pinDescriptor})
        , padNameLabel(PadNameLabel{this->padDescriptor})
    {
        this->setAcceptedMouseButtons(Qt::MouseButton::NoButton);
    }

    void VerticalLabelGroup::insertLabel(std::unique_ptr<Label>&& label) {
        this->secondaryLabels.emplace_back(std::move(label));
    }

    void VerticalLabelGroup::refreshGeometry() {
        this->secondaryLabelsByRowIndex.clear();

        this->pinNumberLabel.refreshGeometry();
        this->padNameLabel.refreshGeometry();

        const auto primaryRowWidth = this->pinNumberLabel.minimumWidth + VerticalLabelGroup::LABEL_MARGIN
            + this->padNameLabel.minimumWidth;
        auto maxLabelWidth = std::max(primaryRowWidth, VerticalLabelGroup::MIN_LABEL_WIDTH);

        for (const auto& secondaryLabel : this->secondaryLabels) {
            if (!secondaryLabel->enabled) {
                continue;
            }

            secondaryLabel->refreshGeometry();

            if (secondaryLabel->width > maxLabelWidth) {
                maxLabelWidth = secondaryLabel->width;
            }
        }

        const auto addLabelToRow = [this, maxLabelWidth] (Label& label) -> std::size_t {
            for (auto& [rowIndex, rowLabels] : this->secondaryLabelsByRowIndex) {
                const auto occupiedRowWidth = std::accumulate(
                    rowLabels.begin(),
                    rowLabels.end(),
                    int{0},
                    [] (int width, const Label* label) {
                        return width + label->minimumWidth + VerticalLabelGroup::LABEL_MARGIN;
                    }
                ) - VerticalLabelGroup::LABEL_MARGIN;
                const auto availableRowWidth = maxLabelWidth - occupiedRowWidth;

                if (label.minimumWidth <= availableRowWidth) {
                    rowLabels.emplace_back(&label);
                    return rowIndex;
                }
            }

            const auto newRowIndex = this->secondaryLabelsByRowIndex.empty()
                ? 0
                : this->secondaryLabelsByRowIndex.size();
            this->secondaryLabelsByRowIndex.emplace(newRowIndex, std::vector{&label});
            return newRowIndex;
        };

        for (auto& secondaryLabel : this->secondaryLabels) {
            if (!secondaryLabel->enabled) {
                continue;
            }

            addLabelToRow(*secondaryLabel);
        }

        // Resize label widths to fill any excess row space
        for (const auto& rowLabels : std::views::values(this->secondaryLabelsByRowIndex)) {
            const auto occupiedRowWidth = std::accumulate(
                rowLabels.begin(),
                rowLabels.end(),
                int{0},
                [] (int width, const Label* label) {
                    return width + label->minimumWidth + VerticalLabelGroup::LABEL_MARGIN;
                }
            ) - VerticalLabelGroup::LABEL_MARGIN;
            const auto excessWidth = maxLabelWidth - occupiedRowWidth;

            if (excessWidth == 0) {
                continue;
            }

            for (auto* label : rowLabels) {
                label->width += excessWidth / static_cast<int>(rowLabels.size());
            }

            // Add the remainder to the first label in the row
            (*rowLabels.begin())->width += excessWidth % static_cast<int>(rowLabels.size());
        }

        if (primaryRowWidth < maxLabelWidth) {
            this->padNameLabel.width += maxLabelWidth - primaryRowWidth;
        }

        this->size.setWidth(maxLabelWidth + (VerticalLabelGroup::HORIZ_PADDING * 2));
        this->size.setHeight(
            (
                static_cast<int>(this->secondaryLabelsByRowIndex.size() + 1)
                    * (Label::HEIGHT + VerticalLabelGroup::LABEL_MARGIN)
            ) - VerticalLabelGroup::LABEL_MARGIN + (VerticalLabelGroup::VERTICAL_PADDING * 2)
            + (!this->secondaryLabelsByRowIndex.empty() ? VerticalLabelGroup::SECONDARY_LABEL_SEPARATOR_SPACING : 0)
        );
    }

    QRectF VerticalLabelGroup::boundingRect() const {
        return QRectF{QPointF{0, 0}, this->size};
    }

    void VerticalLabelGroup::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
        static constexpr auto LINE_COLOR = QColor{0x5E, 0x5C, 0x59};
        static constexpr auto HIGHLIGHTED_LINE_COLOR = QColor{0x7D, 0x7D, 0x7D};

        painter->setOpacity(this->isEnabled() ? 1 : 0.6);
        painter->setPen(Qt::PenStyle::NoPen);
        constexpr auto START_X = VerticalLabelGroup::HORIZ_PADDING;
        auto yPos = VerticalLabelGroup::VERTICAL_PADDING;

        this->pinNumberLabel.paint(painter, START_X, yPos);
        this->padNameLabel.paint(
            painter,
            START_X + this->pinNumberLabel.width + VerticalLabelGroup::LABEL_MARGIN,
            yPos
        );
        yPos += Label::HEIGHT + VerticalLabelGroup::LABEL_MARGIN;

        const auto& lineColor = this->pinoutState.hoveredPinNumber.has_value()
            && this->pinoutState.hoveredPinNumber == this->pinNumber
                ? HIGHLIGHTED_LINE_COLOR
                : LINE_COLOR;

        if (!this->secondaryLabelsByRowIndex.empty()) {
            painter->setPen(lineColor);
            const auto centerX = this->size.width() / 2;
            painter->drawLine(
                centerX,
                yPos,
                centerX,
                yPos + VerticalLabelGroup::SECONDARY_LABEL_SEPARATOR_SPACING - VerticalLabelGroup::LABEL_MARGIN
            );
            painter->setPen(Qt::PenStyle::NoPen);

            yPos += VerticalLabelGroup::SECONDARY_LABEL_SEPARATOR_SPACING;

            for (const auto& rowLabels : std::views::values(this->secondaryLabelsByRowIndex)) {
                auto xPos = START_X;

                for (const auto* label : rowLabels) {
                    label->paint(painter, xPos, yPos);
                    xPos += label->width + VerticalLabelGroup::LABEL_MARGIN;
                }

                yPos += Label::HEIGHT + VerticalLabelGroup::LABEL_MARGIN;
            }
        }

        static constexpr auto VERTICAL_BORDER_LENGTH = 10;
        painter->setPen(lineColor);
        painter->drawLine(0, 0, 0, VERTICAL_BORDER_LENGTH);
        painter->drawLine(0, 0, this->size.width(), 0);
        painter->drawLine(this->size.width(), 0, this->size.width(), VERTICAL_BORDER_LENGTH);

        painter->drawLine(0, this->size.height() - 1, 0, this->size.height() - 1 - VERTICAL_BORDER_LENGTH);
        painter->drawLine(0, this->size.height() - 1, this->size.width(), this->size.height() - 1);
        painter->drawLine(
            this->size.width(),
            this->size.height() - 1, this->size.width(),
            this->size.height() - 1 - VERTICAL_BORDER_LENGTH
        );
    }
}
