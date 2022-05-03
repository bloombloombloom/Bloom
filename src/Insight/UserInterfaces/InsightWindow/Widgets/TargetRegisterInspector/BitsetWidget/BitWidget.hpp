#pragma once

#include <QWidget>
#include <bitset>
#include <QSize>
#include <QString>
#include <QEvent>
#include <QPaintEvent>
#include <QPainter>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/Label.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ClickableWidget.hpp"

#include "BitBodyWidget.hpp"

namespace Bloom::Widgets
{
    class BitWidget: public QWidget
    {
        Q_OBJECT

    private:
        static const int VERTICAL_SPACING = 3;

    public:
        static constexpr int LABEL_HEIGHT = 14;
        static constexpr int LABEL_COUNT = 2;
        static constexpr int WIDTH = BitBodyWidget::WIDTH;
        static constexpr int HEIGHT = BitBodyWidget::HEIGHT + (BitWidget::LABEL_HEIGHT * BitWidget::LABEL_COUNT)
            + BitWidget::VERTICAL_SPACING;
        static constexpr int SPACING = 6;

        BitWidget(
            int bitIndex,
            int bitNumber,
            std::bitset<std::numeric_limits<unsigned char>::digits>& bitset,
            bool readOnly,
            QWidget* parent
        );

    signals:
        void bitChanged();

    private:
        int bitIndex = 0;
        int bitNumber = 0;
        std::bitset<std::numeric_limits<unsigned char>::digits>& bitset;
        bool readOnly = true;

        BitBodyWidget* body = nullptr;

        Label* bitLabel = nullptr;
        Label* bitNumberLabel = nullptr;
    };
}
