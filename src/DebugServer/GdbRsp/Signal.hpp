#pragma once

namespace Bloom::DebugServer::Gdb
{
    enum class Signal: unsigned char
    {
        TRAP = 5,
        INTERRUPTED = 2,
    };
}
