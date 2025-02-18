#include "DipPinout.hpp"

#include <algorithm>
#include <numeric>

#include "src/Services/StringService.hpp"

namespace Widgets::PinoutWidgets
{
    DipPinout::DipPinout(
        const Targets::TargetPinoutDescriptor& pinoutDescriptor,
        const Targets::TargetDescriptor& targetDescriptor,
        const PinoutState& pinoutState
    )
        : packageBodySize({
            (Pin::WIDTH + DipPinout::PIN_MARGIN) * static_cast<int>(pinoutDescriptor.pinDescriptors.size() / 2)
                - DipPinout::PIN_MARGIN + (DipPinout::PACKAGE_PADDING * 2),
            std::max(
                std::min(static_cast<int>(pinoutDescriptor.pinDescriptors.size()) * 8, DipPinout::MAX_PACKAGE_HEIGHT),
                DipPinout::MIN_PACKAGE_HEIGHT
            )
        })
        , packageBodyPosition({})
        , pinoutDescriptor(pinoutDescriptor)
        , pinoutState(pinoutState)
    {
        assert((pinoutDescriptor.pinDescriptors.size() % 2) == 0);
        auto sortedPinDescriptors = std::vector<const Targets::TargetPinDescriptor*>{};
        sortedPinDescriptors.reserve(pinoutDescriptor.pinDescriptors.size());

        std::transform(
            pinoutDescriptor.pinDescriptors.begin(),
            pinoutDescriptor.pinDescriptors.end(),
            std::back_inserter(sortedPinDescriptors),
            [] (const auto& pinDescriptor) {
                return &pinDescriptor;
            }
        );

        std::sort(
            sortedPinDescriptors.begin(),
            sortedPinDescriptors.end(),
            [] (const Targets::TargetPinDescriptor* descA, const Targets::TargetPinDescriptor* descB) {
                return
                    Services::StringService::toUint16(descA->position, 10)
                        < Services::StringService::toUint16(descB->position, 10);
            }
        );

        this->bottomQuadrantLabelGroupSet->setParentItem(this);
        this->topQuadrantLabelGroupSet->setParentItem(this);

        const auto setSize = sortedPinDescriptors.size() / 2;

        for (auto i = std::size_t{0}; i < setSize; i++) {
            // Bottom pins
            const auto& pinDescriptor = *sortedPinDescriptors[i];
            auto* labelGroup = new VerticalLabelGroup{
                pinDescriptor,
                pinDescriptor.padKey.has_value()
                    ? std::optional{std::cref(targetDescriptor.getPadDescriptor(*(pinDescriptor.padKey)))}
                    : std::nullopt,
                this->pinoutState
            };

            labelGroup->setParentItem(this->bottomQuadrantLabelGroupSet);
            this->bottomQuadrantLabelGroupSet->labelGroups.emplace_back(labelGroup);

            auto* pin = new Pin{
                pinDescriptor,
                pinDescriptor.padKey.has_value()
                    ? std::optional{std::cref(targetDescriptor.getPadDescriptor(*(pinDescriptor.padKey)))}
                    : std::nullopt,
                Pin::Orientation::VERTICAL,
                this->pinoutState
            };
            pin->setParentItem(this);

            this->bottomLabelGroupPinPairs.emplace_back(
                VerticalLabelGroupPinPair{.labelGroup = labelGroup, .pin = pin}
            );
        }

        for (auto i = setSize; i < (setSize * 2); i++) {
            // Top pins
            const auto& pinDescriptor = *sortedPinDescriptors[i];
            auto* labelGroup = new VerticalLabelGroup{
                pinDescriptor,
                pinDescriptor.padKey.has_value()
                    ? std::optional{std::cref(targetDescriptor.getPadDescriptor(*(pinDescriptor.padKey)))}
                    : std::nullopt,
                this->pinoutState
            };

            labelGroup->setParentItem(this->topQuadrantLabelGroupSet);
            this->topQuadrantLabelGroupSet->labelGroups.emplace_back(labelGroup);

            auto* pin = new Pin{
                pinDescriptor,
                pinDescriptor.padKey.has_value()
                    ? std::optional{std::cref(targetDescriptor.getPadDescriptor(*(pinDescriptor.padKey)))}
                    : std::nullopt,
                Pin::Orientation::VERTICAL,
                this->pinoutState
            };
            pin->setParentItem(this);

            this->topLabelGroupPinPairs.emplace_back(
                VerticalLabelGroupPinPair{.labelGroup = labelGroup, .pin = pin}
            );
        }
    }

