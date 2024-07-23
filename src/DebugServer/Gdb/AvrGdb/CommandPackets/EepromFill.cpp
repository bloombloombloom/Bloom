#include "EepromFill.hpp"

#include <QByteArray>
#include <algorithm>

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

    EepromFill::EepromFill(Monitor&& monitorPacket, const TargetDescriptor& gdbTargetDescriptor)
        : Monitor(std::move(monitorPacket))
        , eepromAddressSpaceDescriptor(gdbTargetDescriptor.eepromAddressSpaceDescriptor)
        , eepromMemorySegmentDescriptor(gdbTargetDescriptor.eepromMemorySegmentDescriptor)
    {
        const auto fillValueOptionIt = this->commandOptions.find("value");
        if (fillValueOptionIt != this->commandOptions.end() && fillValueOptionIt->second.has_value()) {
            this->fillValue = Services::StringService::dataFromHex(*(fillValueOptionIt->second));
        }
    }

    void EepromFill::handle(
        Gdb::DebugSession& debugSession,
        const Gdb::TargetDescriptor& gdbTargetDescriptor,
        const Targets::TargetDescriptor& targetDescriptor,
        TargetControllerService& targetControllerService
    ) {
        Logger::info("Handling EepromFill packet");

        try {
            const auto eepromSize = this->eepromMemorySegmentDescriptor.size();
            const auto fillValueSize = this->fillValue.size();

            if (fillValueSize == 0) {
                throw InvalidCommandOption{"Fill value required"};
            }

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

            // Repeat this->fillValue until we've filled `data`
            while (data.size() < eepromSize) {
                data.insert(
                    data.end(),
                    this->fillValue.begin(),
                    this->fillValue.begin() + static_cast<std::int32_t>(
                        std::min(fillValueSize, (eepromSize - data.size()))
                    )
                );
            }

            const auto hexValues = Services::StringService::toHex(data);
            Logger::debug("Filling EEPROM with values: " + hexValues);

            targetControllerService.writeMemory(
                this->eepromAddressSpaceDescriptor,
                this->eepromMemorySegmentDescriptor,
                this->eepromMemorySegmentDescriptor.addressRange.startAddress,
                std::move(data)
            );

            debugSession.connection.writePacket(ResponsePacket{Services::StringService::toHex(
                "Filled " + std::to_string(eepromSize) + " bytes of EEPROM, with values: " + hexValues + "\n"
            )});

        } catch (const InvalidCommandOption& exception) {
            Logger::error(exception.getMessage());
            debugSession.connection.writePacket(
                ResponsePacket{Services::StringService::toHex(exception.getMessage() + "\n")}
            );

        } catch (const Exception& exception) {
            Logger::error("Failed to fill EEPROM - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket{});
        }
    }
}
