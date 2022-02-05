#include "QuadFlatPackageWidget.hpp"

#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <vector>
#include <QFile>
#include <QLine>

#include "src/Helpers/Paths.hpp"
#include "PinWidget.hpp"
#include "BodyWidget.hpp"

namespace Bloom::Widgets::InsightTargetWidgets::Qfp
{
    using namespace Bloom::Targets;

    QuadFlatPackageWidget::QuadFlatPackageWidget(
        const TargetVariant& targetVariant,
        InsightWorker& insightWorker,
        QWidget* parent
    ): TargetPackageWidget(targetVariant, insightWorker, parent) {
        assert((targetVariant.pinDescriptorsByNumber.size() % 4) == 0);

        auto stylesheetFile = QFile(QString::fromStdString(
                Paths::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetWidgets/QFP/Stylesheets/QuadFlatPackage.qss"
            )
        );
        stylesheetFile.open(QFile::ReadOnly);
        this->setStyleSheet(stylesheetFile.readAll());

        this->pinWidgets.reserve(targetVariant.pinDescriptorsByNumber.size());
        this->layout = new QVBoxLayout();
        this->layout->setSpacing(0);
        this->layout->setContentsMargins(0, 0, 0, 0);

        this->horizontalLayout = new QHBoxLayout();
        this->horizontalLayout->setSpacing(0);
        this->horizontalLayout->setDirection(QBoxLayout::Direction::LeftToRight);
        this->horizontalLayout->setAlignment(Qt::AlignmentFlag::AlignHCenter);

        this->topPinLayout = new QHBoxLayout();
        this->topPinLayout->setSpacing(PinWidget::WIDTH_SPACING);
        this->topPinLayout->setDirection(QBoxLayout::Direction::RightToLeft);
        this->topPinLayout->setAlignment(Qt::AlignmentFlag::AlignHCenter);

        this->rightPinLayout = new QVBoxLayout();
        this->rightPinLayout->setSpacing(PinWidget::WIDTH_SPACING);
        this->rightPinLayout->setDirection(QBoxLayout::Direction::BottomToTop);
        this->rightPinLayout->setAlignment(Qt::AlignmentFlag::AlignVCenter);

        this->bottomPinLayout = new QHBoxLayout();
        this->bottomPinLayout->setSpacing(PinWidget::WIDTH_SPACING);
        this->bottomPinLayout->setDirection(QBoxLayout::Direction::LeftToRight);
        this->bottomPinLayout->setAlignment(Qt::AlignmentFlag::AlignHCenter);

        this->leftPinLayout = new QVBoxLayout();
        this->leftPinLayout->setSpacing(PinWidget::WIDTH_SPACING);
        this->leftPinLayout->setDirection(QBoxLayout::Direction::TopToBottom);
        this->leftPinLayout->setAlignment(Qt::AlignmentFlag::AlignVCenter);

        const auto pinCountPerLayout = static_cast<int>(targetVariant.pinDescriptorsByNumber.size() / 4);
        for (const auto& [targetPinNumber, targetPinDescriptor]: targetVariant.pinDescriptorsByNumber) {
            auto* pinWidget = new PinWidget(targetPinDescriptor, targetVariant, insightWorker, this);
            this->pinWidgets.push_back(pinWidget);
            TargetPackageWidget::pinWidgets.push_back(pinWidget);

            if (pinWidget->position == Position::LEFT) {
                this->leftPinLayout->addWidget(pinWidget, 0, Qt::AlignmentFlag::AlignRight);

            } else if (pinWidget->position == Position::BOTTOM) {
                this->bottomPinLayout->addWidget(pinWidget, 0, Qt::AlignmentFlag::AlignTop);

            } else if (pinWidget->position == Position::RIGHT) {
                this->rightPinLayout->addWidget(pinWidget, 0, Qt::AlignmentFlag::AlignLeft);

            } else if (pinWidget->position == Position::TOP) {
                this->topPinLayout->addWidget(pinWidget, 0, Qt::AlignmentFlag::AlignBottom);
            }
        }

        this->bodyWidget = new BodyWidget(this);
        this->layout->addLayout(this->topPinLayout);
        this->horizontalLayout->addLayout(this->leftPinLayout);
        this->horizontalLayout->addWidget(this->bodyWidget);
        this->horizontalLayout->addLayout(this->rightPinLayout);
        this->layout->addLayout(this->horizontalLayout);
        this->layout->addLayout(this->bottomPinLayout);
        this->setLayout(this->layout);

        // Layout sizing, positioning and padding
        const auto verticalPinWidgetHeight = PinWidget::MAXIMUM_VERTICAL_HEIGHT;
        const auto verticalPinWidgetWidth = PinWidget::MAXIMUM_VERTICAL_WIDTH;
        const auto horizontalPinWidgetHeight = PinWidget::MAXIMUM_HORIZONTAL_HEIGHT;
        const auto horizontalPinWidgetWidth = PinWidget::MAXIMUM_HORIZONTAL_WIDTH;

        /*
         * Horizontal layouts are the right and left pin layouts - the ones that hold horizontal pin widgets.
         * The bottom and top layouts are vertical layouts, as they hold the vertical pin widgets.
         */
        const auto horizontalLayoutHeight = ((horizontalPinWidgetHeight + PinWidget::WIDTH_SPACING) * pinCountPerLayout
            + PinWidget::PIN_WIDGET_LAYOUT_PADDING - PinWidget::WIDTH_SPACING);

        const auto verticalLayoutWidth = ((verticalPinWidgetWidth + PinWidget::WIDTH_SPACING) * pinCountPerLayout
            + PinWidget::PIN_WIDGET_LAYOUT_PADDING - PinWidget::WIDTH_SPACING);

        const auto height = horizontalLayoutHeight + (verticalPinWidgetHeight * 2) + (
            (
                PinWidget::PIN_LABEL_LONG_LINE_LENGTH
                + PinWidget::PIN_LABEL_SHORT_LINE_LENGTH
                + (PinWidget::MAXIMUM_LABEL_HEIGHT * 2)
                + (PinWidget::PIN_LABEL_SPACING * 3)
            ) * 2
        );
        const auto width = verticalLayoutWidth + (horizontalPinWidgetWidth * 2) + (
            (
                PinWidget::PIN_LABEL_LONG_LINE_LENGTH
                + PinWidget::MAXIMUM_LABEL_WIDTH
                + PinWidget::MAXIMUM_PIN_DIRECTION_LABEL_WIDTH
                + (PinWidget::PIN_LABEL_SPACING * 2)
            ) * 2
        );

        this->topPinLayout->insertSpacing(0, horizontalPinWidgetWidth);
        this->topPinLayout->addSpacing(horizontalPinWidgetWidth);

        this->bottomPinLayout->insertSpacing(0, horizontalPinWidgetWidth);
        this->bottomPinLayout->addSpacing(horizontalPinWidgetWidth);

        this->leftPinLayout->setGeometry(QRect(
            0,
            verticalPinWidgetHeight,
            horizontalPinWidgetWidth,
            horizontalLayoutHeight
        ));

        this->bodyWidget->setFixedSize(verticalLayoutWidth, horizontalLayoutHeight);

        this->rightPinLayout->setGeometry(QRect(
            horizontalLayoutHeight + horizontalPinWidgetWidth,
            verticalPinWidgetHeight,
            horizontalPinWidgetWidth,
            horizontalLayoutHeight
        ));

        const auto pinWidgetLayoutMargin = PinWidget::PIN_WIDGET_LAYOUT_PADDING / 2;

        this->topPinLayout->setContentsMargins(
            pinWidgetLayoutMargin,
            0,
            pinWidgetLayoutMargin,
            0
        );

        this->bottomPinLayout->setContentsMargins(
            pinWidgetLayoutMargin,
            0,
            pinWidgetLayoutMargin,
            0
        );

        this->leftPinLayout->setContentsMargins(
            0,
            pinWidgetLayoutMargin,
            0,
            pinWidgetLayoutMargin
        );

        this->rightPinLayout->setContentsMargins(
            0,
            pinWidgetLayoutMargin,
            0,
            pinWidgetLayoutMargin
        );

        this->setFixedSize(width, height);

        // Set the fixed size and center the widget
        this->setGeometry(
            (parent->width() / 2) - (width / 2),
            (parent->height() / 2) - (height / 2),
            width,
            height
        );
    }

