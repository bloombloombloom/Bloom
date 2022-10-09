#pragma once

#include <optional>

#include "ResponsePacket.hpp"

#include "src/DebugServer/Gdb/Signal.hpp"
#include "src/DebugServer/Gdb/StopReason.hpp"

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

        explicit TargetStopped(Signal signal, const std::optional<StopReason>& stopReason = std::nullopt)
            : signal(signal)
            , stopReason(stopReason)
        {
            std::string packetData = "T" + Packet::toHex(std::vector({static_cast<unsigned char>(this->signal)}));

            if (this->stopReason.has_value()) {
                static const auto stopReasonMapping = getStopReasonToNameMapping();
                const auto stopReasonName = stopReasonMapping.valueAt(this->stopReason.value());

                if (stopReasonName.has_value()) {
                    packetData += stopReasonName.value() + ":;";
                }
            }

            this->data = {packetData.begin(), packetData.end()};
        }
    };
}