    std::vector<Pair<
        const Targets::TargetPadDescriptor&,
        LabelGroupInterface*
    >> DipPinout::padDescriptorLabelGroupPairs() {
        auto output = std::vector<Pair<const Targets::TargetPadDescriptor&, LabelGroupInterface*>>{};

        for (auto& [labelGroup, pin] : this->bottomLabelGroupPinPairs) {
            if (!pin->padDescriptor.has_value()) {
                continue;
            }

            output.emplace_back(pin->padDescriptor->get(), labelGroup);
        }

        for (auto& [labelGroup, pin] : this->topLabelGroupPinPairs) {
            if (!pin->padDescriptor.has_value()) {
                continue;
            }

            output.emplace_back(pin->padDescriptor->get(), labelGroup);
        }

        return output;
    }

    void DipPinout::refreshGeometry() {
        this->bottomQuadrantLabelGroupSet->refreshGeometry();
        this->topQuadrantLabelGroupSet->refreshGeometry();

        const auto setSize = this->pinoutDescriptor.pinDescriptors.size() / 2;
        const auto maxVerticalPinLineAHeight = static_cast<int>(
            DipPinout::MIN_LINE_A_LENGTH + (DipPinout::PIN_LINE_A_SPACING
                * std::ceil(static_cast<float>(setSize) / static_cast<float>(2)))
        );
        this->packageBodyPosition.setX(
            std::max(this->bottomQuadrantLabelGroupSet->size.width(), this->topQuadrantLabelGroupSet->size.width()) / 2
                - (this->packageBodySize.width() / 2)
        );
        this->packageBodyPosition.setY(
            this->topQuadrantLabelGroupSet->size.height()
                + maxVerticalPinLineAHeight + DipPinout::PIN_LINE_MARGIN + Pin::HEIGHT + DipPinout::PIN_MARGIN
        );

        this->size.setWidth(
            std::max(this->bottomQuadrantLabelGroupSet->size.width(), this->topQuadrantLabelGroupSet->size.width())
        );
        this->size.setHeight(
             this->topQuadrantLabelGroupSet->size.height() + this->packageBodySize.height()
                + ((maxVerticalPinLineAHeight + DipPinout::PIN_LINE_MARGIN + Pin::HEIGHT + DipPinout::PIN_MARGIN) * 2)
                + this->bottomQuadrantLabelGroupSet->size.height()
        );

        // Position the pins
        auto bottomPinXPosition = this->packageBodyPosition.x() + DipPinout::PACKAGE_PADDING;
        for (auto& [labelGroup, pin] : this->bottomLabelGroupPinPairs) {
            pin->setPos(
                bottomPinXPosition,
                this->packageBodyPosition.y() + this->packageBodySize.height() + DipPinout::PIN_MARGIN
            );

            bottomPinXPosition += Pin::WIDTH + DipPinout::PIN_MARGIN;
        }

        auto topPinXPosition = this->packageBodyPosition.x() + this->packageBodySize.width()
            - DipPinout::PACKAGE_PADDING - Pin::WIDTH;
        for (auto& [labelGroup, pin] : this->topLabelGroupPinPairs) {
            pin->setPos(
                topPinXPosition,
                this->packageBodyPosition.y() - DipPinout::PIN_MARGIN - Pin::HEIGHT
            );

            topPinXPosition -= Pin::WIDTH + DipPinout::PIN_MARGIN;
        }

        this->topQuadrantLabelGroupSet->setPos(
            this->packageBodyPosition.x() + (this->packageBodySize.width() / 2)
                - (this->topQuadrantLabelGroupSet->size.width() / 2),
            0
        );

        this->bottomQuadrantLabelGroupSet->setPos(
            this->packageBodyPosition.x() + (this->packageBodySize.width() / 2)
                - (this->bottomQuadrantLabelGroupSet->size.width() / 2),
            this->packageBodyPosition.y() + this->packageBodySize.height() + DipPinout::PIN_MARGIN + Pin::HEIGHT
                + DipPinout::PIN_LINE_MARGIN + maxVerticalPinLineAHeight
        );
    }

