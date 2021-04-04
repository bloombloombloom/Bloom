#pragma once

#include <cstdint>
#include <vector>

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap
{
    class Response
    {
    private:
        unsigned char responseId = 0x00;

        std::vector<unsigned char> data;

    protected:
        void setResponseId(unsigned char commandId) {
            this->responseId = commandId;
        }

        void setData(const std::vector<unsigned char>& data) {
            this->data = data;
        }

    public:
        Response() = default;
        virtual void init(unsigned char* response, std::size_t length);
        virtual void init(std::vector<unsigned char> response) {
            this->init(response.data(), response.size());
        }

        unsigned char getResponseId() const {
            return this->responseId;
        }

        virtual const std::vector<unsigned char>& getData() const {
            return this->data;
        }

        virtual ~Response() = default;
    };
}
