#pragma once

#include <cstdint>
#include <sstream>
#include <iomanip>

namespace Targets::Microchip::Avr
{
    /**
     * All AVR targets carry a three-byte signature that is *usually* unique to the target.
     *
     * The AVR target signature consists of three bytes: 0xAABBCC
     *  Byte AA (byteZero) identifies the manufacture of the target (usually 1E for Atmel/Microchip)
     *  Byte BB (byteOne) sometimes indicates the flash/ram capacity of the target
     *  Byte CC (byteTwo) identifies the target
     *
     * Some AVR targets have been found to carry identical signatures. For example, the AT90PWM1, AT90PWM2B
     * and the AT90PWM3B all carry a signature of 0x1E9383. Although these devices may not differ in
     * significant ways, Bloom does still take duplicate signatures into account, to ensure that the correct
     * target description file is used.
     *
     * This class represents an AVR target signature.
     */
    struct TargetSignature
    {
        unsigned char byteZero = 0x00;
        unsigned char byteOne = 0x00;
        unsigned char byteTwo = 0x00;

        TargetSignature() = default;
        TargetSignature(unsigned char byteZero, unsigned char byteOne, unsigned char byteTwo)
            : byteZero(byteZero)
            , byteOne(byteOne)
            , byteTwo(byteTwo)
        {};

        explicit TargetSignature(const std::string& hex) {
            const auto signature = static_cast<std::uint32_t>(std::stoul(hex, nullptr, 16));

            this->byteZero = static_cast<unsigned char>(signature >> 16);
            this->byteOne = static_cast<unsigned char>(signature >> 8);
            this->byteTwo = static_cast<unsigned char>(signature);
        }

        [[nodiscard]] std::string toHex() const {
            std::stringstream stream;
            stream << std::hex << std::setfill('0');
            stream << std::setw(2) << static_cast<unsigned int>(this->byteZero);
            stream << std::setw(2) << static_cast<unsigned int>(this->byteOne);
            stream << std::setw(2) << static_cast<unsigned int>(this->byteTwo);

            return "0x" + stream.str();
        }

        bool operator == (const TargetSignature& signature) const {
            return
                signature.byteZero == this->byteZero
                && signature.byteOne == this->byteOne
                && signature.byteTwo == this->byteTwo;
        }

        bool operator != (const TargetSignature& signature) const {
            return !(*this == signature);
        }
    };
}
