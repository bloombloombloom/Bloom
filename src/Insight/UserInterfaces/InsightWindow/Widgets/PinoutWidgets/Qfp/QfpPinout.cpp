#include "QfpPinout.hpp"

#include <algorithm>
#include <numeric>

#include "src/Services/StringService.hpp"

namespace Widgets::PinoutWidgets
{
    QfpPinout::QfpPinout(
        const Targets::TargetPinoutDescriptor& pinoutDescriptor,
        const Targets::TargetDescriptor& targetDescriptor,
        const PinoutState& pinoutState
    )
        : packageBodySize({
            (Pin::WIDTH + QfpPinout::PIN_MARGIN) * static_cast<int>(pinoutDescriptor.pinDescriptors.size() / 4)
                - QfpPinout::PIN_MARGIN + (QfpPinout::PACKAGE_PADDING * 2),
            (Pin::WIDTH + QfpPinout::PIN_MARGIN) * static_cast<int>(pinoutDescriptor.pinDescriptors.size() / 4)
                - QfpPinout::PIN_MARGIN + (QfpPinout::PACKAGE_PADDING * 2)
        })
        , packageBodyPosition({})
        , pinoutDescriptor(pinoutDescriptor)
        , pinoutState(pinoutState)
    {
        assert((pinoutDescriptor.pinDescriptors.size() % 4) == 0);
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

        this->leftQuadrantLabelGroupSet->setParentItem(this);
        this->bottomQuadrantLabelGroupSet->setParentItem(this);
        this->rightQuadrantLabelGroupSet->setParentItem(this);
        this->topQuadrantLabelGroupSet->setParentItem(this);

        const auto quadSize = sortedPinDescriptors.size() / 4;

        for (auto i = std::size_t{0}; i < quadSize; i++) {
            // Left pins
            const auto& pinDescriptor = *sortedPinDescriptors[i];
            auto* labelGroup = new HorizontalLabelGroup{
                pinDescriptor,
                pinDescriptor.padKey.has_value()
                    ? std::optional{std::cref(targetDescriptor.getPadDescriptor(*(pinDescriptor.padKey)))}
                    : std::nullopt,
                HorizontalLabelGroup::Direction::LEFT,
                this->pinoutState
            };

            labelGroup->setParentItem(this->leftQuadrantLabelGroupSet);
            this->leftQuadrantLabelGroupSet->labelGroups.emplace_back(labelGroup);

            auto* pin = new Pin{
                pinDescriptor,
                pinDescriptor.padKey.has_value()
                    ? std::optional{std::cref(targetDescriptor.getPadDescriptor(*(pinDescriptor.padKey)))}
                    : std::nullopt,
                Pin::Orientation::HORIZONTAL,
                this->pinoutState
            };
            pin->setParentItem(this);

            this->leftLabelGroupPinPairs.emplace_back(
                HorizontalLabelGroupPinPair{.labelGroup = labelGroup, .pin = pin}
            );
        }

        for (auto i = quadSize; i < (quadSize * 2); i++) {
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

        for (auto i = quadSize * 2; i < (quadSize * 3); i++) {
            // Right pins
            const auto& pinDescriptor = *sortedPinDescriptors[i];
            auto* labelGroup = new HorizontalLabelGroup{
                pinDescriptor,
                pinDescriptor.padKey.has_value()
                    ? std::optional{std::cref(targetDescriptor.getPadDescriptor(*(pinDescriptor.padKey)))}
                    : std::nullopt,
                HorizontalLabelGroup::Direction::RIGHT,
                this->pinoutState
            };

            labelGroup->setParentItem(this->rightQuadrantLabelGroupSet);
            this->rightQuadrantLabelGroupSet->labelGroups.emplace_back(labelGroup);

            auto* pin = new Pin{
                pinDescriptor,
                pinDescriptor.padKey.has_value()
                    ? std::optional{std::cref(targetDescriptor.getPadDescriptor(*(pinDescriptor.padKey)))}
                    : std::nullopt,
                Pin::Orientation::HORIZONTAL,
                this->pinoutState
            };
            pin->setParentItem(this);

            this->rightLabelGroupPinPairs.emplace_back(
                HorizontalLabelGroupPinPair{.labelGroup = labelGroup, .pin = pin}
            );
        }

        for (auto i = quadSize * 3; i < (quadSize * 4); i++) {
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
    >> QfpPinout::padDescriptorLabelGroupPairs() {
        auto output = std::vector<Pair<const Targets::TargetPadDescriptor&, LabelGroupInterface*>>{};

        for (auto& [labelGroup, pin] : this->leftLabelGroupPinPairs) {
            if (!pin->padDescriptor.has_value()) {
                continue;
            }

            output.emplace_back(pin->padDescriptor->get(), labelGroup);
        }

        for (auto& [labelGroup, pin] : this->bottomLabelGroupPinPairs) {
            if (!pin->padDescriptor.has_value()) {
                continue;
            }

            output.emplace_back(pin->padDescriptor->get(), labelGroup);
        }

        for (auto& [labelGroup, pin] : this->rightLabelGroupPinPairs) {
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

    void QfpPinout::refreshGeometry() {
        this->leftQuadrantLabelGroupSet->refreshGeometry();
        this->bottomQuadrantLabelGroupSet->refreshGeometry();
        this->rightQuadrantLabelGroupSet->refreshGeometry();
        this->topQuadrantLabelGroupSet->refreshGeometry();

        const auto quadSize = this->pinoutDescriptor.pinDescriptors.size() / 4;
        const auto maxHorizontalPinLineAHeight = static_cast<int>(
            QfpPinout::MIN_LINE_A_LENGTH + (QfpPinout::PIN_LINE_A_SPACING
                * std::ceil(static_cast<float>(quadSize) / static_cast<float>(2)))
        );

        const auto maxVerticalPinLineAHeight = this->minVerticalLineALength() + static_cast<int>(
            QfpPinout::PIN_LINE_A_SPACING * std::ceil(static_cast<float>(quadSize) / static_cast<float>(2))
        );

        this->packageBodyPosition.setX(
            std::max(
                this->leftQuadrantLabelGroupSet->size.width() + maxHorizontalPinLineAHeight + QfpPinout::PIN_LINE_MARGIN + Pin::HEIGHT + QfpPinout::PIN_MARGIN,
                (std::max(this->bottomQuadrantLabelGroupSet->size.width(), this->topQuadrantLabelGroupSet->size.width()) / 2) - (this->packageBodySize.width() / 2)
            )
        );
        this->packageBodyPosition.setY(
            this->topQuadrantLabelGroupSet->size.height()
                + maxVerticalPinLineAHeight + QfpPinout::PIN_LINE_MARGIN + Pin::HEIGHT + QfpPinout::PIN_MARGIN
        );

        this->size.setWidth(
            std::max(
                std::max(
                    this->bottomQuadrantLabelGroupSet->size.width(),
                    this->topQuadrantLabelGroupSet->size.width()
                ),
                this->leftQuadrantLabelGroupSet->size.width() + this->packageBodySize.width() + (
                    (maxHorizontalPinLineAHeight + QfpPinout::PIN_LINE_MARGIN + Pin::HEIGHT + QfpPinout::PIN_MARGIN) * 2
                ) + this->rightQuadrantLabelGroupSet->size.width()
            )
        );
        this->size.setHeight(
             this->topQuadrantLabelGroupSet->size.height() + this->packageBodySize.height()
                + ((maxVerticalPinLineAHeight + QfpPinout::PIN_LINE_MARGIN + Pin::HEIGHT + QfpPinout::PIN_MARGIN) * 2)
                + this->bottomQuadrantLabelGroupSet->size.height()
        );

        // Position the pins
        auto leftPinYPosition = this->packageBodyPosition.y() + QfpPinout::PACKAGE_PADDING;
        for (auto& [labelGroup, pin] : this->leftLabelGroupPinPairs) {
            pin->setPos(
                this->packageBodyPosition.x() - Pin::HEIGHT - QfpPinout::PIN_MARGIN,
                leftPinYPosition
            );

            leftPinYPosition += Pin::WIDTH + QfpPinout::PIN_MARGIN;
        }

        auto bottomPinXPosition = this->packageBodyPosition.x() + QfpPinout::PACKAGE_PADDING;
        for (auto& [labelGroup, pin] : this->bottomLabelGroupPinPairs) {
            pin->setPos(
                bottomPinXPosition,
                this->packageBodyPosition.y() + this->packageBodySize.height() + QfpPinout::PIN_MARGIN
            );

            bottomPinXPosition += Pin::WIDTH + QfpPinout::PIN_MARGIN;
        }

        auto rightPinYPosition = this->packageBodyPosition.y() + this->packageBodySize.height()
            - QfpPinout::PACKAGE_PADDING - Pin::WIDTH;
        for (auto& [labelGroup, pin] : this->rightLabelGroupPinPairs) {
            pin->setPos(
                this->packageBodyPosition.x() + this->packageBodySize.width() + QfpPinout::PIN_MARGIN,
                rightPinYPosition
            );

            rightPinYPosition -= Pin::WIDTH + QfpPinout::PIN_MARGIN;
        }

        auto topPinXPosition = this->packageBodyPosition.x() + this->packageBodySize.width()
            - QfpPinout::PACKAGE_PADDING - Pin::WIDTH;
        for (auto& [labelGroup, pin] : this->topLabelGroupPinPairs) {
            pin->setPos(
                topPinXPosition,
                this->packageBodyPosition.y() - QfpPinout::PIN_MARGIN - Pin::HEIGHT
            );

            topPinXPosition -= Pin::WIDTH + QfpPinout::PIN_MARGIN;
        }

        this->leftQuadrantLabelGroupSet->setPos(
            this->packageBodyPosition.x() - QfpPinout::PIN_MARGIN - Pin::HEIGHT - QfpPinout::PIN_LINE_MARGIN
                - maxHorizontalPinLineAHeight - this->leftQuadrantLabelGroupSet->size.width(),
            this->packageBodyPosition.y() + (this->packageBodySize.height() / 2) - (this->leftQuadrantLabelGroupSet->size.height() / 2)
        );

        this->topQuadrantLabelGroupSet->setPos(
            this->packageBodyPosition.x() + (this->packageBodySize.width() / 2)
                - (this->topQuadrantLabelGroupSet->size.width() / 2),
            0
        );

        this->rightQuadrantLabelGroupSet->setPos(
            this->packageBodyPosition.x() + this->packageBodySize.width() + QfpPinout::PIN_MARGIN + Pin::HEIGHT + QfpPinout::PIN_LINE_MARGIN
                + maxHorizontalPinLineAHeight,
            this->packageBodyPosition.y() + (this->packageBodySize.height() / 2) - (this->leftQuadrantLabelGroupSet->size.height() / 2)
        );

        this->bottomQuadrantLabelGroupSet->setPos(
            this->packageBodyPosition.x() + (this->packageBodySize.width() / 2)
                - (this->bottomQuadrantLabelGroupSet->size.width() / 2),
            this->packageBodyPosition.y() + this->packageBodySize.height() + QfpPinout::PIN_MARGIN + Pin::HEIGHT
                + QfpPinout::PIN_LINE_MARGIN + maxVerticalPinLineAHeight
        );
    }

    QRectF QfpPinout::boundingRect() const {
        return QRectF{QPointF{0, 0}, this->size};
    }

    void QfpPinout::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
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
            this->packageBodyPosition.x() + 10,
            this->packageBodyPosition.y() + 10,
            14,
            14
        );

        painter->setRenderHints(QPainter::RenderHint::Antialiasing, false);

        // Pin lines
        const auto quadSize = this->pinoutDescriptor.pinDescriptors.size() / 4;

        {
            // Left pin lines
            auto lineAEndXOffset = int{0};
            for (auto pinIndex = std::size_t{0}; pinIndex < this->leftLabelGroupPinPairs.size(); ++pinIndex) {
                const auto* pin = this->leftLabelGroupPinPairs[pinIndex].pin;
                const auto* labelGroup = this->leftLabelGroupPinPairs[pinIndex].labelGroup;

                painter->setPen(
                    this->pinoutState.hoveredPinNumber.has_value() && this->pinoutState.hoveredPinNumber == pin->number
                        ? HIGHLIGHTED_LINE_COLOR
                        : LINE_COLOR
                );

                const auto lineAStartX = static_cast<int>(pin->pos().x() - QfpPinout::PIN_LINE_MARGIN);
                const auto lineAStartY = static_cast<int>(pin->pos().y() + (Pin::WIDTH / 2));
                const auto lineAEndX = static_cast<int>(lineAStartX - QfpPinout::MIN_LINE_A_LENGTH - lineAEndXOffset);

                painter->drawLine(lineAStartX, lineAStartY, lineAEndX, lineAStartY);

                if (quadSize % 2 != 0 || pinIndex != (quadSize / 2 - 1)) {
                    lineAEndXOffset = pinIndex <= (quadSize / 2 - 1)
                        ? lineAEndXOffset + QfpPinout::PIN_LINE_A_SPACING
                        : lineAEndXOffset - QfpPinout::PIN_LINE_A_SPACING;
                }

                const auto lineBStartX = lineAEndX;
                const auto lineBStartY = lineAStartY;
                const auto lineBEndY = static_cast<int>(
                    this->leftQuadrantLabelGroupSet->pos().y() + labelGroup->pos().y()
                        + (labelGroup->size.height() / 2)
                );

                painter->drawLine(lineBStartX, lineBStartY, lineBStartX, lineBEndY);

                const auto lineCStartX = lineBStartX;
                const auto lineCStartY = lineBEndY;
                const auto lineCEndX = static_cast<int>(
                    this->leftQuadrantLabelGroupSet->pos().x() + labelGroup->pos().x() + labelGroup->size.width() + QfpPinout::PIN_LINE_MARGIN
                );

                painter->drawLine(lineCStartX, lineCStartY, lineCEndX, lineCStartY);
            }
        }

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
                const auto lineAStartY = static_cast<int>(pin->pos().y() + Pin::HEIGHT + QfpPinout::PIN_LINE_MARGIN);
                const auto lineAEndY = static_cast<int>(lineAStartY + this->minVerticalLineALength() + lineAEndYOffset);

                painter->drawLine(lineAStartX, lineAStartY, lineAStartX, lineAEndY);

                if (quadSize % 2 != 0 || pinIndex != (quadSize / 2 - 1)) {
                    lineAEndYOffset = pinIndex <= (quadSize / 2 - 1)
                        ? lineAEndYOffset + QfpPinout::PIN_LINE_A_SPACING
                        : lineAEndYOffset - QfpPinout::PIN_LINE_A_SPACING;
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
            // Right pin lines
            auto lineAEndXOffset = int{0};
            for (auto pinIndex = std::size_t{0}; pinIndex < this->rightLabelGroupPinPairs.size(); ++pinIndex) {
                const auto* pin = this->rightLabelGroupPinPairs[pinIndex].pin;
                const auto* labelGroup = this->rightLabelGroupPinPairs[pinIndex].labelGroup;

                painter->setPen(
                    this->pinoutState.hoveredPinNumber.has_value() && this->pinoutState.hoveredPinNumber == pin->number
                        ? HIGHLIGHTED_LINE_COLOR
                        : LINE_COLOR
                );

                const auto lineAStartX = static_cast<int>(pin->pos().x() + Pin::HEIGHT + QfpPinout::PIN_LINE_MARGIN);
                const auto lineAStartY = static_cast<int>(pin->pos().y() + (Pin::WIDTH / 2));
                const auto lineAEndX = static_cast<int>(lineAStartX + QfpPinout::MIN_LINE_A_LENGTH + lineAEndXOffset);

                painter->drawLine(lineAStartX, lineAStartY, lineAEndX, lineAStartY);

                if (quadSize % 2 != 0 || pinIndex != (quadSize / 2 - 1)) {
                    lineAEndXOffset = pinIndex <= (quadSize / 2 - 1)
                        ? lineAEndXOffset + QfpPinout::PIN_LINE_A_SPACING
                        : lineAEndXOffset - QfpPinout::PIN_LINE_A_SPACING;
                }

                const auto lineBStartX = lineAEndX;
                const auto lineBStartY = lineAStartY;
                const auto lineBEndY = static_cast<int>(
                    this->rightQuadrantLabelGroupSet->pos().y() + labelGroup->pos().y()
                        + (labelGroup->size.height() / 2)
                );

                painter->drawLine(lineBStartX, lineBStartY, lineBStartX, lineBEndY);

                const auto lineCStartX = lineBStartX;
                const auto lineCStartY = lineBEndY;
                const auto lineCEndX = static_cast<int>(
                    this->rightQuadrantLabelGroupSet->pos().x() + labelGroup->pos().x() - QfpPinout::PIN_LINE_MARGIN
                );

                painter->drawLine(lineCStartX, lineCStartY, lineCEndX, lineCStartY);
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
                const auto lineAStartY = static_cast<int>(pin->pos().y() - QfpPinout::PIN_LINE_MARGIN);
                const auto lineAEndY = lineAStartY - (this->minVerticalLineALength() + lineAEndYOffset);

                painter->drawLine(lineAStartX, lineAStartY, lineAStartX, lineAEndY);

                if (quadSize % 2 != 0 || pinIndex != (quadSize / 2 - 1)) {
                    lineAEndYOffset = pinIndex <= (quadSize / 2 - 1)
                        ? lineAEndYOffset + QfpPinout::PIN_LINE_A_SPACING
                        : lineAEndYOffset - QfpPinout::PIN_LINE_A_SPACING;
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

    int QfpPinout::minVerticalLineALength() const {
        return std::max(
            (
                this->leftQuadrantLabelGroupSet->size.height() - (this->packageBodySize.height()
                    + ((QfpPinout::PIN_MARGIN + Pin::HEIGHT + QfpPinout::PIN_LINE_MARGIN) * 2)
                ) + 10
            ) / 2,
            0
        ) + QfpPinout::MIN_LINE_A_LENGTH;
    }
}
