#include "DualInlinePackageWidget.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <vector>
#include <QEvent>
#include <QFile>

#include "src/Helpers/Paths.hpp"

using namespace Bloom::Widgets::InsightTargetWidgets::Dip;
using namespace Bloom::Exceptions;

using Bloom::Targets::TargetVariant;

DualInlinePackageWidget::DualInlinePackageWidget(
    const TargetVariant& targetVariant,
    InsightWorker& insightWorker,
    QWidget* parent
): TargetPackageWidget(targetVariant, insightWorker, parent) {
    auto stylesheetFile = QFile(QString::fromStdString(
            Paths::compiledResourcesPath()
            + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetWidgets/DIP/Stylesheets/DualInlinePackage.qss"
        )
    );
    stylesheetFile.open(QFile::ReadOnly);
    this->setStyleSheet(stylesheetFile.readAll());

    this->pinWidgets.reserve(targetVariant.pinDescriptorsByNumber.size());
    this->layout = new QVBoxLayout();
    this->layout->setSpacing(PinWidget::WIDTH_SPACING);
    this->layout->setContentsMargins(0, 0, 0, 0);
    this->layout->setAlignment(Qt::AlignmentFlag::AlignVCenter);

    this->topPinLayout = new QHBoxLayout();
    this->topPinLayout->setSpacing(PinWidget::WIDTH_SPACING);
    this->topPinLayout->setDirection(QBoxLayout::Direction::RightToLeft);
    this->topPinLayout->setAlignment(Qt::AlignmentFlag::AlignHCenter);
    this->bottomPinLayout = new QHBoxLayout();
    this->bottomPinLayout->setSpacing(PinWidget::WIDTH_SPACING);
    this->bottomPinLayout->setAlignment(Qt::AlignmentFlag::AlignHCenter);

    for (const auto& [targetPinNumber, targetPinDescriptor]: targetVariant.pinDescriptorsByNumber) {
        auto* pinWidget = new PinWidget(targetPinDescriptor, targetVariant, insightWorker, this);
        this->pinWidgets.push_back(pinWidget);
        TargetPackageWidget::pinWidgets.push_back(pinWidget);

        if (targetPinNumber <= (targetVariant.pinDescriptorsByNumber.size() / 2)) {
            this->bottomPinLayout->addWidget(pinWidget, 0, Qt::AlignmentFlag::AlignHCenter);
        } else {
            this->topPinLayout->addWidget(pinWidget, 0, Qt::AlignmentFlag::AlignHCenter);
        }
    }

    this->layout->addLayout(this->topPinLayout);
    this->bodyWidget = new BodyWidget(this);
    this->layout->addWidget(this->bodyWidget, 0, Qt::AlignmentFlag::AlignVCenter);
    this->layout->addLayout(this->bottomPinLayout);
    this->setLayout(this->layout);

    const auto bodyWidgetWidth = ((PinWidget::MINIMUM_WIDTH + PinWidget::WIDTH_SPACING)
        * static_cast<int>(this->pinWidgets.size() / 2)) - PinWidget::WIDTH_SPACING + 46;

    const auto width = bodyWidgetWidth;
    const auto height = (
        (
            PinWidget::MAXIMUM_HEIGHT + PinWidget::WIDTH_SPACING + PinWidget::PIN_LABEL_LONG_LINE_LENGTH
            + PinWidget::PIN_LABEL_SHORT_LINE_LENGTH + (
                (PinWidget::LABEL_HEIGHT + PinWidget::PIN_LABEL_SPACING) * 2
            )
        ) * 2) + BodyWidget::HEIGHT;

    this->bodyWidget->setGeometry(
        0,
        PinWidget::MAXIMUM_HEIGHT + PinWidget::WIDTH_SPACING,
        width,
        BodyWidget::HEIGHT
    );
    this->topPinLayout->setGeometry(QRect(0, 0, width, PinWidget::MAXIMUM_HEIGHT));
    this->bottomPinLayout->setGeometry(
        QRect(
            0,
            (PinWidget::MAXIMUM_HEIGHT + BodyWidget::HEIGHT + (PinWidget::WIDTH_SPACING * 2)),
            width,
            PinWidget::MAXIMUM_HEIGHT
        )
    );
    this->topPinLayout->setContentsMargins(23, 0, 23, 0);
    this->bottomPinLayout->setContentsMargins( 23, 0, 23, 0);

    this->setFixedSize(width, height);

    this->setGeometry(
        (parent->width() / 2) - (width / 2),
        (parent->height() / 2) - (height / 2),
        width,
        height
    );
}

