#pragma once

#include <cstdint>

#include "TargetDescriptor.hpp"
#include "Connection.hpp"
#include "Feature.hpp"

namespace Bloom::DebugServer::Gdb
{
    class DebugSession
    {
    public:
        /**
         * The connection serving this debug session.
         */
        Connection connection;

        /**
         * A set of all GDB features supported for this debug session, along with any optional values (some GDB
         * features can be specified with a value, such as Feature::PACKET_SIZE).
         */
        std::set<std::pair<Feature, std::optional<std::string>>> supportedFeatures;

        /**
         * The GDB target descriptor of the connected target.
         */
        const TargetDescriptor& gdbTargetDescriptor;

        /**
         * When the GDB client is waiting for the target to halt, this is set to true so we know when to notify the
         * client.
         */
        bool waitingForBreak = false;

        DebugSession(
            Connection&& connection,
            const std::set<std::pair<Feature, std::optional<std::string>>>& supportedFeatures,
            const TargetDescriptor& targetDescriptor
        );

        DebugSession(const DebugSession& other) = delete;
        DebugSession(DebugSession&& other) = delete;

        DebugSession& operator = (const DebugSession& other) = delete;
        DebugSession& operator = (DebugSession&& other) = delete;

        ~DebugSession();
    };
}
