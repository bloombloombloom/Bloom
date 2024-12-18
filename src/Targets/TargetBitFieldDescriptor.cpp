#include "TargetBitFieldDescriptor.hpp"

#include <bitset>
#include <limits>
#include <numeric>

namespace Targets
{
    TargetBitFieldDescriptor::TargetBitFieldDescriptor(
        const std::string& key,
        const std::string& name,
        std::uint64_t mask,
        std::optional<std::string> description
    )
        : key(key)
        , name(name)
        , mask(mask)
        , description(description)
    {}

    std::size_t TargetBitFieldDescriptor::width() const {
        const auto maskBitset = std::bitset<std::numeric_limits<decltype(TargetBitFieldDescriptor::mask)>::digits>{
            this->mask
        };

        auto width = std::size_t{0};
        for (auto maskIndex = std::size_t{0}; maskIndex < maskBitset.size(); ++maskIndex) {
            if (maskBitset[maskIndex]) {
                ++width;
            }
        }

        return width;
    }

    TargetBitFieldDescriptor TargetBitFieldDescriptor::clone() const {
        return {*this};
    }
}