void DualInlinePackageWidget::paintEvent(QPaintEvent* event) {
    auto painter = QPainter(this);
    this->drawWidget(painter);
}

void DualInlinePackageWidget::drawWidget(QPainter& painter) {
    using Targets::TargetPinState;

    static auto pinNameFont = QFont("'Ubuntu', sans-serif");
    static auto pinDirectionFont = pinNameFont;
    pinNameFont.setPixelSize(11);
    pinDirectionFont.setPixelSize(10);

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

        if (pinWidget->position == Position::TOP) {
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
                pinGeoPosition.x() + (PinWidget::MINIMUM_WIDTH / 2),
                pinGeoPosition.y() - pinNameLabelLineLength,
                pinGeoPosition.x() + (PinWidget::MINIMUM_WIDTH / 2),
                pinGeoPosition.y()
            ));

            painter.setPen(pinStateChanged ? pinChangedFontColor : pinNameFontColor);
            painter.drawText(
                QRect(
                    pinGeoPosition.x() + (PinWidget::MINIMUM_WIDTH / 2)
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
                    pinGeoPosition.x() + (PinWidget::MINIMUM_WIDTH / 2),
                    pinGeoPosition.y() - pinNameLabelLineLength - PinWidget::MAXIMUM_LABEL_HEIGHT
                        - pinDirectionLabelLineLength,
                    pinGeoPosition.x() + (PinWidget::MINIMUM_WIDTH / 2),
                    pinGeoPosition.y() - pinNameLabelLineLength - PinWidget::MAXIMUM_LABEL_HEIGHT
                ));

                painter.setPen(pinDirectionFontColor);
                painter.drawText(
                    QRect(
                        pinGeoPosition.x() + (PinWidget::MINIMUM_WIDTH / 2)
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
                pinGeoPosition.x() + (PinWidget::MINIMUM_WIDTH / 2),
                pinGeoPosition.y() + PinWidget::MAXIMUM_HEIGHT,
                pinGeoPosition.x() + (PinWidget::MINIMUM_WIDTH / 2),
                pinGeoPosition.y() + PinWidget::MAXIMUM_HEIGHT + pinNameLabelLineLength
            ));

            painter.setPen(pinStateChanged ? pinChangedFontColor : pinNameFontColor);
            painter.drawText(
                QRect(
                    pinGeoPosition.x() + (PinWidget::MINIMUM_WIDTH / 2)
                        - (PinWidget::MAXIMUM_LABEL_WIDTH / 2),
                    pinGeoPosition.y() + + PinWidget::MAXIMUM_HEIGHT + pinNameLabelLineLength,
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
                    pinGeoPosition.x() + (PinWidget::MINIMUM_WIDTH / 2),
                    pinGeoPosition.y() + PinWidget::MAXIMUM_HEIGHT + pinNameLabelLineLength
                        + PinWidget::MAXIMUM_LABEL_HEIGHT,
                    pinGeoPosition.x() + (PinWidget::MINIMUM_WIDTH / 2),
                    pinGeoPosition.y() + PinWidget::MAXIMUM_HEIGHT + pinNameLabelLineLength
                        + PinWidget::MAXIMUM_LABEL_HEIGHT + pinDirectionLabelLineLength
                ));

                painter.setPen(pinDirectionFontColor);
                painter.drawText(
                    QRect(
                        pinGeoPosition.x() + (PinWidget::MINIMUM_WIDTH / 2)
                            - (PinWidget::MAXIMUM_LABEL_WIDTH / 2),
                        pinGeoPosition.y() + PinWidget::MAXIMUM_HEIGHT + pinNameLabelLineLength
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
