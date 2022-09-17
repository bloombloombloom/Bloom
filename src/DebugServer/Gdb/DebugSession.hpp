#pragma once

#include <cstdint>
#include <optional>

#include "TargetDescriptor.hpp"
#include "Connection.hpp"
#include "Feature.hpp"
#include "ProgrammingSession.hpp"

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

        /**
         * When the user attempts to program the target via GDB's 'load' command, GDB will send a number of
         * FlashWrite (vFlashWrite) packets to Bloom. We group the data in these packets and flush it all at once, upon
         * receiving a FlashDone (vFlashDone) packet.
         *
         * The grouped data is held in a ProgrammingSession object, against the active debug session. Once the data has
         * been flushed, the ProgrammingSession object is destroyed.
         *
         * See the ProgrammingSession struct and GDB RSP documentation for more.
         *
         * This member holds the current (if any) ProgrammingSession object. It should only be populated during
         * programming.
         */
        std::optional<ProgrammingSession> programmingSession = std::nullopt;

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
