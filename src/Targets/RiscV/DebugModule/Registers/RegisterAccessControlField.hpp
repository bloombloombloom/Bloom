#pragma once

#include <cstdint>

#include "src/Targets/RiscV/RiscVGeneric.hpp"

namespace Targets::RiscV::DebugModule::Registers
{
    struct RegisterAccessControlField
    {
        enum class RegisterSize: std::uint8_t
        {
            SIZE_32 = 0x02,
            SIZE_64 = 0x03,
            SIZE_128 = 0x04,
        };

        ::Targets::RiscV::Registers::RegisterNumber registerNumber;
        bool write:1 = false;
        bool transfer:1 = false;
        bool postExecute:1 = false;
        bool postIncrement:1 = false;
        RegisterSize size:3 = RegisterSize::SIZE_32;

        RegisterAccessControlField() = default;

        RegisterAccessControlField(
            ::Targets::RiscV::Registers::RegisterNumber registerNumber,
            bool write,
            bool transfer,
            bool postExecute,
            bool postIncrement,
            RegisterSize size
        )
            : registerNumber(registerNumber)
            , write(write)
            , transfer(transfer)
            , postExecute(postExecute)
            , postIncrement(postIncrement)
            , size(size)
        {}

        constexpr explicit RegisterAccessControlField(std::uint32_t controlValue)
            : registerNumber(static_cast<::Targets::RiscV::Registers::RegisterNumber>(controlValue & 0xFFFF))
            , write(static_cast<bool>(controlValue & (0x01 << 16)))
            , transfer(static_cast<bool>(controlValue & (0x01 << 17)))
            , postExecute(static_cast<bool>(controlValue & (0x01 << 18)))
            , postIncrement(static_cast<bool>(controlValue & (0x01 << 19)))
            , size(static_cast<RegisterSize>((controlValue >> 20) & 0x07))
        {}

        constexpr std::uint32_t value() const {
            return std::uint32_t{0}
                | static_cast<std::uint32_t>(this->registerNumber)
                | static_cast<std::uint32_t>(this->write) << 16
                | static_cast<std::uint32_t>(this->transfer) << 17
                | static_cast<std::uint32_t>(this->postExecute) << 18
                | static_cast<std::uint32_t>(this->postIncrement) << 19
                | static_cast<std::uint32_t>(this->size) << 20
            ;
        }
    };
}
