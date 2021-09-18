#pragma once

#include <QWidget>
#include <bitset>
#include <QLabel>
#include <QSize>
#include <QString>
#include <QEvent>
#include <QPaintEvent>
#include <QPainter>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ClickableWidget.hpp"

#include "BitBodyWidget.hpp"

namespace Bloom::Widgets
{
    class BitWidget: public QWidget
    {
    Q_OBJECT
    private:
        const static int VERTICAL_SPACING = 3;

        int bitIndex = 0;
        int bitNumber = 0;
        std::bitset<std::numeric_limits<unsigned char>::digits>& bitset;
        bool readOnly = true;

        BitBodyWidget* body = nullptr;

        QLabel* bitLabel = nullptr;
        QLabel* bitNumberLabel = nullptr;

    public:
        constexpr static int LABEL_HEIGHT = 14;
        constexpr static int LABEL_COUNT = 2;
        constexpr static int WIDTH = BitBodyWidget::WIDTH;
        constexpr static int HEIGHT = BitBodyWidget::HEIGHT + (BitWidget::LABEL_HEIGHT * BitWidget::LABEL_COUNT)
            + BitWidget::VERTICAL_SPACING;
        constexpr static int SPACING = 6;

        BitWidget(
            int bitIndex,
            int bitNumber,
            std::bitset<std::numeric_limits<unsigned char>::digits>& bitset,
            bool readOnly,
            QWidget* parent
        );

    signals:
        void bitChanged();
    };
}
