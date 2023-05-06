#include "FocusedRegionGroupItem.hpp"

#include <cassert>

namespace Bloom::Widgets
{
    FocusedRegionGroupItem::FocusedRegionGroupItem(
        const FocusedMemoryRegion& focusedRegion,
        std::unordered_map<Targets::TargetMemoryAddress, ByteItem>& byteItemsByAddress,
        HexViewerItem* parent
    )
        : GroupItem(focusedRegion.addressRange.startAddress, parent)
        , focusedMemoryRegion(focusedRegion)
    {
        const auto& startAddress = this->focusedMemoryRegion.addressRange.startAddress;
        const auto& endAddress = this->focusedMemoryRegion.addressRange.endAddress;

        // Sanity check
        assert(byteItemsByAddress.contains(startAddress) && byteItemsByAddress.contains(endAddress));

        for (auto address = startAddress; address <= endAddress; ++address) {
            auto& byteItem = byteItemsByAddress.at(address);
            byteItem.parent = this;
            byteItem.grouped = true;

            this->items.push_back(&byteItem);
        }
    }

    FocusedRegionGroupItem::~FocusedRegionGroupItem() {
        const auto updateChildItems = [] (const decltype(this->items)& items, const auto& updateChildItems) -> void {
            for (auto& item : items) {
                auto* byteItem = dynamic_cast<ByteItem*>(item);

                if (byteItem != nullptr) {
                    byteItem->grouped = false;
                    continue;
                }

                auto* groupItem = dynamic_cast<GroupItem*>(item);

                if (groupItem != nullptr) {
                    updateChildItems(groupItem->items, updateChildItems);
                }
            }
        };

        updateChildItems(this->items, updateChildItems);
    }

    void FocusedRegionGroupItem::refreshValue(const HexViewerSharedState& hexViewerState) {
        if (!hexViewerState.data.has_value()) {
            this->valueLabel = std::nullopt;
            return;
        }

        const auto regionStartAddress = this->focusedMemoryRegion.addressRange.startAddress;
        const auto regionEndAddress = this->focusedMemoryRegion.addressRange.endAddress;
        const auto startIndex = regionStartAddress - hexViewerState.memoryDescriptor.addressRange.startAddress;
        auto value = Targets::TargetMemoryBuffer(
            hexViewerState.data->begin() + startIndex,
            hexViewerState.data->begin() + startIndex + (regionEndAddress - regionStartAddress + 1)
        );

        if (this->focusedMemoryRegion.endianness == Targets::TargetMemoryEndianness::LITTLE) {
            std::reverse(value.begin(), value.end());
        }

        switch (this->focusedMemoryRegion.dataType) {
            case MemoryRegionDataType::UNSIGNED_INTEGER: {
                std::uint64_t integerValue = 0;
                for (const auto& byte : value) {
                    integerValue = (integerValue << 8) | byte;
                }

                this->valueLabel = QString::number(integerValue);
                break;
            }
            case MemoryRegionDataType::SIGNED_INTEGER: {
                const auto valueSize = value.size();

                if (valueSize == 1) {
                    this->valueLabel = QString::number(static_cast<int8_t>(value[0]));
                    break;
                }

                if (valueSize == 2) {
                    this->valueLabel = QString::number(static_cast<int16_t>((value[0] << 8) | value[1]));
                    break;
                }

                if (valueSize <= 4) {
                    std::int32_t integerValue = 0;
                    for (const auto& byte : value) {
                        integerValue = (integerValue << 8) | byte;
                    }

                    this->valueLabel = QString::number(integerValue);
                    break;
                }

                std::int64_t integerValue = 0;
                for (const auto& byte : value) {
                    integerValue = (integerValue << 8) | byte;
                }

                this->valueLabel = QString::number(integerValue);
                break;
            }
            case MemoryRegionDataType::ASCII_STRING: {
                // Replace non-ASCII chars with '?'
                auto asciiData = value;

                std::replace_if(
                    asciiData.begin(),
                    asciiData.end(),
                    [] (unsigned char value) {
                        /*
                         * We only care about non-control characters (except for the white space character) in
                         * the standard ASCII range.
                         */
                        return value < 32 || value > 126;
                    },
                    '?'
                );

                this->valueLabel = "'" + QString::fromLatin1(
                    reinterpret_cast<const char*>(asciiData.data()),
                    static_cast<qsizetype>(asciiData.size())
                ) + "'";
                break;
            }
            default: {
                this->valueLabel = std::nullopt;
            }
        }
    }

    QMargins FocusedRegionGroupItem::groupMargins(
        const HexViewerSharedState* hexViewerState,
        const int maximumWidth
    ) const {
        if (hexViewerState->settings.displayAnnotations) {
            constexpr auto averageSymbolWidth = 8;
            const auto nameLabelWidth = static_cast<int>(this->focusedMemoryRegion.name.size() * averageSymbolWidth);
            const auto valueLabelWidth = static_cast<int>(
                this->valueLabel.has_value() ? this->valueLabel->size() * averageSymbolWidth : 0
            );

            const auto minimumWidth = std::min(std::max(nameLabelWidth, valueLabelWidth), maximumWidth);

            const auto byteItemSize = (this->focusedMemoryRegion.addressRange.endAddress
                - this->focusedMemoryRegion.addressRange.startAddress + 1);
            const auto estimatedWidth = static_cast<int>(
                byteItemSize * (ByteItem::WIDTH + ByteItem::RIGHT_MARGIN) - ByteItem::RIGHT_MARGIN
            );

            const auto annotationMargin = std::min(
                static_cast<int>(
                    estimatedWidth < minimumWidth ? minimumWidth - estimatedWidth : 0
                ),
                std::max(maximumWidth - estimatedWidth, 0)
            );

            return QMargins(
                annotationMargin / 2,
                this->focusedMemoryRegion.dataType != MemoryRegionDataType::UNKNOWN
                    ? FocusedRegionGroupItem::ANNOTATION_HEIGHT
                    : 0,
                annotationMargin / 2,
                FocusedRegionGroupItem::ANNOTATION_HEIGHT
            );
        }

        return QMargins(0, 0, 0, 0);
    }
}
