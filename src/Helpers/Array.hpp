#pragma once

#include <array>
#include <concepts>
#include <cstddef>

class Array
{
public:
    template<std::size_t size, std::semiregular ValueType>
    static constexpr auto repeat(const ValueType& value) {
        auto output = std::array<ValueType, size>{};
        output.fill(value);
        return output;
    }
};
