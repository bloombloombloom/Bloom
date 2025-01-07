#include "CmsisDapInterface.hpp"

#include <thread>

namespace DebugToolDrivers::Protocols::CmsisDap
{
    using namespace Exceptions;

    CmsisDapInterface::CmsisDapInterface(Usb::HidInterface&& usbHidInterface)
        : usbHidInterface(std::move(usbHidInterface))
    {}

    void CmsisDapInterface::sendCommand(const Command& cmsisDapCommand) {
        if (this->commandDelay.count() > 0) {
            const auto now = duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()
            ).count();
            const auto difference = (now - this->lastCommandSentTimeStamp);

            if (difference < this->commandDelay.count()) {
                std::this_thread::sleep_for(std::chrono::milliseconds{this->commandDelay.count() - difference});
            }

            this->lastCommandSentTimeStamp = now;
        }

        this->getUsbHidInterface().write(cmsisDapCommand.rawCommand());
    }
}
