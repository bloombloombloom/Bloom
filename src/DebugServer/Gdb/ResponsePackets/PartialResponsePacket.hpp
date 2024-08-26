#pragma once

#include <string>

#include "ResponsePacket.hpp"

namespace DebugServer::Gdb::ResponsePackets
{
    class PartialResponsePacket: public ResponsePacket
    {
    public:
        PartialResponsePacket(const std::string& output)
            : ResponsePacket(std::vector<unsigned char>{'O'})
        {
            this->data.insert(this->data.end(), output.begin(), output.end());
        }
    };
}
