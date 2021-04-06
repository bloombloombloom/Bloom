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
                default: {
                    return TargetSignature(payloadData[0], payloadData[1], payloadData[2]);
                }
            }
        }
    };
}
