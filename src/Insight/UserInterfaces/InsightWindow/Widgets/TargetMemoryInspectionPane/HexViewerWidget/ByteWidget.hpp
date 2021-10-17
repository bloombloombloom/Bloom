#pragma once

#include <cstdint>
#include <QEvent>
#include <optional>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ClickableWidget.hpp"

namespace Bloom::Widgets
{
    class ByteWidget: public ClickableWidget
    {
        Q_OBJECT

    public:
        static constexpr int WIDTH = 25;
        static constexpr int HEIGHT = 20;

        static constexpr int RIGHT_MARGIN = 5;
        static constexpr int BOTTOM_MARGIN = 5;

        std::size_t byteIndex;
        unsigned char value = 0x00;
        std::uint32_t address = 0x00;
        QString addressHex;
        QString relativeAddressHex;
        bool valueInitialised = false;

        std::size_t currentRowIndex = 0;
        std::size_t currentColumnIndex = 0;

        ByteWidget(
            std::size_t byteNumber,
            std::uint32_t address,
            std::optional<ByteWidget*>& hoveredByteWidget,
            QWidget* parent
        );
        void setValue(unsigned char value);

    public slots:
        void setSelected(bool selected);

    signals:
        void selected(ByteWidget*);
        void enter(ByteWidget*);
        void leave(ByteWidget*);

    protected:
        virtual void postSetSelected(bool selected) {};
        bool event(QEvent* event) override;
        void paintEvent(QPaintEvent* event) override;
        void drawWidget(QPainter& painter);

    private:
        bool hoverActive = false;
        bool valueChanged = false;

        QString hexValue;
        std::optional<QString> asciiValue;

        std::optional<ByteWidget*>& hoveredByteWidget;
    };
}
