#pragma once

#include <vector>

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap
{
    class Response
    {
    public:
        Response() = default;
        virtual ~Response() = default;

        virtual void init(const std::vector<unsigned char>& rawResponse);

        [[nodiscard]] unsigned char getResponseId() const {
            return this->responseId;
        }

        [[nodiscard]] virtual const std::vector<unsigned char>& getData() const {
            return this->data;
        }

    protected:
        void setResponseId(unsigned char commandId) {
            this->responseId = commandId;
        }

        void setData(const std::vector<unsigned char>& data) {
            this->data = data;
        }

    private:
        unsigned char responseId = 0x00;
        std::vector<unsigned char> data;
    };
}
