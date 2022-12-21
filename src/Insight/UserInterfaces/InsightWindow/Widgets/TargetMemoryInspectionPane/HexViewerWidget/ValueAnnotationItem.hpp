#pragma once

#include <QPainter>
#include <QWidget>
#include <QStyleOptionGraphicsItem>
#include <optional>

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

        [[nodiscard]] const QFont& getLabelFont() const override {
            static auto labelFont = std::optional<QFont>();

            if (!labelFont.has_value()) {
                labelFont = QFont("'Ubuntu', sans-serif");
                labelFont->setPixelSize(11);
                labelFont->setItalic(true);
            }

            return labelFont.value();
        }

    private:
        static constexpr auto DEFAULT_LABEL_TEXT = "??";

        FocusedMemoryRegion focusedMemoryRegion;
        Targets::TargetMemoryBuffer value;
        Targets::TargetMemoryEndianness endianness = Targets::TargetMemoryEndianness::LITTLE;

        void refreshLabelText();
    };
}
