#pragma once

#include "DiscoveryCommandFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Discovery
{
    /**
     * The query context is the type of query to execute.
     */
    enum class QueryContext : unsigned char
    {
        COMMAND_HANDLERS = 0x00,
        TOOL_NAME = 0x80,
        SERIAL_NUMBER = 0x81,
        MANUFACTURE_DATE = 0x82,
    };

    /**
     * The Discovery protocol handler only supports one command; the Query command. This command is used to
     * query information from the device, such as device capabilities, manufacture date, serial number, etc.
     */
    class Query: public DiscoveryCommandFrame
    {
    private:
        QueryContext context;

    public:
        Query() : DiscoveryCommandFrame() {}

        Query(QueryContext context) : DiscoveryCommandFrame() {
            this->setContext(context);
        }

        void setContext(QueryContext context) {
            this->context = context;
        }

        virtual std::vector<unsigned char> getPayload() const override {
            /*
             * The payload for the Query command consists of three bytes. A command ID (0x00), version (0x00) and a
             * query context.
             */
            auto output = std::vector<unsigned char>(3, 0x00);
            output[2] = static_cast<unsigned char>(this->context);

            return output;
        }
    };
}
