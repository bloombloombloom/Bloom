#pragma once

#include "HouseKeepingCommandFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::HouseKeeping
{
    /**
     * The Start Session command begins a session with the tool.
     */
    class StartSession: public HouseKeepingCommandFrame
    {
    private:
        void init() {
            /*
             * The payload for the Start Session command consists of two bytes. A command ID byte (0x10) and a
             * version byte (0x00).
             */
            auto payload = std::vector<unsigned char>(2);
            payload[0] = 0x10;
            payload[1] = 0x00;
            this->setPayload(payload);
        }

    public:
        StartSession() : HouseKeepingCommandFrame() {
            this->init();
        }
    };

}
