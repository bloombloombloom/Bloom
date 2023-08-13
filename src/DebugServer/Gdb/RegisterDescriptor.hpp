#pragma once

#include <cstdint>
#include <string>

namespace DebugServer::Gdb
{
    using GdbRegisterId = std::uint16_t;

    /*
     * GDB defines a set of registers for each target architecture.
     *
     * Each register in the set is assigned an ID, which is used to identify the registers. Although the mapping of
     * registers to IDs is hardcoded in GDB, GDB server implementations are expected to be aware of this mapping.
     */
    struct RegisterDescriptor
    {
        GdbRegisterId id;
        std::uint16_t size;
        std::string name;

        RegisterDescriptor(GdbRegisterId id, std::uint16_t size, const std::string& name)
            : id(id)
            , size(size)
            , name(name)
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

namespace std
{
    /**
     * Hashing function for RegisterDescriptor type.
     *
     * This is required in order to use RegisterDescriptor as a key in an std::unordered_map (see the BiMap
     * class).
     */
    template<>
    class hash<DebugServer::Gdb::RegisterDescriptor>
    {
    public:
        std::size_t operator () (const DebugServer::Gdb::RegisterDescriptor& descriptor) const {
            // We use the GDB register number as the hash, as it is unique to the register.
            return static_cast<size_t>(descriptor.id);
        }
    };
}
