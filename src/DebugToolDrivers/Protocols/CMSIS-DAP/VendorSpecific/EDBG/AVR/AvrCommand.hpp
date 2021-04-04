#pragma once

#include <vector>

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/Command.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr
{
    class AvrCommand: public Command
    {
    private:
        size_t fragmentNumber = 1;
        size_t fragmentCount = 1;

        std::vector<unsigned char> commandPacket;

    public:
        AvrCommand() {
            this->setCommandId(0x80);
        }

        /**
         * Constructs raw command data on the fly.
         *
         * @return
         */
        std::vector<unsigned char> getData() const override;

        size_t getFragmentNumber() const {
            return this->fragmentNumber;
        }

        void setFragmentNumber(size_t fragmentNumber) {
            this->fragmentNumber = fragmentNumber;
        }

        size_t getFragmentCount() const {
            return this->fragmentCount;
        }

        void setFragmentCount(size_t fragmentCount) {
            this->fragmentCount = fragmentCount;
        }

        const std::vector<unsigned char>& getCommandPacket() const {
            return this->commandPacket;
        }

        void setCommandPacket(const std::vector<unsigned char>& commandPacket) {
            this->commandPacket = commandPacket;
        }
    };

}