    void QuadFlatPackageWidget::paintEvent(QPaintEvent* event) {
        auto painter = QPainter(this);
        this->drawWidget(painter);
    }

    void QuadFlatPackageWidget::drawWidget(QPainter& painter) {
        static auto pinNameFont = QFont("'Ubuntu', sans-serif");
        static auto pinDirectionFont = pinNameFont;
        pinNameFont.setPixelSize(11);
        pinDirectionFont.setPixelSize(9);

        static const auto lineColor = QColor(0x4F, 0x4F, 0x4F);
        static const auto pinNameFontColor = QColor(0xA6, 0xA7, 0xAA);
        static const auto pinDirectionFontColor = QColor(0x8A, 0x8A, 0x8D);
        static const auto pinChangedFontColor = QColor(0x4D, 0x7B, 0xBA);

        static const auto inDirectionText = QString("IN");
        static const auto outDirectionText = QString("OUT");

        for (const auto* pinWidget : this->pinWidgets) {
            const auto pinGeoPosition = pinWidget->pos();
            const auto& pinState = pinWidget->getPinState();
            const auto pinStateChanged = pinWidget->hasPinStateChanged();

            painter.setFont(pinNameFont);

            if (pinWidget->position == Position::LEFT) {
                painter.setPen(lineColor);
                painter.drawLine(QLine(
                    pinGeoPosition.x() - PinWidget::PIN_LABEL_LONG_LINE_LENGTH,
                    pinGeoPosition.y() + (PinWidget::MAXIMUM_HORIZONTAL_HEIGHT / 2),
                    pinGeoPosition.x(),
                    pinGeoPosition.y() + (PinWidget::MAXIMUM_HORIZONTAL_HEIGHT / 2)
                ));

                painter.setPen(pinStateChanged ? pinChangedFontColor : pinNameFontColor);
                painter.drawText(
                    QRect(
                        pinGeoPosition.x() - PinWidget::PIN_LABEL_LONG_LINE_LENGTH
                            - PinWidget::MAXIMUM_LABEL_WIDTH - (PinWidget::PIN_LABEL_SPACING * 2),
                        pinGeoPosition.y() + (PinWidget::MAXIMUM_VERTICAL_WIDTH / 2)
                            - (PinWidget::MAXIMUM_LABEL_HEIGHT / 2),
                        PinWidget::MAXIMUM_LABEL_WIDTH,
                        PinWidget::MAXIMUM_LABEL_HEIGHT
                    ),
                    Qt::AlignCenter,
                    pinWidget->pinNameLabelText
                );

                if (pinState.has_value() && pinState->ioDirection.has_value()) {
                    painter.setFont(pinDirectionFont);

                    painter.setPen(pinDirectionFontColor);
                    painter.drawText(
                        QRect(
                            pinGeoPosition.x() - PinWidget::PIN_LABEL_LONG_LINE_LENGTH
                                - PinWidget::MAXIMUM_LABEL_WIDTH - (PinWidget::PIN_LABEL_SPACING * 3)
                                - PinWidget::MAXIMUM_PIN_DIRECTION_LABEL_WIDTH,
                            pinGeoPosition.y() + (PinWidget::MAXIMUM_VERTICAL_WIDTH / 2)
                                - (PinWidget::MAXIMUM_LABEL_HEIGHT / 2),
                            PinWidget::MAXIMUM_PIN_DIRECTION_LABEL_WIDTH,
                            PinWidget::MAXIMUM_LABEL_HEIGHT
                        ),
                        Qt::AlignCenter,
                        pinState->ioDirection == TargetPinState::IoDirection::INPUT ? inDirectionText : outDirectionText
                    );
                }

            } else if (pinWidget->position == Position::RIGHT) {
                painter.setPen(lineColor);
                painter.drawLine(QLine(
                    pinGeoPosition.x() + PinWidget::MAXIMUM_HORIZONTAL_WIDTH,
                    pinGeoPosition.y() + (PinWidget::MAXIMUM_VERTICAL_WIDTH / 2),
                    pinGeoPosition.x() + PinWidget::MAXIMUM_HORIZONTAL_WIDTH
                        + PinWidget::PIN_LABEL_LONG_LINE_LENGTH,
                    pinGeoPosition.y() + (PinWidget::MAXIMUM_VERTICAL_WIDTH / 2)
                ));

                painter.setPen(pinStateChanged ? pinChangedFontColor : pinNameFontColor);
                painter.drawText(
                    QRect(
                        pinGeoPosition.x() + PinWidget::MAXIMUM_HORIZONTAL_WIDTH
                            + PinWidget::PIN_LABEL_LONG_LINE_LENGTH + 8,
                        pinGeoPosition.y() + (PinWidget::MAXIMUM_VERTICAL_WIDTH / 2)
                            - (PinWidget::MAXIMUM_LABEL_HEIGHT / 2),
                        PinWidget::MAXIMUM_LABEL_WIDTH,
                        PinWidget::MAXIMUM_LABEL_HEIGHT
                    ),
                    Qt::AlignCenter,
                    pinWidget->pinNameLabelText
                );

                if (pinState.has_value() && pinState->ioDirection.has_value()) {
                    painter.setFont(pinDirectionFont);

                    painter.setPen(pinDirectionFontColor);
                    painter.drawText(
                        QRect(
                            pinGeoPosition.x() + PinWidget::MAXIMUM_HORIZONTAL_WIDTH
                                + PinWidget::PIN_LABEL_LONG_LINE_LENGTH + PinWidget::MAXIMUM_LABEL_WIDTH
                                + (PinWidget::PIN_LABEL_SPACING * 3),
                            pinGeoPosition.y() + (PinWidget::MAXIMUM_VERTICAL_WIDTH / 2)
                                - (PinWidget::MAXIMUM_LABEL_HEIGHT / 2),
                            PinWidget::MAXIMUM_PIN_DIRECTION_LABEL_WIDTH,
                            PinWidget::MAXIMUM_LABEL_HEIGHT
                        ),
                        Qt::AlignCenter,
                        pinState->ioDirection == TargetPinState::IoDirection::INPUT ? inDirectionText : outDirectionText
                    );
                }

            } else if (pinWidget->position == Position::TOP) {
                painter.setPen(lineColor);
                const auto pinNameLabelLineLength = (pinWidget->getPinNumber() % 2 == 0 ?
                    PinWidget::PIN_LABEL_LONG_LINE_LENGTH
                    : PinWidget::PIN_LABEL_SHORT_LINE_LENGTH
                );
                const auto pinDirectionLabelLineLength = (pinWidget->getPinNumber() % 2 == 0 ?
                    PinWidget::PIN_LABEL_SHORT_LINE_LENGTH
                    : PinWidget::PIN_LABEL_LONG_LINE_LENGTH
                );

                painter.drawLine(QLine(
                    pinGeoPosition.x() + (PinWidget::MAXIMUM_VERTICAL_WIDTH / 2),
                    pinGeoPosition.y() - pinNameLabelLineLength,
                    pinGeoPosition.x() + (PinWidget::MAXIMUM_VERTICAL_WIDTH / 2),
                    pinGeoPosition.y()
                ));

                painter.setPen(pinStateChanged ? pinChangedFontColor : pinNameFontColor);
                painter.drawText(
                    QRect(
                        pinGeoPosition.x() + (PinWidget::MAXIMUM_VERTICAL_WIDTH / 2)
                            - (PinWidget::MAXIMUM_LABEL_WIDTH / 2),
                        pinGeoPosition.y() - pinNameLabelLineLength - PinWidget::MAXIMUM_LABEL_HEIGHT,
                        PinWidget::MAXIMUM_LABEL_WIDTH,
                        PinWidget::MAXIMUM_LABEL_HEIGHT
                    ),
                    Qt::AlignCenter,
                    pinWidget->pinNameLabelText
                );

                if (pinState.has_value() && pinState->ioDirection.has_value()) {
                    painter.setFont(pinDirectionFont);

                    painter.setPen(lineColor);
                    painter.drawLine(QLine(
                        pinGeoPosition.x() + (PinWidget::MAXIMUM_VERTICAL_WIDTH / 2),
                        pinGeoPosition.y() - pinNameLabelLineLength - PinWidget::MAXIMUM_LABEL_HEIGHT
                            - pinDirectionLabelLineLength,
                        pinGeoPosition.x() + (PinWidget::MAXIMUM_VERTICAL_WIDTH / 2),
                        pinGeoPosition.y() - pinNameLabelLineLength - PinWidget::MAXIMUM_LABEL_HEIGHT
                    ));

                    painter.setPen(pinDirectionFontColor);
                    painter.drawText(
                        QRect(
                            pinGeoPosition.x() + (PinWidget::MAXIMUM_VERTICAL_WIDTH / 2)
                                - (PinWidget::MAXIMUM_LABEL_WIDTH / 2),
                            pinGeoPosition.y() - pinNameLabelLineLength - pinDirectionLabelLineLength
                                - (PinWidget::MAXIMUM_LABEL_HEIGHT * 2),
                            PinWidget::MAXIMUM_LABEL_WIDTH,
                            PinWidget::MAXIMUM_LABEL_HEIGHT
                        ),
                        Qt::AlignCenter,
                        pinState->ioDirection == TargetPinState::IoDirection::INPUT ? inDirectionText : outDirectionText
                    );
                }

            } else if (pinWidget->position == Position::BOTTOM) {
                painter.setPen(lineColor);
                const auto pinNameLabelLineLength = (pinWidget->getPinNumber() % 2 == 0 ?
                    PinWidget::PIN_LABEL_LONG_LINE_LENGTH
                    : PinWidget::PIN_LABEL_SHORT_LINE_LENGTH
                );
                const auto pinDirectionLabelLineLength = (pinWidget->getPinNumber() % 2 == 0 ?
                    PinWidget::PIN_LABEL_SHORT_LINE_LENGTH
                    : PinWidget::PIN_LABEL_LONG_LINE_LENGTH
                );

                painter.drawLine(QLine(
                    pinGeoPosition.x() + (PinWidget::MAXIMUM_VERTICAL_WIDTH / 2),
                    pinGeoPosition.y() + PinWidget::MAXIMUM_VERTICAL_HEIGHT,
                    pinGeoPosition.x() + (PinWidget::MAXIMUM_VERTICAL_WIDTH / 2),
                    pinGeoPosition.y() + PinWidget::MAXIMUM_VERTICAL_HEIGHT + pinNameLabelLineLength
                ));

                painter.setPen(pinStateChanged ? pinChangedFontColor : pinNameFontColor);
                painter.drawText(
                    QRect(
                        pinGeoPosition.x() + (PinWidget::MAXIMUM_VERTICAL_WIDTH / 2)
                            - (PinWidget::MAXIMUM_LABEL_WIDTH / 2),
                        pinGeoPosition.y() + + PinWidget::MAXIMUM_VERTICAL_HEIGHT + pinNameLabelLineLength,
                        PinWidget::MAXIMUM_LABEL_WIDTH,
                        PinWidget::MAXIMUM_LABEL_HEIGHT
                    ),
                    Qt::AlignCenter,
                    pinWidget->pinNameLabelText
                );

                if (pinState.has_value() && pinState->ioDirection.has_value()) {
                    painter.setFont(pinDirectionFont);

                    painter.setPen(lineColor);
                    painter.drawLine(QLine(
                        pinGeoPosition.x() + (PinWidget::MAXIMUM_VERTICAL_WIDTH / 2),
                        pinGeoPosition.y() + PinWidget::MAXIMUM_VERTICAL_HEIGHT + pinNameLabelLineLength
                            + PinWidget::MAXIMUM_LABEL_HEIGHT,
                        pinGeoPosition.x() + (PinWidget::MAXIMUM_VERTICAL_WIDTH / 2),
                        pinGeoPosition.y() + PinWidget::MAXIMUM_VERTICAL_HEIGHT + pinNameLabelLineLength
                            + PinWidget::MAXIMUM_LABEL_HEIGHT + pinDirectionLabelLineLength
                    ));

                    painter.setPen(pinDirectionFontColor);
                    painter.drawText(
                        QRect(
                            pinGeoPosition.x() + (PinWidget::MAXIMUM_VERTICAL_WIDTH / 2)
                                - (PinWidget::MAXIMUM_LABEL_WIDTH / 2),
                            pinGeoPosition.y() + PinWidget::MAXIMUM_VERTICAL_HEIGHT + pinNameLabelLineLength
                                + PinWidget::MAXIMUM_LABEL_HEIGHT + pinDirectionLabelLineLength,
                            PinWidget::MAXIMUM_LABEL_WIDTH,
                            PinWidget::MAXIMUM_LABEL_HEIGHT
                        ),
                        Qt::AlignCenter,
                        pinState->ioDirection == TargetPinState::IoDirection::INPUT ? inDirectionText : outDirectionText
                    );
                }
            }
        }
    }
}
