#pragma once

#include <cstdint>
#include <vector>

namespace Services
{
    class IntegerService
    {
    public:
        static std::uint64_t toUint64(const std::vector<unsigned char>& data);
    };
}
