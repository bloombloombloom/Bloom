#pragma once

#include <QGraphicsItem>
#include <cstdint>

#include "src/Targets/TargetMemory.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/FocusedMemoryRegion.hpp"

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
        static constexpr int VERTICAL_LINE_LENGTH = 5;

        const int width;
        const int height;
        const Targets::TargetMemoryAddress startAddress;
        const Targets::TargetMemoryAddress endAddress;
        const Targets::TargetMemorySize size;
        AnnotationItemPosition position = AnnotationItemPosition::TOP;

        AnnotationItem(
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemorySize size,
            QString labelText,
            AnnotationItemPosition position
        );

        AnnotationItem(
            const Targets::TargetMemoryAddressRange&
            addressRange,
            const QString& labelText,
            AnnotationItemPosition position
        );

        AnnotationItem(const FocusedMemoryRegion& focusedMemoryRegion, AnnotationItemPosition position);

        [[nodiscard]] QRectF boundingRect() const override {
            return QRectF(0, 0, this->width, this->height);
        }
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    protected:
        QString labelText;

        [[nodiscard]] virtual QColor getLineColor() const {
            return QColor(0x4F, 0x4F, 0x4F);
        }

        [[nodiscard]] virtual QColor getLabelFontColor() const {
            return QColor(0x68, 0x68, 0x68);
        }

        [[nodiscard]] virtual int getLabelFontSize() const {
            return 12;
        }
    };
}
