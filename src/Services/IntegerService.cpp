#include "IntegerService.hpp"

#include <cassert>

namespace Services
{
    std::uint64_t IntegerService::toUint64(const std::vector<unsigned char>& data) {
        assert(data.size() <= 8);
        auto output = std::uint64_t{0};

        for (const auto& byte : data) {
            output = (output << 8) | byte;
        }

        return output;
    }
}
