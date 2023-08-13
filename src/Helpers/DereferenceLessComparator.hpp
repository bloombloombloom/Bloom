#pragma once

#include <type_traits>

template<typename Type>
    requires std::is_pointer<Type>::value
struct DereferenceLessComparator
{
    constexpr bool operator () (const Type& lhs, const Type& rhs) const {
        return *lhs < *rhs;
    }
};
