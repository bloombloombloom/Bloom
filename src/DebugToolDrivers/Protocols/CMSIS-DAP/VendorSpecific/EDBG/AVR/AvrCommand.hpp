#pragma once

#include <vector>
#include <cstdint>

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/Command.hpp"

#include "AvrResponse.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr
{
    /**
     * AVR CMSIS-DAP vendor command.
     */
    class AvrCommand: public Command
    {
    public:
        /*
         * AVR CMSIS-DAP vendor commands *do not* directly result in an AvrResponse object. The device will respond
         * immediately upon receiving this command, simply acknowledging receipt of the command.
         *
         * If a response is expected to follow upon the execution of the AVR command, it must be requested from the
         * device, using the AvrResponseCommand (see that class declaration for more info).
         *
         * For this reason, the ExpectedResponseType for this command is just the standard Response type.
         *
         * For more on the purpose of this alias, see the Command class.
         */
        using ExpectedResponseType = Response;

        AvrCommand(
            std::size_t fragmentCount,
            std::size_t fragmentNumber,
            std::vector<unsigned char> commandPacket
        )
            : fragmentCount(fragmentCount)
            , fragmentNumber(fragmentNumber)
            , commandPacket(std::move(commandPacket))
        {
            this->setCommandId(0x80);
        }

        /**
         * Constructs raw command data on the fly.
         *
         * @return
         */
        [[nodiscard]] std::vector<unsigned char> getData() const override;

        [[nodiscard]] std::size_t getFragmentNumber() const {
            return this->fragmentNumber;
        }

        [[nodiscard]] std::size_t getFragmentCount() const {
            return this->fragmentCount;
        }

        [[nodiscard]] const std::vector<unsigned char>& getCommandPacket() const {
            return this->commandPacket;
        }

    private:
        std::size_t fragmentNumber = 1;
        std::size_t fragmentCount = 1;

        std::vector<unsigned char> commandPacket;
    };
}
