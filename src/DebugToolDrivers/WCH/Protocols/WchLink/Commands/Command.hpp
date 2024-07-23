#pragma once

#include <cstdint>
#include <cassert>
#include <vector>
#include <array>

#include "src/DebugToolDrivers/WCH/Protocols/WchLink/Responses/Response.hpp"


namespace DebugToolDrivers::Wch::Protocols::WchLink::Commands
{
    template <class PayloadContainerType = std::vector<unsigned char>>
    class Command
    {
        static_assert(
            std::is_same_v<typename PayloadContainerType::value_type, unsigned char>,
            "Invalid payload container type"
        );

    public:
        using ExpectedResponseType = Responses::Response;

        std::uint8_t commandId;
        PayloadContainerType payload;

        explicit Command(std::uint8_t commandId)
            : commandId(commandId)
        {};

        virtual ~Command() = default;

        Command(const Command& other) = default;
        Command(Command&& other) noexcept = default;

        Command& operator = (const Command& other) = default;
        Command& operator = (Command&& other) noexcept = default;

        [[nodiscard]] auto getRawCommand() const {
            assert(this->payload.size() <= 256);

            auto rawCommand = std::vector<unsigned char>(3 + this->payload.size());

            rawCommand[0] = 0x81;
            rawCommand[1] = this->commandId;
            rawCommand[2] = static_cast<std::uint8_t>(this->payload.size());

            if (!this->payload.empty()) {
                std::copy(this->payload.begin(), this->payload.end(), rawCommand.begin() + 3);
            }

            return rawCommand;
        }
    };
}
