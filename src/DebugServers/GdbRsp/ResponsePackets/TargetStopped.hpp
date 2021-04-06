#pragma once

#include "ResponsePacket.hpp"
#include "../Signal.hpp"
#include "../StopReason.hpp"
#include "src/Targets/TargetRegister.hpp"

namespace Bloom::DebugServers::Gdb::ResponsePackets
{
    using Bloom::Targets::TargetRegisterMap;

    /**
     * The TargetStopped class implements the response packet structure for any commands that expect a "StopReply"
     * packet in response.
     */
    class TargetStopped: public ResponsePacket
    {
    public:
        Signal signal;
        std::optional<TargetRegisterMap> registerMap;
        std::optional<StopReason> stopReason;

        TargetStopped(Signal signal): signal(signal) {}

        std::vector<unsigned char> getData() const override {
            std::string output = "T" + this->dataToHex({static_cast<unsigned char>(this->signal)});

            if (this->stopReason.has_value()) {
                auto stopReasonMapping = getStopReasonToNameMapping();
                auto stopReasonName = stopReasonMapping.valueAt(this->stopReason.value());

                if (stopReasonName.has_value()) {
                    output += stopReasonName.value() + ":;";
                }
            }

            if (this->registerMap.has_value()) {
                for (const auto& [registerId, registerValue] : this->registerMap.value()) {
                    output += this->dataToHex({static_cast<unsigned char>(registerId)});
                    output += ":" + this->dataToHex(registerValue.value) + ";";
                }
            }

            return std::vector<unsigned char>(output.begin(), output.end());
        }
    };
}
