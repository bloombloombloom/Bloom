#pragma once

namespace DebugServer::Gdb
{
    enum class Signal: unsigned char
    {
        TRAP = 5,
        INTERRUPTED = 2,
    };
}
