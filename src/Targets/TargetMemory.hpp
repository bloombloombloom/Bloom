#pragma once

#include <cstdint>
#include <vector>
#include <span>

namespace Targets
{
    using TargetMemoryAddress = std::uint32_t;
    using TargetMemorySize = std::uint32_t;
    using TargetStackPointer = TargetMemoryAddress;
    using TargetMemoryBuffer = std::vector<unsigned char>;
    using TargetMemoryBufferSpan = std::span<const TargetMemoryBuffer::value_type>;

    using TargetAddressSpaceId = std::size_t;
    using TargetMemorySegmentId = std::size_t;

    enum class TargetMemoryEndianness: std::uint8_t
    {
        BIG,
        LITTLE,
    };

    struct TargetMemoryAccess
    {
        bool readable = false;
        bool writeable = false;

        TargetMemoryAccess(
            bool readable,
            bool writeable
        )
            : readable(readable)
            , writeable(writeable)
        {}
    };
}
