#include "EepromFill.hpp"

#include <QByteArray>
#include <algorithm>
#include <stdexcept>

#include "src/DebugServer/Gdb/ResponsePackets/ResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"

#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"

#include "src/DebugServer/Gdb/Exceptions/InvalidCommandOption.hpp"
#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::ResponsePacket;
    using ResponsePackets::ErrorResponsePacket;

    using ::Exceptions::Exception;
    using Exceptions::InvalidCommandOption;

    EepromFill::EepromFill(Gdb::CommandPackets::Monitor&& monitorPacket)
        : CommandPacket(monitorPacket)
        , rawFillValue(monitorPacket.commandArguments.size() >= 3 ? monitorPacket.commandArguments[2] : std::string{})
    {}

    void EepromFill::handle(
        DebugSession& debugSession,
        const AvrGdbTargetDescriptor& gdbTargetDescriptor,
        const Targets::TargetDescriptor& targetDescriptor,
        TargetControllerService& targetControllerService
    ) {
        Logger::info("Handling EepromFill packet");

        try {
            if (this->rawFillValue.empty()) {
                throw InvalidCommandOption{"Fill value required"};
            }

            const auto eepromSize = gdbTargetDescriptor.eepromMemorySegmentDescriptor.size();

            const auto fillValue = Services::StringService::dataFromHex(
                this->rawFillValue.size() >= 3 && this->rawFillValue[0] == '0'
                && (this->rawFillValue[1] == 'X' || this->rawFillValue[1] == 'x')
                    ? this->rawFillValue.substr(2)
                    : this->rawFillValue
            );
            const auto fillValueSize = fillValue.size();

            if (fillValueSize > eepromSize) {
                throw InvalidCommandOption{
                    "Fill value size (" + std::to_string(fillValueSize) + " bytes) exceeds EEPROM size ("
                        + std::to_string(eepromSize) + " bytes)"
                };
            }

            if ((eepromSize % fillValueSize) != 0) {
                Logger::warning(
                    "The fill value size (" + std::to_string(fillValueSize) + " bytes) is not a multiple of the EEPROM "
                        "size (" + std::to_string(eepromSize) + " bytes) - the fill value will be truncated"
                );
            }

            Logger::warning("Filling " + std::to_string(eepromSize) + " bytes of EEPROM");

            auto data = Targets::TargetMemoryBuffer{};
            data.reserve(eepromSize);

            // Repeat fillValue until we've filled `data`
            while (data.size() < eepromSize) {
                data.insert(
                    data.end(),
                    fillValue.begin(),
                    fillValue.begin() + static_cast<std::int32_t>(
                        std::min(fillValueSize, (eepromSize - data.size()))
                    )
                );
            }

            const auto hexValues = Services::StringService::toHex(data);
            Logger::debug("Filling EEPROM with values: " + hexValues);

            targetControllerService.writeMemory(
                gdbTargetDescriptor.eepromAddressSpaceDescriptor,
                gdbTargetDescriptor.eepromMemorySegmentDescriptor,
                gdbTargetDescriptor.eepromMemorySegmentDescriptor.addressRange.startAddress,
                std::move(data)
            );

            debugSession.connection.writePacket(ResponsePacket{Services::StringService::toHex(
                "Filled " + std::to_string(eepromSize) + " bytes of EEPROM, with values: " + hexValues + "\n"
            )});

        } catch (const InvalidCommandOption& exception) {
            debugSession.connection.writePacket(
                ResponsePacket{Services::StringService::toHex(exception.getMessage() + "\n")}
            );

        } catch (const std::invalid_argument& exception) {
            debugSession.connection.writePacket(
                ResponsePacket{Services::StringService::toHex("Invalid fill value\n")}
            );

        } catch (const Exception& exception) {
            Logger::error("Failed to fill EEPROM - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket{});
        }
    }
}
