#pragma once

#include <stdexcept>

namespace Bloom::Exceptions
{
    class Exception: public std::runtime_error
    {
    public:
        explicit Exception()
            : std::runtime_error("")
        {}

        explicit Exception(const std::string& message)
            : std::runtime_error(message.c_str())
            , message(message)
        {}

        explicit Exception(const char* message)
            : std::runtime_error(message)
            , message(std::string(message))
        {}

        virtual ~Exception() = default;

        Exception(const Exception& other) noexcept = default;
        Exception(Exception&& other) = default;

        Exception& operator = (const Exception& other) = default;
        Exception& operator = (Exception&& other) = default;

        const char* what() const noexcept override {
            return this->message.c_str();
        }

        const std::string& getMessage() const {
            return this->message;
        }

    protected:
        std::string message;
    };
}
