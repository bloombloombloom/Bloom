#pragma once

#include "Avr8GenericCommandFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class Run: public Avr8GenericCommandFrame
    {
    private:
        void init() {
            /*
             * The run command consists of 2 bytes:
             * 1. Command ID (0x32)
             * 2. Version (0x00)
             */
            auto payload = std::vector<unsigned char>(2);
            payload[0] = 0x32;
            payload[1] = 0x00;
            this->setPayload(payload);
        }

    public:
        Run() {
            init();
        };
    };

}
