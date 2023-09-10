#pragma once

#include <cstdint>

#include "TargetDescriptor.hpp"
#include "DebugSession.hpp"

#include "src/DebugServer/Gdb/GdbRspDebugServer.hpp"

namespace DebugServer::Gdb::AvrGdb
{
    class AvrGdbRsp: public GdbRspDebugServer
    {
    public:
        AvrGdbRsp(
            const DebugServerConfig& debugServerConfig,
            EventListener& eventListener,
            EventFdNotifier& eventNotifier
        );

        std::string getName() const override {
            return "AVR GDB Remote Serial Protocol Debug Server";
        }

    protected:
        DebugSession* startDebugSession(Connection&& connection) override;

        void endDebugSession() override;

        const Gdb::TargetDescriptor& getGdbTargetDescriptor() override;

        DebugSession* getActiveDebugSession() override;

        std::unique_ptr<Gdb::CommandPackets::CommandPacket> resolveCommandPacket(
            const RawPacket& rawPacket
        ) override;

        /**
         * Should return a set of GDB features supported by the AVR GDB server. Each supported feature may come with an
         * optional value.
         *
         * The set of features returned by this function will be stored against the active debug session object.
         *
         * @return
         */
        std::set<std::pair<Feature, std::optional<std::string>>> getSupportedFeatures();

        void handleTargetStoppedGdbResponse(Targets::TargetProgramCounter programAddress) override;

    private:
        TargetDescriptor gdbTargetDescriptor;

        /**
         * When a connection with a GDB client is established, a new instance of the DebugSession class is created and
         * held here. A value of std::nullopt means there is no active debug session present.
         */
        std::optional<DebugSession> activeDebugSession;
    };
}
