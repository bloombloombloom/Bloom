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

namespace Bloom::Widgets
{
    class BitBodyWidget: public ClickableWidget
    {
    Q_OBJECT
    private:
        int bitIndex = 0;
        std::bitset<std::numeric_limits<unsigned char>::digits>::reference bit;
        bool readOnly = true;
        bool hoverActive = false;

    protected:
        bool event(QEvent* event) override;
        void mouseReleaseEvent(QMouseEvent* event) override;
        void paintEvent(QPaintEvent* event) override;
        void drawWidget(QPainter& painter);

    public:
        constexpr static int WIDTH = 23;
        constexpr static int HEIGHT = 30;

        BitBodyWidget(
            int bitIndex,
            std::bitset<std::numeric_limits<unsigned char>::digits>::reference bit,
            bool readOnly,
            QWidget* parent
        );
    };
}
