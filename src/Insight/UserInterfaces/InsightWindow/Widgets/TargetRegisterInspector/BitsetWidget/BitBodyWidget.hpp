#pragma once

#include <QWidget>
#include <bitset>
#include <QSize>
#include <QString>
#include <QEvent>
#include <QPaintEvent>
#include <QPainter>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ClickableWidget.hpp"

namespace Widgets
{
    class BitBodyWidget: public ClickableWidget
    {
        Q_OBJECT

    public:
        static constexpr int WIDTH = 19;
        static constexpr int HEIGHT = 29;

        BitBodyWidget(
            int bitIndex,
            std::bitset<std::numeric_limits<unsigned char>::digits>::reference bit,
            bool readOnly,
            QWidget* parent
        );

    protected:
        bool event(QEvent* event) override;
        void mouseReleaseEvent(QMouseEvent* event) override;
        void paintEvent(QPaintEvent* event) override;
        void drawWidget(QPainter& painter);

    private:
        int bitIndex = 0;
        std::bitset<std::numeric_limits<unsigned char>::digits>::reference bit;
        bool readOnly = true;
        bool hoverActive = false;
    };
}
