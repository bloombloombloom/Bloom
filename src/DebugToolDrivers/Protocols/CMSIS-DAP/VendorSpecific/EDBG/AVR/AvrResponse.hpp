#pragma once

#include <cstdint>
#include <vector>

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/Response.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr
{
    class AvrResponse: public Response
    {
    private:
        std::uint8_t fragmentNumber = 0;
        std::uint8_t fragmentCount = 0;

        std::vector<unsigned char> responsePacket;

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

    public:
        AvrResponse() = default;

        /**
         * Construct an AVRResponse object from a Response object.
         *
         * @param response
         */
        void init(const Response& response) {
            auto rawData = response.getData();
            rawData.insert(rawData.begin(), response.getResponseId());
            this->init(rawData.data(), rawData.size());
        }

        void init(unsigned char* response, std::size_t length) override;

        std::uint8_t getFragmentNumber() const {
            return this->fragmentNumber;
        }

        std::uint8_t getFragmentCount() const {
            return this->fragmentCount;
        }

        const std::vector<unsigned char>& getResponsePacket() const {
            return this->responsePacket;
        }
    };
}
