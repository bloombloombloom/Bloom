#include "ConstructHexViewerByteItems.hpp"

#include <QMetaType>

namespace Bloom
{
    ConstructHexViewerByteItems::ConstructHexViewerByteItems(
        const Targets::TargetMemoryDescriptor& memoryDescriptor,
        std::optional<Targets::TargetStackPointer>& currentStackPointer,
        Widgets::ByteItem** hoveredByteItem,
        std::set<Widgets::ByteItem*>& highlightedByteItems,
        Widgets::HexViewerWidgetSettings& settings
    )
        : memoryDescriptor(memoryDescriptor)
        , currentStackPointer(currentStackPointer)
        , hoveredByteItem(hoveredByteItem)
        , highlightedByteItems(highlightedByteItems)
        , settings(settings)
    {
        qRegisterMetaType<std::map<Targets::TargetMemoryAddress, Widgets::ByteItem*>>();
    }

    void ConstructHexViewerByteItems::run(Services::TargetControllerService&) {
        const auto memorySize = this->memoryDescriptor.size();
        const auto startAddress = this->memoryDescriptor.addressRange.startAddress;

        for (Targets::TargetMemorySize i = 0; i < memorySize; i++) {
            const auto address = startAddress + i;

            this->byteItemsByAddress.emplace(
                address,
                new Widgets::ByteItem(
                    i,
                    address,
                    this->currentStackPointer,
                    this->hoveredByteItem,
                    this->highlightedByteItems,
                    settings
                )
            );
        }

        emit this->byteItems(std::move(this->byteItemsByAddress));
    }
}
