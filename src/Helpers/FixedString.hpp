#pragma once

#include <cstddef>
#include <algorithm>

template <std::size_t length>
struct FixedString
{
    char value[length];

    constexpr FixedString(const char (&str)[length]) {
        std::copy_n(str, length, this->value);
    }
};