    QRectF DipPinout::boundingRect() const {
        return QRectF{QPointF{0, 0}, this->size};
    }

    void DipPinout::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
        static constexpr auto BODY_COLOR = QColor{0x7D, 0x7D, 0x7D};
        static constexpr auto DISABLED_BODY_COLOR = QColor{0x7D, 0x7D, 0x7D, 153};
        static constexpr auto BG_COLOR = QColor{0x37, 0x38, 0x35};
        static constexpr auto LINE_COLOR = QColor{0x5E, 0x5C, 0x59};
        static constexpr auto HIGHLIGHTED_LINE_COLOR = QColor{0x7D, 0x7D, 0x7D};

        painter->setRenderHints(QPainter::RenderHint::Antialiasing | QPainter::RenderHint::SmoothPixmapTransform, true);

        painter->setBrush(!this->isEnabled() ? DISABLED_BODY_COLOR : BODY_COLOR);
        painter->setPen(Qt::PenStyle::NoPen);

        // Package body
        painter->drawRect(
            this->packageBodyPosition.x(),
            this->packageBodyPosition.y(),
            this->packageBodySize.width(),
            this->packageBodySize.height()
        );

        painter->setBrush(BG_COLOR);
        painter->drawEllipse(
            this->packageBodyPosition.x() - (DipPinout::P1_EDGE_INDICATOR_DIAMETER / 2),
            this->packageBodyPosition.y() + (this->packageBodySize.height() / 2)
                - (DipPinout::P1_EDGE_INDICATOR_DIAMETER / 2),
            DipPinout::P1_EDGE_INDICATOR_DIAMETER,
            DipPinout::P1_EDGE_INDICATOR_DIAMETER
        );
        painter->drawEllipse(
            this->packageBodyPosition.x() + DipPinout::P1_INDICATOR_MARGIN,
            this->packageBodyPosition.y() + this->packageBodySize.height() - DipPinout::P1_INDICATOR_DIAMETER
                - DipPinout::P1_INDICATOR_MARGIN,
            DipPinout::P1_INDICATOR_DIAMETER,
            DipPinout::P1_INDICATOR_DIAMETER
        );

        painter->setRenderHints(QPainter::RenderHint::Antialiasing, false);

        // Pin lines
        const auto setSize = this->pinoutDescriptor.pinDescriptors.size() / 2;

