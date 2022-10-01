#include "Command.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap
{
    Command::Command(unsigned char commandId)
        : id(commandId)
    {}

    Command::operator std::vector<unsigned char>() const {
        auto rawCommand = std::vector<unsigned char>(1, this->id);
        rawCommand.insert(rawCommand.end(), this->data.begin(), this->data.end());

        return rawCommand;
    }
}
