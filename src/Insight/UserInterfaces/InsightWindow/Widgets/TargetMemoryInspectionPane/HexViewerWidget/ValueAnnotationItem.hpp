#pragma once

#include <QPainter>
#include <QWidget>
#include <QStyleOptionGraphicsItem>

#include "AnnotationItem.hpp"

#include "src/Targets/TargetMemory.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/FocusedMemoryRegion.hpp"

namespace Bloom::Widgets
{
    class ValueAnnotationItem: public AnnotationItem
    {
    public:
        explicit ValueAnnotationItem(const FocusedMemoryRegion& focusedMemoryRegion);
        void setValue(const Targets::TargetMemoryBuffer& value);
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    protected:
        [[nodiscard]] QColor getLabelFontColor() const override {
            return QColor(0x94, 0x6F, 0x30);
        }

        [[nodiscard]] int getLabelFontSize() const override {
            return 11;
        }

    private:
        static constexpr auto DEFAULT_LABEL_TEXT = "??";

        FocusedMemoryRegion focusedMemoryRegion;
        Targets::TargetMemoryBuffer value;
        Targets::TargetMemoryEndianness endianness = Targets::TargetMemoryEndianness::LITTLE;

        void refreshLabelText();
    };
}
