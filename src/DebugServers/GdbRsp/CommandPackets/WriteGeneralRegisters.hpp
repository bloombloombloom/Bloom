#pragma once

#include <optional>

#include "CommandPacket.hpp"
#include "src/Targets/TargetRegister.hpp"

namespace Bloom::DebugServers::Gdb::CommandPackets
{
    /**
     * The WriteGeneralRegisters class implements the structure for "G" and "P" packets. Upon receiving this packet,
     * server is expected to write register values to the target.
     */
    class WriteGeneralRegisters: public CommandPacket
    {
    private:
        void init();

    public:
        Bloom::Targets::TargetRegisterMap registerMap;
        int registerNumber;
        std::vector<unsigned char> registerValue;

        WriteGeneralRegisters(std::vector<unsigned char> rawPacket): CommandPacket(rawPacket) {
            init();
        };

        virtual void dispatchToHandler(Gdb::GdbRspDebugServer& gdbRspDebugServer) override;
    };
}
