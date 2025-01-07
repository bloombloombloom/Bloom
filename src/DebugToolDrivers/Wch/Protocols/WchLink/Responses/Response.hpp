#pragma once

#include <vector>

namespace DebugToolDrivers::Wch::Protocols::WchLink::Responses
{
    class Response
    {
    public:
        std::vector<unsigned char> payload;

        explicit Response(const std::vector<unsigned char>& payload)
            : payload(payload)
        {}
    };
}
