#pragma once

#include "ResponsePacket.hpp"
#include "../Signal.hpp"
#include "../StopReason.hpp"
#include "src/Targets/TargetRegister.hpp"

namespace Bloom::DebugServer::Gdb::ResponsePackets
{
    /**
     * The TargetStopped class implements the response packet structure for any commands that expect a "StopReply"
     * packet in response.
     */
    class TargetStopped: public ResponsePacket
    {
    public:
        Signal signal;
        std::optional<StopReason> stopReason;

        explicit TargetStopped(Signal signal): signal(signal) {}

        [[nodiscard]] std::vector<unsigned char> getData() const override {
            std::string output = "T" + Packet::toHex(std::vector({static_cast<unsigned char>(this->signal)}));

            if (this->stopReason.has_value()) {
                auto stopReasonMapping = getStopReasonToNameMapping();
                auto stopReasonName = stopReasonMapping.valueAt(this->stopReason.value());

                if (stopReasonName.has_value()) {
                    output += stopReasonName.value() + ":;";
                }
            }

            return std::vector<unsigned char>(output.begin(), output.end());
        }
    };
}
