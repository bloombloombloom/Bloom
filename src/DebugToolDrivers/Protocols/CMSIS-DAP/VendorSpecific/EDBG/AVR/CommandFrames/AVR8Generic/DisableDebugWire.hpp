#pragma once

#include "Avr8GenericCommandFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class DisableDebugWire: public Avr8GenericCommandFrame
    {
    private:
        void init() {
            /*
             * The disable debugWire command consists of 2 bytes:
             * 1. Command ID (0x17)
             * 2. Version (0x00)
             */
            auto payload = std::vector<unsigned char>(2);
            payload[0] = 0x17;
            payload[1] = 0x00;
            this->setPayload(payload);
        }

    public:
        DisableDebugWire() {
            init();
        };
    };
}