        {
            // Bottom pin lines
            auto lineAEndYOffset = int{0};
            for (auto pinIndex = std::size_t{0}; pinIndex < this->bottomLabelGroupPinPairs.size(); ++pinIndex) {
                const auto* pin = this->bottomLabelGroupPinPairs[pinIndex].pin;
                const auto* labelGroup = this->bottomLabelGroupPinPairs[pinIndex].labelGroup;

                painter->setPen(
                    this->pinoutState.hoveredPinNumber.has_value() && this->pinoutState.hoveredPinNumber == pin->number
                        ? HIGHLIGHTED_LINE_COLOR
                        : LINE_COLOR
                );

                const auto lineAStartX = static_cast<int>(pin->pos().x() + (Pin::WIDTH / 2));
                const auto lineAStartY = static_cast<int>(pin->pos().y() + Pin::HEIGHT + DipPinout::PIN_LINE_MARGIN);
                const auto lineAEndY = static_cast<int>(lineAStartY + DipPinout::MIN_LINE_A_LENGTH + lineAEndYOffset);

                painter->drawLine(lineAStartX, lineAStartY, lineAStartX, lineAEndY);

                if (setSize % 2 != 0 || pinIndex != (setSize / 2 - 1)) {
                    lineAEndYOffset = pinIndex <= (setSize / 2 - 1)
                        ? lineAEndYOffset + DipPinout::PIN_LINE_A_SPACING
                        : lineAEndYOffset - DipPinout::PIN_LINE_A_SPACING;
                }

                const auto lineBStartX = lineAStartX;
                const auto lineBEndX = static_cast<int>(
                    this->bottomQuadrantLabelGroupSet->pos().x() + labelGroup->pos().x()
                        + (labelGroup->size.width() / 2)
                );
                const auto lineBStartY = lineAEndY;

                painter->drawLine(lineBStartX, lineBStartY, lineBEndX, lineBStartY);

                const auto lineCStartX = lineBEndX;
                const auto lineCStartY = lineBStartY;
                const auto lineCEndY = static_cast<int>(
                    this->bottomQuadrantLabelGroupSet->pos().y() + labelGroup->pos().y() - 5
                );

                painter->drawLine(lineCStartX, lineCStartY, lineCStartX, lineCEndY);
            }
        }

        {
            // Top pin lines
            auto lineAEndYOffset = int{0};
            for (auto pinIndex = std::size_t{0}; pinIndex < this->topLabelGroupPinPairs.size(); ++pinIndex) {
                const auto* pin = this->topLabelGroupPinPairs[pinIndex].pin;
                const auto* labelGroup = this->topLabelGroupPinPairs[pinIndex].labelGroup;

                painter->setPen(
                    this->pinoutState.hoveredPinNumber.has_value() && this->pinoutState.hoveredPinNumber == pin->number
                        ? HIGHLIGHTED_LINE_COLOR
                        : LINE_COLOR
                );

                const auto lineAStartX = static_cast<int>(pin->pos().x() + (Pin::WIDTH / 2));
                const auto lineAStartY = static_cast<int>(pin->pos().y() - DipPinout::PIN_LINE_MARGIN);
                const auto lineAEndY = lineAStartY - (DipPinout::MIN_LINE_A_LENGTH + lineAEndYOffset);

                painter->drawLine(lineAStartX, lineAStartY, lineAStartX, lineAEndY);

                if (setSize % 2 != 0 || pinIndex != (setSize / 2 - 1)) {
                    lineAEndYOffset = pinIndex <= (setSize / 2 - 1)
                        ? lineAEndYOffset + DipPinout::PIN_LINE_A_SPACING
                        : lineAEndYOffset - DipPinout::PIN_LINE_A_SPACING;
                }

                const auto lineBStartX = lineAStartX;
                const auto lineBEndX = static_cast<int>(
                    this->topQuadrantLabelGroupSet->pos().x() + labelGroup->pos().x()
                        + (labelGroup->size.width() / 2)
                );
                const auto lineBStartY = lineAEndY;

                painter->drawLine(lineBStartX, lineBStartY, lineBEndX, lineBStartY);

                const auto lineCStartX = lineBEndX;
                const auto lineCStartY = lineBStartY;
                const auto lineCEndY = static_cast<int>(
                    this->topQuadrantLabelGroupSet->pos().y() + labelGroup->pos().y() + labelGroup->size.height() + 5
                );

                painter->drawLine(lineCStartX, lineCStartY, lineCStartX, lineCEndY);
            }
        }
    }
}
