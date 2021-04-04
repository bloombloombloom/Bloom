#include "Command.hpp"

using namespace Bloom::DebugToolDrivers::Protocols::CmsisDap;

Command::operator std::vector<unsigned char> () const
{
    auto rawCommand = std::vector<unsigned char>(1, this->getCommandId());
    auto commandData = this->getData();
    rawCommand.insert(rawCommand.end(), commandData.begin(), commandData.end());

    return rawCommand;
}
