#pragma once

#include <vector>
#include <cstdint>

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/Command.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr
{
    class AvrCommand: public Command
    {
    public:
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
