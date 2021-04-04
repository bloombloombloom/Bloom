#pragma once

namespace Bloom::DebugServers::Gdb
{
    enum class Signal: unsigned char
    {
        TRAP = 5,
        INTERRUPTED = 2,
    };
}
