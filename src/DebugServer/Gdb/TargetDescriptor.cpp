#include "TargetDescriptor.hpp"

namespace Bloom::DebugServer::Gdb
{
    TargetDescriptor::TargetDescriptor(
        const Targets::TargetDescriptor& targetDescriptor,
        const BiMap<Targets::TargetMemoryType, std::uint32_t>& memoryOffsetsByType,
        std::map<GdbRegisterId, RegisterDescriptor> gdbRegisterDescriptorsById,
        std::map<Targets::TargetRegisterDescriptorId, GdbRegisterId> gdbRegisterIdsByTargetRegisterDescriptorId,
        std::map<GdbRegisterId, Targets::TargetRegisterDescriptorId> targetRegisterDescriptorIdsByGdbRegisterId
    )
        : targetDescriptor(targetDescriptor)
        , memoryOffsetsByType(memoryOffsetsByType)
        , memoryOffsets(memoryOffsetsByType.getValues())
        , gdbRegisterDescriptorsById(gdbRegisterDescriptorsById)
        , gdbRegisterIdsByTargetRegisterDescriptorId(gdbRegisterIdsByTargetRegisterDescriptorId)
        , targetRegisterDescriptorIdsByGdbRegisterId(targetRegisterDescriptorIdsByGdbRegisterId)
    {}

    std::uint32_t TargetDescriptor::getMemoryOffset(Targets::TargetMemoryType memoryType) const {
        return this->memoryOffsetsByType.valueAt(memoryType).value_or(0);
    }

    Targets::TargetMemoryType TargetDescriptor::getMemoryTypeFromGdbAddress(std::uint32_t address) const {
        // Start with the largest offset until we find a match
        for (
            auto memoryOffsetIt = this->memoryOffsets.rbegin();
            memoryOffsetIt != this->memoryOffsets.rend();
            ++memoryOffsetIt
            ) {
            if ((address & *memoryOffsetIt) == *memoryOffsetIt) {
                return this->memoryOffsetsByType.at(*memoryOffsetIt);
            }
        }

        return Targets::TargetMemoryType::FLASH;
    }

    std::optional<GdbRegisterId> TargetDescriptor::getGdbRegisterIdFromTargetRegisterDescriptorId(
        Targets::TargetRegisterDescriptorId targetRegisterDescriptorId
    ) const {
        const auto gdbRegisterIdIt = this->gdbRegisterIdsByTargetRegisterDescriptorId.find(
            targetRegisterDescriptorId
        );

        if (gdbRegisterIdIt != this->gdbRegisterIdsByTargetRegisterDescriptorId.end()) {
            return gdbRegisterIdIt->second;
        }

        return std::nullopt;
    }

    std::optional<Targets::TargetRegisterDescriptorId> TargetDescriptor::getTargetRegisterDescriptorIdFromGdbRegisterId(
        GdbRegisterId gdbRegisterId
    ) const {
        const auto registerDescriptorIdIt = this->targetRegisterDescriptorIdsByGdbRegisterId.find(gdbRegisterId);

        if (registerDescriptorIdIt != this->targetRegisterDescriptorIdsByGdbRegisterId.end()) {
            return registerDescriptorIdIt->second;
        }

        return std::nullopt;
    }
}
