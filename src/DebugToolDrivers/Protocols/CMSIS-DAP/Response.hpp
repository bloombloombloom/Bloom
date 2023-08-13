#pragma once

#include <vector>

namespace DebugToolDrivers::Protocols::CmsisDap
{
    class Response
    {
    public:
        unsigned char id = 0x00;
        std::vector<unsigned char> data;

        Response(const std::vector<unsigned char>& rawResponse);
        virtual ~Response() = default;

        Response(const Response& other) = default;
        Response(Response&& other) = default;

        Response& operator = (const Response& other) = default;
        Response& operator = (Response&& other) = default;
    };
}
