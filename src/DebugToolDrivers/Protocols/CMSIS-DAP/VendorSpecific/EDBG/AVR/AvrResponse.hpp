#pragma once

#include <cstdint>
#include <vector>

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/Response.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr
{
    class AvrResponse: public Response
    {
    public:
        explicit AvrResponse(const std::vector<unsigned char>& rawResponse);

        [[nodiscard]] std::uint8_t getFragmentNumber() const {
            return this->fragmentNumber;
        }

        [[nodiscard]] std::uint8_t getFragmentCount() const {
            return this->fragmentCount;
        }

        [[nodiscard]] const std::vector<unsigned char>& getResponsePacket() const {
            return this->responsePacket;
        }

    protected:
        void setFragmentNumber(std::uint8_t fragmentNumber) {
            this->fragmentNumber = fragmentNumber;
        }

        void setFragmentCount(std::uint8_t fragmentCount) {
            this->fragmentCount = fragmentCount;
        }

        void setResponsePacket(const std::vector<unsigned char>& responsePacket) {
            this->responsePacket = responsePacket;
        }

    private:
        std::uint8_t fragmentNumber = 0;
        std::uint8_t fragmentCount = 0;

        std::vector<unsigned char> responsePacket;
    };
}
