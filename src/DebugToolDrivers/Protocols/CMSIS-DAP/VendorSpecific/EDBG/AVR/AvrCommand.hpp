#pragma once

#include <vector>
#include <cstdint>

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/Command.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr
{
    class AvrCommand: public Command
    {
    public:
        AvrCommand() {
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

        void setFragmentNumber(std::size_t fragmentNumber) {
            this->fragmentNumber = fragmentNumber;
        }

        [[nodiscard]] std::size_t getFragmentCount() const {
            return this->fragmentCount;
        }

        void setFragmentCount(std::size_t fragmentCount) {
            this->fragmentCount = fragmentCount;
        }

        [[nodiscard]] const std::vector<unsigned char>& getCommandPacket() const {
            return this->commandPacket;
        }

        void setCommandPacket(const std::vector<unsigned char>& commandPacket) {
            this->commandPacket = commandPacket;
        }

    private:
        std::size_t fragmentNumber = 1;
        std::size_t fragmentCount = 1;

        std::vector<unsigned char> commandPacket;
    };
}
