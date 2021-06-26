#pragma once

#include "Avr8GenericResponseFrame.hpp"
#include "src/Targets/Microchip/AVR/AVR8/PhysicalInterface.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::ResponseFrames::Avr8Generic
{
    class GetDeviceId: public Avr8GenericResponseFrame
    {
    public:
        GetDeviceId() = default;
        explicit GetDeviceId(const std::vector<AvrResponse>& AvrResponses): Avr8GenericResponseFrame(AvrResponses) {}

        Targets::Microchip::Avr::TargetSignature extractSignature(
            Targets::Microchip::Avr::Avr8Bit::PhysicalInterface physicalInterface
        ) {
            using Targets::Microchip::Avr::Avr8Bit::PhysicalInterface;
            auto payloadData = this->getPayloadData();

            switch (physicalInterface) {
                case PhysicalInterface::DEBUG_WIRE: {
                    /*
                     * When using the DebugWire physical interface, the get device ID command will return
                     * four bytes, where the first can be ignored.
                     */
                    return Targets::Microchip::Avr::TargetSignature(payloadData[1], payloadData[2], payloadData[3]);
                }
                case PhysicalInterface::PDI:
                case PhysicalInterface::UPDI: {
                    /*
                     * When using the PDI physical interface, the signature is returned in LSB format.
                     */
                    return Targets::Microchip::Avr::TargetSignature(payloadData[3], payloadData[2], payloadData[1]);
                }
                case PhysicalInterface::JTAG: {
                    /*
                     * When using the JTAG interface, the get device ID command returns a 32 bit JTAG ID. This takes
                     * the following form:
                     *
                     *  VVVV PPPPPPPPPPPPPPPP MMMMMMMMMMM L
                     *
                     *  - (V) - Version nibble (4 bits)
                     *  - (P) - Part number (this is typically the AVR signature excluding the manufacture
                     *          byte (0x1E)) (16 bits)
                     *  - (M) - Manufacture identity (11 bits)
                     *  - (L) - LSB indicator (1 bit)
                     *
                     * We convert this into a Avr::TargetSignature by extracting the relevant part number and assuming
                     * a manufacture byte of 0x1E.
                     *
                     * TODO: We're not considering the value of the LSB indicator bit. We're just assuming it will
                     *       always be MSB. Is this assumption correct?
                     */
                    auto jtagId = static_cast<std::uint32_t>(
                        (payloadData[0] << 24) | (payloadData[1] << 16) | (payloadData[2] << 8) | (payloadData[3])
                    );

                    return Targets::Microchip::Avr::TargetSignature(
                        0x1E,
                        static_cast<unsigned char>((jtagId << 4) >> 24),
                        static_cast<unsigned char>((jtagId << 12) >> 24)
                    );
                }
                default: {
                    return Targets::Microchip::Avr::TargetSignature(
                        payloadData[0],
                        payloadData[1],
                        payloadData[2]
                    );
                }
            }
        }
    };
}
