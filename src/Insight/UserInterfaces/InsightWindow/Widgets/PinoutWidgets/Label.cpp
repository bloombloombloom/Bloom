#include "Label.hpp"

namespace Widgets::PinoutWidgets
{
    void Label::refreshGeometry() {
        this->minimumWidth = Label::FONT_METRICS.boundingRect(this->text()).width() + (Label::HORIZ_PADDING * 2);
        this->width = this->minimumWidth;
    }

    void Label::paint(QPainter* painter, int startX, int startY) const {
        painter->setFont(Label::FONT);

        painter->setBrush(this->rectColor());
        painter->setPen(Qt::PenStyle::NoPen);

        const auto rect = QRect{startX, startY, this->width, Label::HEIGHT};
        painter->drawRect(rect);

        painter->setPen(this->changed ? this->changedFontColor() : this->fontColor());
        painter->drawText(rect, Qt::AlignCenter, this->text());
    }

    const QColor& Label::rectColor() const {
        static constexpr auto RECT_COLOR = QColor{0x42, 0x42, 0x42};
        return RECT_COLOR;
    }

    const QColor& Label::fontColor() const {
        static constexpr auto FONT_COLOR = QColor{0xA2, 0xA2, 0xA2};
        return FONT_COLOR;
    }

    const QColor& Label::changedFontColor() const {
        /*
         * I spent a considerable amount of time attempting to find a good font color for highlighting changes.
         * I failed miserably. Have given up, for now.
         *
         * TODO: Revisit after v2.0.0
         */
        return this->fontColor();

        // static constexpr auto FONT_COLOR = QColor{0x54, 0x7F, 0xBA};
        // return FONT_COLOR;
    }
}
