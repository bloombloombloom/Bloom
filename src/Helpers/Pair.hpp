#pragma once

template<typename FirstType, typename SecondType>
struct Pair
{
    FirstType first;
    SecondType second;

    Pair(FirstType&& first, SecondType&& second)
        requires (!std::is_reference_v<FirstType> && !std::is_reference_v<SecondType>)
        : first(std::move(first))
        , second(std::move(second))
    {}

    Pair(const FirstType& first, const SecondType& second)
        : first(first)
        , second(second)
    {}
};
