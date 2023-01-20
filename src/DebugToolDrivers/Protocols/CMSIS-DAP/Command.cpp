#include "Command.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap
{
    Command::Command(unsigned char commandId)
        : id(commandId)
    {}

    std::vector<unsigned char> Command::rawCommand() const {
        auto rawCommand = std::vector<unsigned char>(1, this->id);
        rawCommand.reserve(this->data.size() + 1);
        rawCommand.insert(rawCommand.end(), this->data.begin(), this->data.end());

        return rawCommand;
    }
}
