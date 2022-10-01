#pragma once

#include <cstdint>
#include <string>

namespace Bloom::DebugServer::Gdb
{
    using GdbRegisterNumber = int;

    /*
     * GDB defines a set of registers for each target architecture.
     *
     * Each register in the set is assigned a register number, which is used to identify the register.
     */
    struct RegisterDescriptor
    {
        GdbRegisterNumber number;
        std::uint16_t size;
        std::string name;

        RegisterDescriptor(GdbRegisterNumber number, std::uint16_t size, const std::string& name)
            : number(number)
            , size(size)
            , name(name)
        {};

        bool operator == (const RegisterDescriptor& other) const {
            return this->number == other.number;
        }

        bool operator != (const RegisterDescriptor& other) const {
            return !(*this == other);
        }

        bool operator < (const RegisterDescriptor& rhs) const {
            return this->number < rhs.number;
        }

        bool operator > (const RegisterDescriptor& rhs) const {
            return rhs < *this;
        }

        bool operator <= (const RegisterDescriptor& rhs) const {
            return !(rhs < *this);
        }

        bool operator >= (const RegisterDescriptor& rhs) const {
            return !(*this < rhs);
        }
    };
}

namespace std
{
    /**
     * Hashing function for RegisterDescriptor type.
     *
     * This is required in order to use RegisterDescriptor as a key in an std::unordered_map (see the BiMap
     * class).
     */
    template<>
    class hash<Bloom::DebugServer::Gdb::RegisterDescriptor>
    {
    public:
        std::size_t operator () (const Bloom::DebugServer::Gdb::RegisterDescriptor& descriptor) const {
            // We use the GDB register number as the hash, as it is unique to the register.
            return static_cast<size_t>(descriptor.number);
        }
    };
}
