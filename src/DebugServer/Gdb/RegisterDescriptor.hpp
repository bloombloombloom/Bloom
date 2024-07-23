#pragma once

#include <cstdint>

namespace DebugServer::Gdb
{
    using GdbRegisterId = std::uint16_t;

    /*
     * GDB defines a set of registers for each target architecture.
     *
     * Each register in the set is assigned an ID, which is used to identify the register. Although the mapping of
     * registers to IDs is hardcoded in GDB, GDB server implementations are expected to be aware of this mapping.
     */
    struct RegisterDescriptor
    {
        GdbRegisterId id;
        std::uint16_t size;

        RegisterDescriptor(GdbRegisterId id, std::uint16_t size)
            : id(id)
            , size(size)
        {};

        bool operator == (const RegisterDescriptor& other) const {
            return this->id == other.id;
        }

        bool operator != (const RegisterDescriptor& other) const {
            return !(*this == other);
        }

        bool operator < (const RegisterDescriptor& rhs) const {
            return this->id < rhs.id;
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
