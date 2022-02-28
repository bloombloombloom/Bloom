#pragma once

#include <vector>

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/Response.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/Edbg.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr
{
    enum class AvrEventId : unsigned char
    {
        AVR8_BREAK_EVENT = 0x40,
    };

    inline bool operator == (unsigned char rawId, AvrEventId id) {
        return static_cast<unsigned char>(id) == rawId;
    }

    inline bool operator == (AvrEventId id, unsigned char rawId) {
        return rawId == id;
    }

    class AvrEvent: public Response
    {
    public:
        explicit AvrEvent(const std::vector<unsigned char>& rawResponse);

        [[nodiscard]] const std::vector<unsigned char>& getEventData() const {
            return this->eventData;
        }

        [[nodiscard]] size_t getEventDataSize() const {
            return this->eventData.size();
        }

        [[nodiscard]] AvrEventId getEventId() const {
            return static_cast<AvrEventId>(this->eventId);
        }

    protected:
        void setEventData(const std::vector<unsigned char>& eventData) {
            this->eventData = eventData;
        }

    private:
        unsigned char eventId = 0;
        std::vector<unsigned char> eventData;
    };
}
