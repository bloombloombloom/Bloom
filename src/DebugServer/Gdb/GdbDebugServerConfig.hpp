#pragma once

#include <cstdint>
#include "src/ProjectConfig.hpp"

namespace Bloom::DebugServer::Gdb
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

        explicit GdbDebugServerConfig(const DebugServerConfig& debugServerConfig);
    };
}
