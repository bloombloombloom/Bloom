#pragma once

#include <QWidget>
#include <bitset>
#include <QLabel>
#include <QSize>
#include <QString>
#include <QEvent>

#include "BitWidget.hpp"

namespace Bloom::Widgets
{
    class BitsetWidget: public QWidget
    {
        Q_OBJECT

    public:
        constexpr static int VALUE_GRAPHIC_HEIGHT = 20;
        constexpr static int HEIGHT = BitWidget::HEIGHT + BitsetWidget::VALUE_GRAPHIC_HEIGHT + 8;
        constexpr static int WIDTH = (BitWidget::WIDTH + BitWidget::SPACING) * 8;

        BitsetWidget(int byteNumber, unsigned char& byte, bool readOnly, QWidget* parent);

        void updateValue();

    signals:
        void byteChanged();

    protected:
        void paintEvent(QPaintEvent* event) override;
        void drawWidget(QPainter& painter);

    private:
        int byteNumber = 0;
        unsigned char& byte;
        std::bitset<std::numeric_limits<unsigned char>::digits> bitset = {byte};
        bool readOnly = true;

        QWidget* container = nullptr;
    };
}
