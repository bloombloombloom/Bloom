#include "CmsisDapInterface.hpp"

#include <thread>

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/Command.hpp"

namespace DebugToolDrivers::Protocols::CmsisDap
{
    using namespace Exceptions;

    CmsisDapInterface::CmsisDapInterface(Usb::HidInterface&& usbHidInterface)
        : usbHidInterface(std::move(usbHidInterface))
    {}

    void CmsisDapInterface::sendCommand(const Command& cmsisDapCommand) {
        if (this->msSendCommandDelay.count() > 0) {
            using namespace std::chrono;
            std::int64_t now = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
            std::int64_t difference = (now - this->lastCommandSentTimeStamp);

            if (difference < this->msSendCommandDelay.count()) {
                std::this_thread::sleep_for(milliseconds(this->msSendCommandDelay.count() - difference));
            }

            this->lastCommandSentTimeStamp = now;
        }

        this->getUsbHidInterface().write(cmsisDapCommand.rawCommand());
    }
}
