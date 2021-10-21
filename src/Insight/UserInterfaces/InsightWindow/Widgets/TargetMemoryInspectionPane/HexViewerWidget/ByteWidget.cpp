#include "ByteWidget.hpp"

#include <QPainter>
#include <QStyle>

using namespace Bloom::Widgets;

ByteWidget::ByteWidget(
    std::size_t byteIndex,
    std::uint32_t address,
    std::optional<ByteWidget*>& hoveredByteWidget,
    QWidget* parent
): ClickableWidget(parent), byteIndex(byteIndex), address(address), hoveredByteWidget(hoveredByteWidget) {
    this->setObjectName("byte");
    auto onClick = [this] {
        this->setSelected(true);
    };

    this->addressHex = "0x" + QString::number(this->address, 16).rightJustified(8, '0').toUpper();
    this->relativeAddressHex = "0x" + QString::number(this->byteIndex, 16).rightJustified(8, '0').toUpper();

    QObject::connect(this, &ClickableWidget::clicked, this, onClick);
    QObject::connect(this, &ClickableWidget::rightClicked, this, onClick);

    this->setSelected(false);
}

void ByteWidget::setValue(unsigned char value) {
    this->valueChanged = this->valueInitialised && this->value != value;

    this->value = value;
    this->hexValue = QString::number(this->value, 16).rightJustified(2, '0').toUpper();
    this->asciiValue = (this->value >= 32 && this->value <= 126)
        ? std::optional(QString(QChar(this->value))) : std::nullopt;

    this->valueInitialised = true;
}

void ByteWidget::setSelected(bool selected) {
    this->setProperty("selected", selected);
    this->style()->unpolish(this);
    this->style()->polish(this);

    if (selected) {
        emit this->selected(this);
    }

    this->postSetSelected(selected);
}

bool ByteWidget::event(QEvent* event) {
    if (this->isEnabled()) {
        switch (event->type()) {
            case QEvent::Enter: {
                this->hoverActive = true;
                emit this->enter(this);
                this->update();
                break;
            }
            case QEvent::Leave: {
                this->hoverActive = false;
                emit this->leave(this);
                this->update();
                break;
            }
            default: {
                break;
            }
        }
    }

    return QWidget::event(event);
}

void ByteWidget::paintEvent(QPaintEvent* event) {
    auto painter = QPainter(this);
    this->drawWidget(painter);
}

void ByteWidget::drawWidget(QPainter& painter) {
    painter.setRenderHints(QPainter::RenderHint::Antialiasing | QPainter::RenderHint::SmoothPixmapTransform, true);
    painter.setPen(Qt::PenStyle::NoPen);

    static const auto widgetRect = QRect(0, 0, ByteWidget::WIDTH, ByteWidget::HEIGHT);

    if (this->hoveredByteWidget.has_value()
        && (
            this->hoveredByteWidget.value()->currentColumnIndex == this->currentColumnIndex
            || this->hoveredByteWidget.value()->currentRowIndex == this->currentRowIndex
        )
    ) {
        painter.setBrush(QColor(0x8E, 0x8B, 0x83, this->hoverActive ? 70 : 30));
        painter.drawRect(widgetRect);
    }

    auto textColor = QColor(this->valueChanged ? "#547fba" : "#afb1b3");

    if (this->valueInitialised) {

        if (!this->isEnabled()) {
            textColor.setAlpha(100);
        }

        painter.setPen(textColor);
        painter.drawText(widgetRect, Qt::AlignCenter, this->hexValue);

    } else {
        textColor.setAlpha(100);
        painter.setPen(textColor);

        static const auto placeholderString = QString("??");
        painter.drawText(widgetRect, Qt::AlignCenter, placeholderString);
    }
}
