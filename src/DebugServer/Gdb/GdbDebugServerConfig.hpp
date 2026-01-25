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
         * If this is set to false, Bloom will communicate its ability to disable package acknowledgement to GDB.
         * GDB may then send the appropriate packet to disable packet acknowledgement. However, this isn't
         * guaranteed - GDB may be configured to keep packet acknowledgement enabled (via the
         * `set remote noack-packet off` command).
         *
         * This parameter is optional. If not specified, the default value set here will be used.
         */
        bool packetAcknowledgement = false;

        /**
         * Determines whether the debug server will force Bloom to shut down after receiving a detach packet from the
         * GDB client.
         *
         * This can be used to prevent aggressive IDEs from terminating Bloom's process without giving it a chance to
         * shut down gracefully, assuming the IDE sends a detach packet and waits for a response before killing Bloom's
         * process.
         */
        bool shutdownOnDetach = false;

        explicit GdbDebugServerConfig(const DebugServerConfig& debugServerConfig);
    };
}
