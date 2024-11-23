#pragma once

#include <cstdint>

namespace DebugToolDrivers::Protocols::RiscVDebugSpec::DebugModule
{
    using RegisterAddress = std::uint8_t;
    using RegisterValue = std::uint32_t;
    using HartIndex = std::uint32_t;

    enum class DmiOperation: std::uint8_t
    {
        IGNORE = 0x00,
        READ = 0x01,
        WRITE = 0x02,
    };

    enum class DmiOperationStatus: std::uint8_t
    {
        SUCCESS = 0x00,
        FAILED = 0x02,
        BUSY = 0x03,
    };

    enum class AbstractCommandError: std::uint8_t
    {
        NONE = 0x00,
        BUSY = 0x01,
        NOT_SUPPORTED = 0x02,
        EXCEPTION = 0x03,
        HALT_RESUME = 0x04,
        BUS = 0x05,
        OTHER = 0x07,
        CLEAR = 0x07,
    };

    enum class MemoryAccessStrategy: std::uint8_t
    {
        ABSTRACT_COMMAND,
        PROGRAM_BUFFER,
    };
}
