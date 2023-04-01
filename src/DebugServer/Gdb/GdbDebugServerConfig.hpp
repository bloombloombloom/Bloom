#pragma once

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

        /**
         * GDB tends to remove all breakpoints when target execution stops, and then installs them again, just before
         * resuming target execution. This can result in excessive wearing of the target's program memory, as well as
         * a negative impact on performance.
         *
         * When breakpoint caching is enabled, Bloom's GDB server will perform internal bookkeeping of the breakpoints
         * installed and removed via GDB. Then, just before resuming target execution, it will only apply the necessary
         * changes to the target, avoiding the excessive wear and IO.
         *
         * This param is optional, and is enabled by default.
         */
        bool breakpointCachingEnabled = true;

        explicit GdbDebugServerConfig(const DebugServerConfig& debugServerConfig);
    };
}
