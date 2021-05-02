#pragma once

#include "Avr8GenericResponseFrame.hpp"
#include "src/Targets/Microchip/AVR/Target.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::ResponseFrames::Avr8Generic
{
    using namespace Targets::Microchip::Avr;

    class GetDeviceId: public Avr8GenericResponseFrame
    {
    public:
        GetDeviceId(const std::vector<AvrResponse>& AvrResponses): Avr8GenericResponseFrame(AvrResponses) {}
        GetDeviceId() {}

        TargetSignature extractSignature(Avr8PhysicalInterface physicalInterface) {
            auto payloadData = this->getPayloadData();

            switch (physicalInterface) {
                case Avr8PhysicalInterface::DEBUG_WIRE: {
                    /*
                     * When using the DebugWire physical interface, the get device ID command will return
                     * four bytes, where the first can be ignored.
                     */
                    return TargetSignature(payloadData[1], payloadData[2], payloadData[3]);
                }
                case Avr8PhysicalInterface::PDI:
                case Avr8PhysicalInterface::PDI_1W: {
                    /*
                     * When using the PDI physical interface, the signature is returned in LSB format.
                     */
                    return TargetSignature(payloadData[3], payloadData[2], payloadData[1]);
                }
                case Avr8PhysicalInterface::JTAG: {
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

                    return TargetSignature(
                        0x1E,
                        static_cast<unsigned char>((jtagId << 4) >> 24),
                        static_cast<unsigned char>((jtagId << 12) >> 24)
                    );
                }
                default: {
                    return TargetSignature(payloadData[0], payloadData[1], payloadData[2]);
                }
            }
        }
    };
}
