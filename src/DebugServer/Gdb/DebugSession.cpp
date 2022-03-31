#include "DebugSession.hpp"

#include "src/Logger/Logger.hpp"

namespace Bloom::DebugServer::Gdb
{
    DebugSession::DebugSession(Connection&& connection, const TargetDescriptor& targetDescriptor)
        : connection(std::move(connection))
        , targetDescriptor(targetDescriptor)
    {}

    void DebugSession::terminate() {

    }
}
