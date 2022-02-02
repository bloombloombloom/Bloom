#include "ValueAnnotationItem.hpp"

#include <algorithm>

using namespace Bloom::Widgets;

ValueAnnotationItem::ValueAnnotationItem(const FocusedMemoryRegion& focusedMemoryRegion)
    : AnnotationItem(focusedMemoryRegion, AnnotationItemPosition::TOP)
    , focusedMemoryRegion(focusedMemoryRegion)
    , endianness(focusedMemoryRegion.endianness)
{
    this->labelText = QString(ValueAnnotationItem::DEFAULT_LABEL_TEXT);
}

void ValueAnnotationItem::setValue(const Targets::TargetMemoryBuffer& value) {
    this->value = value;

    if (this->endianness == Targets::TargetMemoryEndianness::LITTLE) {
        std::reverse(this->value.begin(), this->value.end());
    }

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

    if (this->value.empty()) {
        this->labelText = QString(ValueAnnotationItem::DEFAULT_LABEL_TEXT);
        return;
    }

    switch (this->focusedMemoryRegion.dataType) {
        case MemoryRegionDataType::UNSIGNED_INTEGER: {
            std::uint64_t integerValue = 0;
            for (const auto& byte : this->value) {
                integerValue = (integerValue << 8) | byte;
            }

            this->labelText = QString::number(integerValue);
            break;
        }
        case MemoryRegionDataType::SIGNED_INTEGER: {
            const auto valueSize = this->value.size();

            if (valueSize == 1) {
                this->labelText = QString::number(static_cast<int8_t>(this->value[0]));
                break;
            }

            if (valueSize == 2) {
                this->labelText = QString::number(static_cast<int16_t>((this->value[0] << 8) | this->value[1]));
                break;
            }

            if (valueSize <= 4) {
                std::int32_t integerValue = 0;
                for (const auto& byte : this->value) {
                    integerValue = (integerValue << 8) | byte;
                }

                this->labelText = QString::number(integerValue);
                break;
            }

            if (valueSize <= 8) {
                std::int64_t integerValue = 0;
                for (const auto& byte : this->value) {
                    integerValue = (integerValue << 8) | byte;
                }

                this->labelText = QString::number(integerValue);
                break;
            }
        }
        case MemoryRegionDataType::ASCII_STRING: {
            // Replace non-ASCII chars with '?'
            auto asciiData = this->value;

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
