#pragma once

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AvrCommandFrame.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/ResponseFrames/HouseKeeping/HouseKeepingResponseFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::HouseKeeping
{
    enum class ResponseId : unsigned char
    {
        OK = 0x80,
        LIST = 0x81,
        DATA = 0x84,
        FAILED = 0xA0,
        FAILED_WITH_DATA = 0xA1
    };

    inline bool operator == (unsigned char rawId, ResponseId id) {
        return static_cast<unsigned char>(id) == rawId;
    }

    inline bool operator == (ResponseId id, unsigned char rawId) {
        return rawId == id;
    }

    template<class PayloadContainerType>
    class HouseKeepingCommandFrame: public AvrCommandFrame<PayloadContainerType>
    {
    public:
        using ExpectedResponseFrameType = ResponseFrames::HouseKeepingResponseFrame;

        HouseKeepingCommandFrame(): AvrCommandFrame<PayloadContainerType>(ProtocolHandlerId::HOUSE_KEEPING) {}
    };
}
