#pragma once

#include <variant>
#include <type_traits>

template<typename SuccessType, typename ErrorType>
struct Expected
{
    Expected(SuccessType&& result)
        requires (
            !std::is_reference_v<SuccessType>
            && !std::is_fundamental_v<SuccessType>
        )
        : result(std::move(result))
    {}

    Expected(const SuccessType& result)
        : result(result)
    {}

    Expected(const ErrorType& result)
        : result(result)
    {}

    bool hasValue() const {
        return std::holds_alternative<SuccessType>(this->result);
    }

    const SuccessType& value() const {
        return std::get<SuccessType>(this->result);
    }

    const ErrorType& error() const {
        return std::get<ErrorType>(this->result);
    }

private:
    std::variant<SuccessType, ErrorType> result;
};
