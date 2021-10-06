#pragma once

#include "Avr8GenericCommandFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class DeactivatePhysical: public Avr8GenericCommandFrame
    {
    public:
        DeactivatePhysical() {
            init();
        };

    private:
        void init() {
            /*
             * The deactivate physical command consists of 2 bytes:
             * 1. Command ID (0x11)
             * 2. Version (0x00)
             */
            auto payload = std::vector<unsigned char>(2);
            payload[0] = 0x11;
            payload[1] = 0x00;
            this->setPayload(payload);
        }
    };
}
