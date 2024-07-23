#include "TargetPhysicalInterface.hpp"

namespace Targets
{
    std::map<TargetPhysicalInterface, std::string> getPhysicalInterfaceNames() {
        return std::map<TargetPhysicalInterface, std::string>{
            {TargetPhysicalInterface::ISP, "ISP"},
            {TargetPhysicalInterface::DEBUG_WIRE, "debugWIRE"},
            {TargetPhysicalInterface::PDI, "PDI"},
            {TargetPhysicalInterface::JTAG, "JTAG"},
            {TargetPhysicalInterface::UPDI, "UPDI"},
        };
    }
}
