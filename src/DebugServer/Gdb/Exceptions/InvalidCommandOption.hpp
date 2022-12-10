#pragma once

#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugServer::Gdb::Exceptions
{
    /**
     * For GDB monitor commands, each command can define a set of required/optional command options.
     * This exception is thrown if an invalid option is given.
     *
     * This exception is typically thrown and caught within the handling of monitor commands.
     */
    class InvalidCommandOption: public Bloom::Exceptions::Exception
    {
    public:
        explicit InvalidCommandOption(const std::string& message)
            : Bloom::Exceptions::Exception(message)
        {}

        explicit InvalidCommandOption(const char* message)
            : Bloom::Exceptions::Exception(message)
        {}

        explicit InvalidCommandOption() = default;
    };
}
