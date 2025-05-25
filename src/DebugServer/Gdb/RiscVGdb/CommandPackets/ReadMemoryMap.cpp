#include "ReadMemoryMap.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ResponsePacket.hpp"

#include "src/Services/StringService.hpp"
#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::RiscVGdb::CommandPackets
{
    using Services::TargetControllerService;
    using Services::StringService;

    using ResponsePackets::ResponsePacket;

    using Exceptions::Exception;

    ReadMemoryMap::ReadMemoryMap(const RawPacket& rawPacket)
        : CommandPacket(rawPacket)
    {
        if (this->data.size() < 26) {
            throw Exception{"Invalid packet length"};
        }

        /*
         * The read memory map ('qXfer:memory-map:read::...') packet consists of two segments, an offset and a length.
         * These are separated by a comma character.
         */
        const auto command = std::string{this->data.begin() + 23, this->data.end()};

        const auto delimiterPos = command.find_first_of(',');
        if (delimiterPos == std::string::npos) {
            throw Exception{"Invalid packet"};
        }

        this->offset = StringService::toUint32(command.substr(0, delimiterPos), 16);
        this->length = StringService::toUint32(command.substr(delimiterPos + 1), 16);
    }

    void ReadMemoryMap::handle(
        Gdb::DebugSession& debugSession,
        const RiscVGdbTargetDescriptor& gdbTargetDescriptor,
        const Targets::TargetDescriptor& targetDescriptor,
        const Targets::TargetState& targetState,
        TargetControllerService& targetControllerService
    ) {
        using Targets::TargetMemorySegmentType;

        Logger::info("Handling ReadMemoryMap packet");

        static const auto gdbMemoryTypeFromSegment = [] (
            const Targets::TargetMemorySegmentDescriptor& segmentDescriptor
        ) -> std::optional<std::string> {
            switch (segmentDescriptor.type) {
                case TargetMemorySegmentType::FLASH:
                case TargetMemorySegmentType::ALIASED: {
                    return "flash";
                }
                case TargetMemorySegmentType::RAM:
                case TargetMemorySegmentType::IO: {
                    return "ram";
                }
                default: {
                    return std::nullopt;
                }
            }
        };

        auto memoryMap = std::string{"<memory-map>\n"};

        for (const auto& segmentDescriptor : gdbTargetDescriptor.systemAddressSpaceDescriptor.segmentDescriptorsByKey | std::views::values) {
            const auto gdbMemType = gdbMemoryTypeFromSegment(segmentDescriptor);
            if (!gdbMemType.has_value()) {
                continue;
            }

            const auto segmentWritable = (
                segmentDescriptor.debugModeAccess.writeable || segmentDescriptor.programmingModeAccess.writeable
            );

            memoryMap += "<memory type=\"" + (!segmentWritable ? "rom" : *gdbMemType) + "\" start=\"0x"
                + StringService::toHex(segmentDescriptor.addressRange.startAddress) + "\" length=\""
                + std::to_string(segmentDescriptor.size()) + "\"";

            if (segmentWritable && segmentDescriptor.pageSize.has_value()) {
                memoryMap += ">\n    <property name=\"blocksize\">" + std::to_string(*(segmentDescriptor.pageSize))
                    + "</property>\n</memory>\n";
            } else {
                memoryMap += "/>\n";
            }
        }

        memoryMap += "</memory-map>";

        auto responseData = std::vector<unsigned char>{'l'};

        if (this->offset < memoryMap.size() && this->length > 0) {
            responseData.insert(
                responseData.end(),
                memoryMap.begin() + this->offset,
                memoryMap.begin() + this->offset + std::min(
                    static_cast<long>(this->length),
                    static_cast<long>(memoryMap.size() - this->offset)
                )
            );
        }

        debugSession.connection.writePacket(ResponsePacket{responseData});
    }
}
