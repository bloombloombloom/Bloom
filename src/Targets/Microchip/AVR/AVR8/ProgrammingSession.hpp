#pragma once

#include <cstdint>

namespace Targets::Microchip::Avr::Avr8Bit
{
    /**
     * Information relating to a specific AVR8 programming session.
     *
     * Whenever an AVR8 target is put into programming mode, a new programming session is started.
     */
    struct ProgrammingSession
    {
        /**
         * This flag indicates whether we need to manage the EESAVE fuse for this programming session. It will only be
         * set to true if the user has chosen to preserve EEPROM and the EESAVE fuse wasn't already programmed when the
         * programming session started.
         */
        bool managingEesaveFuseBit = false;

        ProgrammingSession() = default;
    };
}
