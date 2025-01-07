#pragma once

#include <vector>

#include "src/DebugToolDrivers/Protocols/CmsisDap/Command.hpp"

#include "AvrResponse.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr
{
    /**
     * All AVR commands result in an automatic response, but that is just a response to confirm
     * receipt of the AVR Command. It is *not* a response to the AVR command.
     *
     * Responses to AVR commands are not automatically sent from the device, they must be requested from the device.
     *
     * An AvrResponseCommand is a CMSIS-DAP vendor command, used to request a response for an AVR command. In response
     * to an AvrResponseCommand, the device will send over an AvrResponse, which will contain the response to the
     * AVR command.
     *
     * For more information on this, see the 'Embedded Debugger-Based Tools Protocols User's Guide'
     * document, by Microchip. Link provided in the AtmelICE device class declaration.
     *
     * An AvrResponseCommand is very simple - it consists of a command ID and nothing more.
     */
    class AvrResponseCommand: public ::DebugToolDrivers::Protocols::CmsisDap::Command
    {
    public:
        using ExpectedResponseType = AvrResponse;

        AvrResponseCommand()
            : Command(0x81)
        {}
    };
}
