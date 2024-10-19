#pragma once

#include <cstdint>

#include "src/ProjectConfig.hpp"

namespace DebugServer::Gdb
{
    /**
     * Extending the generic DebugServerConfig struct to accommodate GDB debug server configuration parameters.
     */
    class GdbDebugServerConfig: public DebugServerConfig
    {
    public:
        /**
         * The port number for the GDB server to listen on.
         *
         * This parameter is optional. If not specified, the default value set here will be used.
         */
        std::uint16_t listeningPortNumber = 1442;

        /**
         * The address for the GDB server to listen on.
         *
         * This parameter is optional. If not specified, the default value set here will be used.
         */
        std::string listeningAddress = "127.0.0.1";

        /**
         * Controls Bloom's range stepping functionality.
         *
         * If this is set to true, the GDB server will service "vCont;r" commands from GDB.
         *
         * This parameter is optional. If not specified, the default value set here will be used.
         */
        bool rangeStepping = true;

        /**
         * Determines whether Bloom will seek to disable packet acknowledgement with GDB, at the start of the debug
         * session.
         *
         * If this is set to false, Bloom will communicate its ability to disable package acknowledgment to GDB.
         * GDB may then send the appropriate packet to disable packet acknowledgment. However, this isn't
         * guaranteed - GDB may be configured to keep packet acknowledgment enabled (via the
         * `set remote noack-packet off` command).
         *
         * This parameter is optional. If not specified, the default value set here will be used.
         */
        bool packetAcknowledgement = false;

        explicit GdbDebugServerConfig(const DebugServerConfig& debugServerConfig);
    };
}
