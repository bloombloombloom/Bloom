#include "AvrGdbRsp.hpp"

namespace Bloom::DebugServers::Gdb::AvrGdb
{
    using namespace Bloom::Exceptions;

    using Bloom::Targets::TargetRegisterDescriptor;
    using Bloom::Targets::TargetRegisterType;

    void AvrGdbRsp::init() {
        DebugServers::Gdb::GdbRspDebugServer::init();

        this->gdbTargetDescriptor = TargetDescriptor(this->targetDescriptor);
    }
}
