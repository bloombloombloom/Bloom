#include "TargetPadDescriptor.hpp"

#include "src/Services/StringService.hpp"

namespace Targets
{
    TargetPadDescriptor::TargetPadDescriptor(
        const std::string& key,
        const std::string& name,
        TargetPadType type
    )
        : id(TargetPadDescriptor::generateId(key))
        , key(key)
        , name(name)
        , type(type)
    {}

    bool TargetPadDescriptor::operator == (const TargetPadDescriptor& other) const {
        return this->id == other.id;
    }

    bool TargetPadDescriptor::operator != (const TargetPadDescriptor& other) const {
        return !(*this == other);
    }

    TargetPadId TargetPadDescriptor::generateId(const std::string& padKey) {
        return Services::StringService::generateUniqueInteger(padKey);
    }
}
