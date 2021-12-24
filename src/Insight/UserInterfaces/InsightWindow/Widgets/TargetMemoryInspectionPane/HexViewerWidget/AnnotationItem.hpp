#pragma once

#include <QGraphicsItem>
#include <cstdint>

namespace Bloom::Widgets
{
    enum class AnnotationItemPosition: std::uint8_t
    {
        TOP,
        BOTTOM,
    };

    class AnnotationItem: public QGraphicsItem
    {
    public:
        static constexpr int TOP_HEIGHT = 26;
        static constexpr int BOTTOM_HEIGHT = 26;

        const int width;
        const int height;
        const std::uint32_t startAddress;
        const std::uint32_t endAddress;
        AnnotationItemPosition position = AnnotationItemPosition::TOP;

        AnnotationItem(
            std::uint32_t startAddress,
            std::size_t size,
            QString labelText,
            AnnotationItemPosition position
        );

        [[nodiscard]] QRectF boundingRect() const override {
            return QRectF(0, 0, this->width, this->height);
        }
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    protected:
        [[nodiscard]] QColor getLineColor() const {
            return QColor(0x4F, 0x4F, 0x4F);
        }

        [[nodiscard]] QColor getLabelFontColor() const {
            return QColor(0x8A, 0x8A, 0x8D);
        }

    private:
        std::size_t size = 0;
        QString labelText;
    };
}
