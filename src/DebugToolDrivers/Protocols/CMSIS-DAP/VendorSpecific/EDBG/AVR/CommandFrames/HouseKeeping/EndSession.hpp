#pragma once

#include "HouseKeepingCommandFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::HouseKeeping
{
    /**
     * The End Session command ends the active session with the tool.
     */
    class EndSession: public HouseKeepingCommandFrame
    {
    private:
        void init() {
            /*
             * The payload for the End Session command consists of three bytes. A command ID byte (0x11), a
             * version byte (0x00) and a reset flag (0x00 for no reset, 0x01 for tool reset).
             */
            auto payload = std::vector<unsigned char>(3);
            payload[0] = 0x11;
            payload[1] = 0x00;
            payload[2] = 0x00;
            this->setPayload(payload);
        }

    public:
        EndSession() : HouseKeepingCommandFrame() {
            this->init();
        }
    };

}
