#include "TargetRegister.hpp"

namespace Targets
{
    std::size_t TargetRegisterDescriptor::getHash() const {
        if (!this->cachedHash.has_value()) {
            auto stringHasher = std::hash<std::string>();

            this->cachedHash = stringHasher(std::to_string(this->startAddress.value_or(0))
                + "_" + std::to_string(static_cast<std::uint8_t>(this->type)));
        }

        return this->cachedHash.value();
    }
}
