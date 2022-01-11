#pragma once

#include <cstdint>
#include <vector>

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap
{
    class Command
    {
    public:
        Command() = default;
        virtual ~Command() = default;

        Command(const Command& other) = default;
        Command(Command&& other) = default;

        Command& operator = (const Command& other) = default;
        Command& operator = (Command&& other) = default;

        [[nodiscard]] unsigned char getCommandId() const {
            return this->commandId;
        }

        void setCommandId(unsigned char commandId) {
            this->commandId = commandId;
        }

        [[nodiscard]] virtual std::vector<unsigned char> getData() const {
            return this->data;
        }

        void setData(const std::vector<unsigned char>& data) {
            this->data = data;
        }

        [[nodiscard]] int getCommandSize() const {
            // +1 for the command ID
            return static_cast<int>(1 + this->getData().size());
        }

        [[nodiscard]] std::uint16_t getDataSize() const {
            return static_cast<std::uint16_t>(this->getData().size());
        }

        /**
         * Converts instance of a CMSIS Command to a vector of unsigned char (buffer), for sending
         * to the debug tool.
         *
         * @return
         */
        explicit virtual operator std::vector<unsigned char>() const;

    private:
        unsigned char commandId = 0x00;
        std::vector<unsigned char> data;
    };
}
