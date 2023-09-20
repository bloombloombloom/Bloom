#pragma once

#include <cstdint>
#include <optional>
#include <map>

#include "TargetDescriptor.hpp"
#include "GdbDebugServerConfig.hpp"
#include "Connection.hpp"
#include "Feature.hpp"
#include "ProgrammingSession.hpp"
#include "RangeSteppingSession.hpp"

#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetBreakpoint.hpp"

#include "src/Services/TargetControllerService.hpp"

namespace DebugServer::Gdb
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
         * The current server configuration.
         */
        const GdbDebugServerConfig& serverConfig;

        /**
         * During a debug session, we can set two types of breakpoints: internal and external.
         *
         * - Internal breakpoints are set by Bloom's GDB server, for reasons that are specific to Bloom's internals.
         *   For example, we use internal breakpoints to facilitate range stepping sessions, where we place
         *   intercepting breakpoints in places where we suspect the target will leave the stepping range.
         *
         * - External breakpoints are requested by the connected client (GDB), typically on behalf of the user.
         *   Sometimes GDB will set some internal breakpoints of its own, but from our perspective these are considered
         *   to be external breakpoints.
         *
         * We track internal and external breakpoints separately.
         */
        std::map<Targets::TargetMemoryAddress, Targets::TargetBreakpoint> internalBreakpointsByAddress;
        std::map<Targets::TargetMemoryAddress, Targets::TargetBreakpoint> externalBreakpointsByAddress;

        /**
         * When the GDB client is waiting for the target to halt, this is set to true so we know when to notify the
         * client.
         */
        bool waitingForBreak = false;

        bool pendingInterrupt = false;

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

        /**
         * When we're range stepping, we maintain a range stepping session, which holds all info related to that
         * particular range stepping session.
         *
         * This member should only be populated during a range stepping session.
         */
        std::optional<RangeSteppingSession> activeRangeSteppingSession = std::nullopt;

        DebugSession(
            Connection&& connection,
            const std::set<std::pair<Feature, std::optional<std::string>>>& supportedFeatures,
            const TargetDescriptor& targetDescriptor,
            const GdbDebugServerConfig& serverConfig
        );

        DebugSession(const DebugSession& other) = delete;
        DebugSession(DebugSession&& other) = delete;

        DebugSession& operator = (const DebugSession& other) = delete;
        DebugSession& operator = (DebugSession&& other) = delete;

        virtual ~DebugSession();

        virtual void setInternalBreakpoint(
            Targets::TargetMemoryAddress address,
            Services::TargetControllerService& targetControllerService
        );

        virtual void removeInternalBreakpoint(
            Targets::TargetMemoryAddress address,
            Services::TargetControllerService& targetControllerService
        );

        virtual void setExternalBreakpoint(
            Targets::TargetMemoryAddress address,
            Services::TargetControllerService& targetControllerService
        );

        virtual void removeExternalBreakpoint(
            Targets::TargetMemoryAddress address,
            Services::TargetControllerService& targetControllerService
        );

        virtual void startRangeSteppingSession(
            RangeSteppingSession&& session,
            Services::TargetControllerService& targetControllerService
        );

        virtual void terminateRangeSteppingSession(
            Services::TargetControllerService& targetControllerService
        );
    };
}
