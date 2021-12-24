#include "ValueAnnotationItem.hpp"

#include <algorithm>

using namespace Bloom::Widgets;

ValueAnnotationItem::ValueAnnotationItem(const FocusedMemoryRegion& focusedMemoryRegion)
: AnnotationItem(focusedMemoryRegion, AnnotationItemPosition::TOP), focusedMemoryRegion(focusedMemoryRegion) {
    this->labelText = QString(ValueAnnotationItem::DEFAULT_LABEL_TEXT);
}

void ValueAnnotationItem::setValue(const Targets::TargetMemoryBuffer& value) {
    this->value = value;
    this->refreshLabelText();
    this->setToolTip(this->labelText);
}

void ValueAnnotationItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    auto font = painter->font();
    font.setItalic(true);
    painter->setFont(font);

    AnnotationItem::paint(painter, option, widget);
}

void ValueAnnotationItem::refreshLabelText() {
    this->update();

    const auto& data = this->value;

    if (data.empty()) {
        this->labelText = QString(ValueAnnotationItem::DEFAULT_LABEL_TEXT);
        return;
    }

    switch (this->focusedMemoryRegion.dataType) {
        case MemoryRegionDataType::INTEGER: {
            std::uint64_t integerValue = 0;
            for (const auto& byte : data) {
                integerValue = (integerValue << 8) | byte;
            }

            this->labelText = QString::number(integerValue);
            break;
        }
        case MemoryRegionDataType::ASCII_STRING: {
            // Replace non-ASCII chars with '?'
            auto asciiData = data;

            std::replace_if(
                asciiData.begin(),
                asciiData.end(),
                [] (unsigned char value) {
                    /*
                     * We only care about non-control characters (with the exception of the white space character) in
                     * the standard ASCII range.
                     */
                    constexpr auto asciiRangeStart = 32;
                    constexpr auto asciiRangeEnd = 126;
                    return value < asciiRangeStart || value > asciiRangeEnd;
                },
                '?'
            );

            this->labelText = "'" + QString::fromLatin1(
                reinterpret_cast<const char*>(asciiData.data()),
                static_cast<qsizetype>(asciiData.size())
            ) + "'";
            break;
        }
        default: {
            this->labelText = QString(ValueAnnotationItem::DEFAULT_LABEL_TEXT);
        }
    }
}
