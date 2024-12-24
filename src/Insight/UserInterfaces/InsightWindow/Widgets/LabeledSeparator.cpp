#include "LabeledSeparator.hpp"

namespace Widgets
{
    LabeledSeparator::LabeledSeparator(QString title, QWidget* parent)
        : QWidget(parent)
        , title(std::move(title))
    {
        this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        this->setFixedHeight(LabeledSeparator::DEFAULT_HEIGHT);
    }

    void LabeledSeparator::paintEvent(QPaintEvent* event) {
        auto painter = QPainter{this};
        this->drawWidget(painter);
    }

    void LabeledSeparator::drawWidget(QPainter& painter) {
        const auto fontMetrics = painter.fontMetrics();
        const auto titleSize = fontMetrics.size(Qt::TextFlag::TextSingleLine, this->title);
        const auto titleRect = QRect{
            QPoint{this->marginLeft, (this->height() - titleSize.height()) / 2},
            titleSize
        };

        const auto lineYPosition = titleRect.y() + (titleRect.height() / 2);
        const auto line = QLine{
            titleRect.right() + 8,
            lineYPosition,
            this->width() - this->marginRight,
            lineYPosition
        };

        painter.drawText(titleRect, Qt::AlignCenter, this->title);

        painter.setPen(this->lineColor);
        painter.drawLine(line);
    }
}
