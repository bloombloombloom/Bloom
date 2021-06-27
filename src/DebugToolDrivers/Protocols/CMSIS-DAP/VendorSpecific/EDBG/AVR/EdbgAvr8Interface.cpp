#include <cstdint>
#include <thread>
#include <math.h>

#include "EdbgAvr8Interface.hpp"
#include "src/Exceptions/InvalidConfig.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/Exceptions/Avr8CommandFailure.hpp"
#include "src/Logger/Logger.hpp"

// Command frames
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/SetParameter.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/GetParameter.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/ActivatePhysical.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/DeactivatePhysical.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/Attach.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/Detach.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/Stop.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/Step.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/Run.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/RunTo.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/GetDeviceId.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/Reset.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/ReadMemory.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/WriteMemory.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/GetProgramCounter.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/SetProgramCounter.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/DisableDebugWire.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/SetSoftwareBreakpoints.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/SetXmegaSoftwareBreakpoint.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/ClearAllSoftwareBreakpoints.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/ClearSoftwareBreakpoints.hpp"

// AVR events
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/Events/AVR8Generic/BreakEvent.hpp"

using namespace Bloom::Targets::Microchip::Avr;
using namespace Bloom::Targets::Microchip::Avr::Avr8Bit;
using namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr;
using namespace Bloom::Exceptions;

using Bloom::Targets::TargetState;
using Bloom::Targets::TargetMemoryType;
using Bloom::Targets::TargetMemoryBuffer;
using Bloom::Targets::TargetRegister;
using Bloom::Targets::TargetRegisterDescriptor;
using Bloom::Targets::TargetRegisterType;
using Bloom::Targets::TargetRegisters;

void EdbgAvr8Interface::setParameter(const Avr8EdbgParameter& parameter, const std::vector<unsigned char>& value) {
    auto commandFrame = CommandFrames::Avr8Generic::SetParameter(parameter, value);

    auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
    if (response.getResponseId() == Avr8ResponseId::FAILED) {
        throw Avr8CommandFailure("Failed to set parameter on device!", response);
    }
}

std::vector<unsigned char> EdbgAvr8Interface::getParameter(const Avr8EdbgParameter& parameter, std::uint8_t size) {
    auto commandFrame = CommandFrames::Avr8Generic::GetParameter(parameter, size);

    auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
    if (response.getResponseId() == Avr8ResponseId::FAILED) {
        throw Avr8CommandFailure("Failed to get parameter from device!", response);
    }

    return response.getPayloadData();
}

void EdbgAvr8Interface::setDebugWireAndJtagParameters() {
    if (this->targetParameters.flashPageSize.has_value()) {
        Logger::debug("Setting DEVICE_FLASH_PAGE_SIZE AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_FLASH_PAGE_SIZE,
            this->targetParameters.flashPageSize.value()
        );
    }

    if (this->targetParameters.flashSize.has_value()) {
        Logger::debug("Setting DEVICE_FLASH_SIZE AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_FLASH_SIZE,
            this->targetParameters.flashSize.value()
        );
    }

    if (this->targetParameters.flashStartAddress.has_value()) {
        Logger::debug("Setting DEVICE_FLASH_BASE AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_FLASH_BASE,
            this->targetParameters.flashStartAddress.value()
        );
    }

    if (this->targetParameters.ramStartAddress.has_value()) {
        Logger::debug("Setting DEVICE_SRAM_START AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_SRAM_START,
            this->targetParameters.ramStartAddress.value()
        );
    }

    if (this->targetParameters.eepromSize.has_value()) {
        Logger::debug("Setting DEVICE_EEPROM_SIZE AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_EEPROM_SIZE,
            this->targetParameters.eepromSize.value()
        );
    }

    if (this->targetParameters.eepromPageSize.has_value()) {
        Logger::debug("Setting DEVICE_EEPROM_PAGE_SIZE AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_EEPROM_PAGE_SIZE,
            this->targetParameters.eepromPageSize.value()
        );
    }

    if (this->targetParameters.ocdRevision.has_value()) {
        Logger::debug("Setting DEVICE_OCD_REVISION AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_OCD_REVISION,
            this->targetParameters.ocdRevision.value()
        );
    }

    if (this->targetParameters.ocdDataRegister.has_value()) {
        Logger::debug("Setting DEVICE_OCD_DATA_REGISTER AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_OCD_DATA_REGISTER,
            this->targetParameters.ocdDataRegister.value()
        );
    }

    if (this->targetParameters.spmcRegisterStartAddress.has_value()) {
        Logger::debug("Setting DEVICE_SPMCR_REGISTER AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_SPMCR_REGISTER,
            this->targetParameters.spmcRegisterStartAddress.value()
        );
    }

    if (this->targetParameters.osccalAddress.has_value()) {
        Logger::debug("Setting DEVICE_OSCCAL_ADDR AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_OSCCAL_ADDR,
            this->targetParameters.osccalAddress.value()
        );
    }

    if (this->targetParameters.eepromAddressRegisterLow.has_value()) {
        Logger::debug("Setting DEVICE_EEARL_ADDR AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_EEARL_ADDR,
            this->targetParameters.eepromAddressRegisterLow.value()
        );
    }

    if (this->targetParameters.eepromAddressRegisterHigh.has_value()) {
        Logger::debug("Setting DEVICE_EEARH_ADDR AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_EEARH_ADDR,
            this->targetParameters.eepromAddressRegisterHigh.value()
        );
    }

    if (this->targetParameters.eepromControlRegisterAddress.has_value()) {
        Logger::debug("Setting DEVICE_EECR_ADDR AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_EECR_ADDR,
            this->targetParameters.eepromControlRegisterAddress.value()
        );
    }

    if (this->targetParameters.eepromDataRegisterAddress.has_value()) {
        Logger::debug("Setting DEVICE_EEDR_ADDR AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_EEDR_ADDR,
            this->targetParameters.eepromDataRegisterAddress.value()
        );
    }

    if (this->targetParameters.bootSectionStartAddress.has_value()) {
        Logger::debug("Setting DEVICE_BOOT_START_ADDR AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_BOOT_START_ADDR,
            this->targetParameters.bootSectionStartAddress.value()
        );
    }
}

void EdbgAvr8Interface::setPdiParameters() {
    if (!this->targetParameters.appSectionPdiOffset.has_value()) {
        throw Exception("Missing required parameter: APPL_BASE_ADDR");
    }

    if (!this->targetParameters.bootSectionPdiOffset.has_value()) {
        throw Exception("Missing required parameter: BOOT_BASE_ADDR");
    }

    if (!this->targetParameters.bootSectionSize.has_value()) {
        throw Exception("Missing required parameter: BOOT_BYTES");
    }

    if (!this->targetParameters.flashSize.has_value()) {
        throw Exception("Missing required parameter: APPLICATION_BYTES");
    }

    if (!this->targetParameters.eepromPdiOffset.has_value()) {
        throw Exception("Missing required parameter: EEPROM_BASE_ADDR");
    }

    if (!this->targetParameters.fuseRegistersPdiOffset.has_value()) {
        throw Exception("Missing required parameter: FUSE_BASE_ADDR");
    }

    if (!this->targetParameters.lockRegistersPdiOffset.has_value()) {
        throw Exception("Missing required parameter: LOCKBIT_BASE_ADDR");
    }

    if (!this->targetParameters.userSignaturesPdiOffset.has_value()) {
        throw Exception("Missing required parameter: USER_SIGN_BASE_ADDR");
    }

    if (!this->targetParameters.productSignaturesPdiOffset.has_value()) {
        throw Exception("Missing required parameter: PROD_SIGN_BASE_ADDR");
    }

    if (!this->targetParameters.ramPdiOffset.has_value()) {
        throw Exception("Missing required parameter: DATA_BASE_ADDR");
    }

    if (!this->targetParameters.flashPageSize.has_value()) {
        throw Exception("Missing required parameter: FLASH_PAGE_BYTES");
    }

    if (!this->targetParameters.eepromSize.has_value()) {
        throw Exception("Missing required parameter: EEPROM_SIZE");
    }

    if (!this->targetParameters.eepromPageSize.has_value()) {
        throw Exception("Missing required parameter: EEPROM_PAGE_SIZE");
    }

    if (!this->targetParameters.nvmBaseAddress.has_value()) {
        throw Exception("Missing required parameter: NVM_BASE");
    }

    Logger::debug("Setting APPL_BASE_ADDR AVR8 parameter");
    this->setParameter(
        Avr8EdbgParameters::DEVICE_XMEGA_APPL_BASE_ADDR,
        this->targetParameters.appSectionPdiOffset.value()
    );

    Logger::debug("Setting BOOT_BASE_ADDR AVR8 parameter");
    this->setParameter(
        Avr8EdbgParameters::DEVICE_XMEGA_BOOT_BASE_ADDR,
        this->targetParameters.bootSectionPdiOffset.value()
    );

    Logger::debug("Setting EEPROM_BASE_ADDR AVR8 parameter");
    this->setParameter(
        Avr8EdbgParameters::DEVICE_XMEGA_EEPROM_BASE_ADDR,
        this->targetParameters.eepromPdiOffset.value()
    );

    Logger::debug("Setting FUSE_BASE_ADDR AVR8 parameter");
    this->setParameter(
        Avr8EdbgParameters::DEVICE_XMEGA_FUSE_BASE_ADDR,
        this->targetParameters.fuseRegistersPdiOffset.value()
    );

    Logger::debug("Setting LOCKBIT_BASE_ADDR AVR8 parameter");
    this->setParameter(
        Avr8EdbgParameters::DEVICE_XMEGA_LOCKBIT_BASE_ADDR,
        this->targetParameters.lockRegistersPdiOffset.value()
    );

    Logger::debug("Setting USER_SIGN_BASE_ADDR AVR8 parameter");
    this->setParameter(
        Avr8EdbgParameters::DEVICE_XMEGA_USER_SIGN_BASE_ADDR,
        this->targetParameters.userSignaturesPdiOffset.value()
    );

    Logger::debug("Setting PROD_SIGN_BASE_ADDR AVR8 parameter");
    this->setParameter(
        Avr8EdbgParameters::DEVICE_XMEGA_PROD_SIGN_BASE_ADDR,
        this->targetParameters.productSignaturesPdiOffset.value()
    );

    Logger::debug("Setting DATA_BASE_ADDR AVR8 parameter");
    this->setParameter(
        Avr8EdbgParameters::DEVICE_XMEGA_DATA_BASE_ADDR,
        this->targetParameters.ramPdiOffset.value()
    );

    Logger::debug("Setting APPLICATION_BYTES AVR8 parameter");
    this->setParameter(
        Avr8EdbgParameters::DEVICE_XMEGA_APPLICATION_BYTES,
        this->targetParameters.flashSize.value()
    );

    Logger::debug("Setting BOOT_BYTES AVR8 parameter");
    this->setParameter(
        Avr8EdbgParameters::DEVICE_XMEGA_BOOT_BYTES,
        this->targetParameters.bootSectionSize.value()
    );

    Logger::debug("Setting FLASH_PAGE_BYTES AVR8 parameter");
    this->setParameter(
        Avr8EdbgParameters::DEVICE_XMEGA_FLASH_PAGE_BYTES,
        this->targetParameters.flashPageSize.value()
    );

    Logger::debug("Setting EEPROM_SIZE AVR8 parameter");
    this->setParameter(
        Avr8EdbgParameters::DEVICE_XMEGA_EEPROM_SIZE,
        this->targetParameters.eepromSize.value()
    );

    Logger::debug("Setting EEPROM_PAGE_SIZE AVR8 parameter");
    this->setParameter(
        Avr8EdbgParameters::DEVICE_XMEGA_EEPROM_PAGE_SIZE,
        static_cast<unsigned char>(this->targetParameters.eepromPageSize.value())
    );

    Logger::debug("Setting NVM_BASE AVR8 parameter");
    this->setParameter(
        Avr8EdbgParameters::DEVICE_XMEGA_NVM_BASE,
        this->targetParameters.nvmBaseAddress.value()
    );
}

void EdbgAvr8Interface::setUpdiParameters() {
    if (!this->targetParameters.signatureSegmentStartAddress.has_value()) {
        throw Exception("Missing required parameter: SIGNATURE BASE ADDRESS");
    }

    if (this->targetParameters.programMemoryUpdiStartAddress.has_value()) {
        /*
         * The program memory base address field for UPDI sessions (DEVICE_UPDI_PROGMEM_BASE_ADDR) seems to be limited
         * to two bytes in size, as opposed to the four byte size for the debugWire, JTAG and PDI equivalent fields.
         * This is why, I suspect, another field was required for the most significant byte of the program memory base
         * address (DEVICE_UPDI_PROGMEM_BASE_ADDR_MSB).
         *
         * The additional DEVICE_UPDI_PROGMEM_BASE_ADDR_MSB field is only one byte in size, so it brings the total
         * capacity for the program memory base address to three bytes. Because of this, we ensure that all TDFs, for
         * targets that support UPDI, specify an address that does not exceed the maximum value of a 24 bit unsigned
         * integer. This is done in our TDF validation script (see src/Targets/TargetDescription/README.md for more).
         */
        auto programMemBaseAddress = this->targetParameters.programMemoryUpdiStartAddress.value();
        Logger::debug("Setting DEVICE_UPDI_PROGMEM_BASE_ADDR AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_UPDI_PROGMEM_BASE_ADDR,
            static_cast<std::uint16_t>(programMemBaseAddress)
        );

        Logger::debug("Setting DEVICE_UPDI_PROGMEM_BASE_ADDR_MSB AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_UPDI_PROGMEM_BASE_ADDR_MSB,
            static_cast<std::uint8_t>(programMemBaseAddress >> 16)
        );
    }

    if (this->targetParameters.flashPageSize.has_value()) {
        /*
         * See the comment above regarding capacity limitations of the DEVICE_UPDI_PROGMEM_BASE_ADDR field.
         *
         * The same applies here, for the flash page size field (DEVICE_UPDI_FLASH_PAGE_SIZE).
         */
        auto flashPageSize = this->targetParameters.flashPageSize.value();
        Logger::debug("Setting DEVICE_UPDI_FLASH_PAGE_SIZE AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_UPDI_FLASH_PAGE_SIZE,
            static_cast<std::uint8_t>(flashPageSize)
        );

        Logger::debug("Setting DEVICE_UPDI_FLASH_PAGE_SIZE_MSB AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_UPDI_FLASH_PAGE_SIZE_MSB,
            static_cast<std::uint8_t>(flashPageSize >> 8)
        );
    }

    if (this->targetParameters.eepromPageSize.has_value()) {
        Logger::debug("Setting DEVICE_UPDI_EEPROM_PAGE_SIZE AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_UPDI_EEPROM_PAGE_SIZE,
            this->targetParameters.eepromPageSize.value()
        );
    }

    if (this->targetParameters.nvmBaseAddress.has_value()) {
        Logger::debug("Setting DEVICE_UPDI_NVMCTRL_ADDR AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_UPDI_NVMCTRL_ADDR,
            this->targetParameters.nvmBaseAddress.value()
        );
    }

    if (this->targetParameters.ocdModuleAddress.has_value()) {
        Logger::debug("Setting DEVICE_UPDI_OCD_ADDR AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_UPDI_OCD_ADDR,
            this->targetParameters.ocdModuleAddress.value()
        );
    }

    if (this->targetParameters.flashSize.has_value()) {
        Logger::debug("Setting DEVICE_UPDI_FLASH_SIZE AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_UPDI_FLASH_SIZE,
            this->targetParameters.flashSize.value()
        );
    }

    if (this->targetParameters.eepromSize.has_value()) {
        Logger::debug("Setting DEVICE_UPDI_EEPROM_SIZE AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_UPDI_EEPROM_SIZE,
            this->targetParameters.eepromSize.value()
        );
    }

    if (this->targetParameters.eepromStartAddress.has_value()) {
        Logger::debug("Setting DEVICE_UPDI_EEPROM_BASE_ADDR AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_UPDI_EEPROM_BASE_ADDR,
            this->targetParameters.eepromStartAddress.value()
        );
    }

    if (this->targetParameters.signatureSegmentStartAddress.has_value()) {
        Logger::debug("Setting DEVICE_UPDI_SIG_BASE_ADDR AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_UPDI_SIG_BASE_ADDR,
            this->targetParameters.signatureSegmentStartAddress.value()
        );
    }

    if (this->targetParameters.fuseSegmentStartAddress.has_value()) {
        Logger::debug("Setting DEVICE_UPDI_FUSE_BASE_ADDR AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_UPDI_FUSE_BASE_ADDR,
            this->targetParameters.fuseSegmentStartAddress.value()
        );
    }

    if (this->targetParameters.fuseSegmentSize.has_value()) {
        Logger::debug("Setting DEVICE_UPDI_FUSE_SIZE AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_UPDI_FUSE_SIZE,
            this->targetParameters.fuseSegmentSize.value()
        );
    }

    if (this->targetParameters.lockbitsSegmentStartAddress.has_value()) {
        Logger::debug("Setting DEVICE_UPDI_LOCK_BASE_ADDR AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_UPDI_LOCK_BASE_ADDR,
            this->targetParameters.lockbitsSegmentStartAddress.value()
        );
    }

    this->setParameter(
        Avr8EdbgParameters::DEVICE_UPDI_24_BIT_ADDRESSING_ENABLE,
        this->targetParameters.programMemoryUpdiStartAddress.value_or(0) > 0xFFFF ?
            static_cast<std::uint8_t>(1) : static_cast<std::uint8_t>(0)
    );
}

void EdbgAvr8Interface::configure(const TargetConfig& targetConfig) {
    auto physicalInterface = targetConfig.jsonObject.find("physicalInterface")->toString().toLower().toStdString();

    auto availablePhysicalInterfaces = this->getPhysicalInterfacesByName();

    if (physicalInterface.empty()
        || availablePhysicalInterfaces.find(physicalInterface) == availablePhysicalInterfaces.end()
    ) {
        throw InvalidConfig("Invalid physical interface config parameter for AVR8 target.");
    }

    auto selectedPhysicalInterface = availablePhysicalInterfaces.find(physicalInterface)->second;

    if (selectedPhysicalInterface == PhysicalInterface::DEBUG_WIRE) {
        Logger::warning("AVR8 debugWire interface selected - the DWEN fuse will need to be enabled");
    }

    this->physicalInterface = selectedPhysicalInterface;

    if (this->physicalInterface == PhysicalInterface::JTAG && !this->family.has_value()) {
        throw InvalidConfig("Cannot use JTAG physical interface with ambiguous target name - please specify the"
            " exact name of the target in your configuration file. See https://bloom.oscillate.io/docs/supported-targets");
    }

    this->configVariant = this->resolveConfigVariant().value_or(Avr8ConfigVariant::NONE);
}

void EdbgAvr8Interface::setTargetParameters(const Avr8Bit::TargetParameters& config) {
    this->targetParameters = config;

    if (!config.stackPointerRegisterStartAddress.has_value()) {
        throw Exception("Failed to find stack pointer register start address");
    }

    if (!config.stackPointerRegisterSize.has_value()) {
        throw Exception("Failed to find stack pointer register size");
    }

    if (!config.statusRegisterStartAddress.has_value()) {
        throw Exception("Failed to find status register start address");
    }

    if (!config.statusRegisterSize.has_value()) {
        throw Exception("Failed to find status register size");
    }

    if (this->configVariant == Avr8ConfigVariant::NONE) {
        auto configVariant = this->resolveConfigVariant();

        if (!configVariant.has_value()) {
            throw Exception("Failed to resolve config variant for the selected "
                "physical interface and AVR8 family. The selected physical interface is not known to be supported "
                "by the AVR8 family.");
        }

        this->configVariant = configVariant.value();
    }

    switch (this->configVariant) {
        case Avr8ConfigVariant::DEBUG_WIRE:
        case Avr8ConfigVariant::MEGAJTAG: {
            this->setDebugWireAndJtagParameters();
            break;
        }
        case Avr8ConfigVariant::XMEGA: {
            this->setPdiParameters();
            break;
        }
        case Avr8ConfigVariant::UPDI: {
            this->setUpdiParameters();
            break;
        }
        default: {
            break;
        }
    }
}

void EdbgAvr8Interface::init() {
    if (this->configVariant == Avr8ConfigVariant::XMEGA) {
        // Default PDI clock to 4MHz
        // TODO: Make this adjustable via a target config parameter
        this->setParameter(Avr8EdbgParameters::PDI_CLOCK_SPEED, static_cast<std::uint16_t>(0x0FA0));
    }

    if (this->configVariant == Avr8ConfigVariant::UPDI) {
        // Default UPDI clock to 1.8MHz
        this->setParameter(Avr8EdbgParameters::PDI_CLOCK_SPEED, static_cast<std::uint16_t>(0x0708));
        this->setParameter(Avr8EdbgParameters::ENABLE_HIGH_VOLTAGE_UPDI, static_cast<unsigned char>(0x00));
    }

    if (this->configVariant == Avr8ConfigVariant::MEGAJTAG) {
        // Default clock value for mega debugging is 2KHz
        // TODO: Make this adjustable via a target config parameter
        this->setParameter(Avr8EdbgParameters::MEGA_DEBUG_CLOCK, static_cast<std::uint16_t>(0x00C8));
        this->setParameter(Avr8EdbgParameters::JTAG_DAISY_CHAIN_SETTINGS, static_cast<std::uint32_t>(0));
    }

    this->setParameter(
        Avr8EdbgParameters::CONFIG_VARIANT,
        static_cast<unsigned char>(this->configVariant)
    );

    this->setParameter(
        Avr8EdbgParameters::CONFIG_FUNCTION,
        static_cast<unsigned char>(this->configFunction)
    );

    this->setParameter(
        Avr8EdbgParameters::PHYSICAL_INTERFACE,
        getAvr8PhysicalInterfaceToIdMapping().at(this->physicalInterface)
    );
}

std::unique_ptr<AvrEvent> EdbgAvr8Interface::getAvrEvent() {
    auto event = this->edbgInterface.requestAvrEvent();

    if (!event.has_value()) {
        return nullptr;
    }

    switch (event->getEventId()) {
        case AvrEventId::AVR8_BREAK_EVENT: {
            // Break event
            return std::make_unique<BreakEvent>(event.value());
        }
        default: {
            /*
             * TODO: This isn't very nice as we're performing an unnecessary copy. Maybe requestAvrEvents should
             *       return a unique_ptr instead?
             */
            return std::make_unique<AvrEvent>(event.value());
        }
    }
}

void EdbgAvr8Interface::clearEvents() {
    while (this->getAvrEvent() != nullptr) {}
}

void EdbgAvr8Interface::activatePhysical(bool applyExternalReset) {
    auto commandFrame = CommandFrames::Avr8Generic::ActivatePhysical(applyExternalReset);

    auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
    if (response.getResponseId() == Avr8ResponseId::FAILED) {
        if (!applyExternalReset) {
            // Try again with external reset applied
            Logger::debug("Failed to activate physical interface on AVR8 target - retrying with external reset applied.");
            return this->activatePhysical(true);

        } else {
            throw Avr8CommandFailure("Activate physical interface command failed", response);
        }
    }

    this->physicalInterfaceActivated = true;
}

void EdbgAvr8Interface::deactivatePhysical() {
    auto commandFrame = CommandFrames::Avr8Generic::DeactivatePhysical();

    auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
    if (response.getResponseId() == Avr8ResponseId::FAILED) {
        throw Avr8CommandFailure("deactivate physical interface on AVR8 target command failed", response);
    }

    this->physicalInterfaceActivated = false;
}

void EdbgAvr8Interface::attach() {
    /*
     * When attaching an ATmega target that is connected via JTAG, we must not set the breakAfterAttach flag, as this
     * results in a timeout.
     *
     * However, in this case the attach command seems to _sometimes_ halt the target anyway, regardless of the
     * value of the breakAfterAttach flag. So we still expect a stop event to be received shortly after issuing
     * the attach command.
     */
    auto commandFrame = CommandFrames::Avr8Generic::Attach(
        this->configVariant != Avr8ConfigVariant::MEGAJTAG
    );

    auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
    if (response.getResponseId() == Avr8ResponseId::FAILED) {
        throw Avr8CommandFailure("Attach AVR8 target command failed", response);
    }

    this->targetAttached = true;

    try {
        // Wait for stopped event
        this->waitForStoppedEvent();

    } catch (const Exception& exception) {
        Logger::error("Execution on AVR8 target could not be halted post attach - " + exception.getMessage());
    }
}

void EdbgAvr8Interface::detach() {
    auto commandFrame = CommandFrames::Avr8Generic::Detach();

    auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
    if (response.getResponseId() == Avr8ResponseId::FAILED) {
        throw Avr8CommandFailure("Detach AVR8 target command failed", response);
    }

    this->targetAttached = false;
}

void EdbgAvr8Interface::activate() {
    if (!this->physicalInterfaceActivated) {
        this->activatePhysical();
    }

    if (!this->targetAttached) {
        this->attach();
    }
}

void EdbgAvr8Interface::deactivate() {
    if (this->targetAttached) {
        if (this->physicalInterface == PhysicalInterface::DEBUG_WIRE && this->disableDebugWireOnDeactivate) {
            try {
                this->disableDebugWire();
                Logger::warning("Successfully disabled debugWire on the AVR8 target - this is only temporary - "
                    "the debugWire module has lost control of the RESET pin. Bloom will no longer be able to interface "
                    "with the target until the next power cycle.");

            } catch (const Exception& exception) {
                // Failing to disable debugWire should never prevent us from proceeding with target deactivation.
                Logger::error(exception.getMessage());
            }
        }

        this->stop();
        this->clearAllBreakpoints();
        this->run();

        this->detach();
    }

    if (this->physicalInterfaceActivated) {
        this->deactivatePhysical();
    }
}

void EdbgAvr8Interface::reset() {
    auto commandFrame = CommandFrames::Avr8Generic::Reset();

    auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
    if (response.getResponseId() == Avr8ResponseId::FAILED) {
        throw Avr8CommandFailure("Reset AVR8 target command failed", response);
    }
}

void EdbgAvr8Interface::stop() {
    auto commandFrame = CommandFrames::Avr8Generic::Stop();

    auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
    if (response.getResponseId() == Avr8ResponseId::FAILED) {
        throw Avr8CommandFailure("Stop AVR8 target command failed", response);
    }

    if (this->getTargetState() == TargetState::RUNNING) {
        this->waitForStoppedEvent();
    }
}

void EdbgAvr8Interface::step() {
    auto commandFrame = CommandFrames::Avr8Generic::Step();

    auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
    if (response.getResponseId() == Avr8ResponseId::FAILED) {
        throw Avr8CommandFailure("Step AVR8 target command failed", response);
    }

    this->targetState = TargetState::RUNNING;
}

void EdbgAvr8Interface::waitForStoppedEvent() {
    auto breakEvent = this->waitForAvrEvent<BreakEvent>();

    if (breakEvent == nullptr) {
        throw Exception("Failed to receive break event for AVR8 target.");
    }

    this->targetState = TargetState::STOPPED;
}

void EdbgAvr8Interface::disableDebugWire() {
    auto commandFrame = CommandFrames::Avr8Generic::DisableDebugWire();

    auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
    if (response.getResponseId() == Avr8ResponseId::FAILED) {
        throw Avr8CommandFailure("Disable debugWire AVR8 target command failed", response);
    }
}

void EdbgAvr8Interface::run() {
    this->clearEvents();
    auto commandFrame = CommandFrames::Avr8Generic::Run();

    auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
    if (response.getResponseId() == Avr8ResponseId::FAILED) {
        throw Avr8CommandFailure("Run AVR8 command failed", response);
    }

    this->targetState = TargetState::RUNNING;
}

void EdbgAvr8Interface::runTo(std::uint32_t address) {
    this->clearEvents();
    Logger::debug("Running to address: " + std::to_string(address));
    auto commandFrame = CommandFrames::Avr8Generic::RunTo(address);

    auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
    if (response.getResponseId() == Avr8ResponseId::FAILED) {
        throw Avr8CommandFailure("Run-to AVR8 command failed", response);
    }

    this->targetState = TargetState::RUNNING;
}

std::uint32_t EdbgAvr8Interface::getProgramCounter() {
    if (this->targetState != TargetState::STOPPED) {
        this->stop();
    }

    auto commandFrame = CommandFrames::Avr8Generic::GetProgramCounter();
    auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
    if (response.getResponseId() == Avr8ResponseId::FAILED) {
        throw Avr8CommandFailure("Get AVR8 program counter command failed", response);
    }

    return response.extractProgramCounter();
}

TargetRegister EdbgAvr8Interface::getStackPointerRegister() {
    return TargetRegister(TargetRegisterDescriptor(TargetRegisterType::STACK_POINTER), this->readMemory(
        Avr8MemoryType::SRAM,
        this->targetParameters.stackPointerRegisterStartAddress.value(),
        this->targetParameters.stackPointerRegisterSize.value()
    ));
}

TargetRegister EdbgAvr8Interface::getStatusRegister() {
    return TargetRegister(TargetRegisterDescriptor(TargetRegisterType::STATUS_REGISTER), this->readMemory(
        Avr8MemoryType::SRAM,
        this->targetParameters.statusRegisterStartAddress.value(),
        this->targetParameters.statusRegisterSize.value()
    ));
}

void EdbgAvr8Interface::setStackPointerRegister(const TargetRegister& stackPointerRegister) {
    auto maximumStackPointerRegisterSize = this->targetParameters.stackPointerRegisterSize.value();
    auto registerValue = stackPointerRegister.value;

    if (registerValue.size() > maximumStackPointerRegisterSize) {
        throw Exception("Provided stack pointer register value exceeds maximum size.");

    } else if (registerValue.size() < maximumStackPointerRegisterSize) {
        // Fill the missing most-significant bytes with 0x00
        registerValue.insert(registerValue.begin(), maximumStackPointerRegisterSize - registerValue.size(), 0x00);
    }

    this->writeMemory(
        Avr8MemoryType::SRAM,
        this->targetParameters.stackPointerRegisterStartAddress.value(),
        registerValue
    );
}

void EdbgAvr8Interface::setStatusRegister(const TargetRegister& statusRegister) {
    auto maximumStatusRegisterSize = this->targetParameters.statusRegisterSize.value();
    auto registerValue = statusRegister.value;

    if (registerValue.size() > maximumStatusRegisterSize) {
        throw Exception("Provided status register value exceeds maximum size.");

    } else if (registerValue.size() < maximumStatusRegisterSize) {
        // Fill the missing most-significant bytes with 0x00
        registerValue.insert(registerValue.begin(), maximumStatusRegisterSize - registerValue.size(), 0x00);
    }

    this->writeMemory(
        Avr8MemoryType::SRAM,
        this->targetParameters.statusRegisterStartAddress.value(),
        registerValue
    );
}

void EdbgAvr8Interface::setProgramCounter(std::uint32_t programCounter) {
    if (this->targetState != TargetState::STOPPED) {
        this->stop();
    }

    /*
     * The program counter will be given in byte address form, but the EDBG tool will be expecting it in word
     * address (16-bit) form. This is why we divide it by 2.
     */
    auto commandFrame = CommandFrames::Avr8Generic::SetProgramCounter(programCounter / 2);
    auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
    if (response.getResponseId() == Avr8ResponseId::FAILED) {
        throw Avr8CommandFailure("Set program counter command failed", response);
    }
}

TargetSignature EdbgAvr8Interface::getDeviceId() {
    if (this->configVariant == Avr8ConfigVariant::UPDI) {
        /*
         * When using the UPDI physical interface, the 'Get device ID' command behaves in an odd manner, where it
         * doesn't actually return the target signature, but instead a fixed four byte string reading:
         * 'A', 'V', 'R' and ' ' (white space).
         *
         * So it appears we cannot use that command for UPDI sessions. As an alternative, we will just read the
         * signature from memory using the signature base address.
         *
         * TODO: Currently, we're assuming the signature will always only ever be three bytes in size, but we may
         *       want to consider pulling the size from the TDF.
         */
        auto signatureMemory = this->readMemory(
            Avr8MemoryType::SRAM,
            this->targetParameters.signatureSegmentStartAddress.value(),
            3
        );

        if (signatureMemory.size() != 3) {
            throw Exception("Failed to read AVR8 signature from target - unexpected response size");
        }

        return TargetSignature(signatureMemory[0], signatureMemory[1], signatureMemory[2]);
    }

    auto commandFrame = CommandFrames::Avr8Generic::GetDeviceId();

    auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
    if (response.getResponseId() == Avr8ResponseId::FAILED) {
        throw Avr8CommandFailure("Get device ID command failed", response);
    }

    return response.extractSignature(this->physicalInterface);
}

void EdbgAvr8Interface::setBreakpoint(std::uint32_t address) {
    auto commandFrame = CommandFrames::Avr8Generic::SetSoftwareBreakpoints({address});

    auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
    if (response.getResponseId() == Avr8ResponseId::FAILED) {
        throw Avr8CommandFailure("Set software breakpoint command failed", response);
    }
}

void EdbgAvr8Interface::clearBreakpoint(std::uint32_t address) {
    auto commandFrame = CommandFrames::Avr8Generic::ClearSoftwareBreakpoints({address});

    auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
    if (response.getResponseId() == Avr8ResponseId::FAILED) {
        throw Avr8CommandFailure("Clear AVR8 software breakpoint command failed", response);
    }
}

void EdbgAvr8Interface::clearAllBreakpoints() {
    auto commandFrame = CommandFrames::Avr8Generic::ClearAllSoftwareBreakpoints();

    auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
    if (response.getResponseId() == Avr8ResponseId::FAILED) {
        throw Avr8CommandFailure("Clear all AVR8 software breakpoints command failed", response);
    }
}

void EdbgAvr8Interface::refreshTargetState() {
    auto avrEvent = this->getAvrEvent();

    if (avrEvent != nullptr && avrEvent->getEventId() == AvrEventId::AVR8_BREAK_EVENT) {
        auto breakEvent = dynamic_cast<BreakEvent*>(avrEvent.get());

        if (breakEvent == nullptr) {
            throw Exception("Failed to process AVR8 break event");
        }

        this->targetState = TargetState::STOPPED;
        return;
    }

    this->targetState = TargetState::RUNNING;
}

TargetState EdbgAvr8Interface::getTargetState() {
    /*
     * We are not informed when a target goes from a stopped state to a running state, so there is no need
     * to query the tool when we already know the target has stopped.
     *
     * This means we have to rely on the assumption that the target cannot enter a running state without
     * our instruction.
     */
    if (this->targetState != TargetState::STOPPED) {
        this->refreshTargetState();
    }

    return this->targetState;
}

TargetMemoryBuffer EdbgAvr8Interface::readMemory(Avr8MemoryType type, std::uint32_t address, std::uint32_t bytes) {
    if (type == Avr8MemoryType::FLASH_PAGE) {
        if (this->targetParameters.flashPageSize.value_or(0) < 1) {
            throw Exception("Missing/invalid flash page size parameter");
        }

        // Flash reads must be done in pages
        auto pageSize = this->targetParameters.flashPageSize.value();

        if ((bytes % pageSize) != 0 || (address % pageSize) != 0) {
            /*
             * The number of bytes to read and/or the start address are not aligned.
             *
             * Align both and call this function again.
             */
            auto alignedAddress = address;
            auto alignedBytesToRead = bytes;

            if ((bytes % pageSize) != 0) {
                auto pagesRequired = static_cast<std::uint32_t>(std::ceil(
                    static_cast<float>(bytes) / static_cast<float>(pageSize)
                ));

                alignedBytesToRead = (pagesRequired * pageSize);
            }

            if ((address % pageSize) != 0) {
                alignedAddress = static_cast<std::uint32_t>(std::floor(
                    static_cast<float>(address) / static_cast<float>(pageSize)
                ) * pageSize);

                /*
                 * Given that we've pushed the start address back, this must be accounted for in the number of
                 * bytes to read. We'll need to include the difference as those are the bytes we're actually
                 * interested in.
                 *
                 * For example:
                 *
                 * Given: page size = 4
                 * Given: dummy memory (each character represents one byte, with first byte at 0x00) = aaaabbbbccccdddd
                 * Given: requested start address = 0x05
                 * Given: requested bytes to read = 4
                 *
                 * The start address (0x05) would be:
                 * aaaabbbbccccdddd
                 *      ^
                 * Because only 4 bytes were requested, starting at address 0x05, we're only interested in the bytes
                 * at addresses 0x05, 0x06, 0x07 and 0x08 (that's bytes bbbc).
                 *
                 * But the start address isn't aligned, so we need to align it by pushing it back to the beginning
                 * of the page (so we'd set it to 0x04, for this example), which is what we do above, when setting
                 * alignedAddress:
                 * aaaabbbbccccdddd
                 *     ^
                 * But now we'll only be reading 4 bytes from start address 0x04, meaning we won't be reading
                 * that 4th byte (0x08). So we need to account for this by adding the difference of the requested start
                 * address and the aligned start address to the number of bytes to read, to ensure that we're reading
                 * all of the bytes that we're interested in. But this will throw off the aligned bytes!! So we need
                 * to also account for this by aligning the additional bytes before adding them to alignedBytesToRead.
                 *
                 * However, we could simply get away with just adding the bytes without aligning
                 * them (alignedBytesToRead += (address - alignedAddress);), as the subsequent recursive call will
                 * align them for us, but it will result in an unnecessary recursion, so we'll just align the
                 * additional bytes here.
                 */
                if ((address - alignedAddress) > (alignedBytesToRead - bytes)) {
                    alignedBytesToRead += static_cast<std::uint32_t>(std::ceil(
                        static_cast<float>(address - alignedAddress) / static_cast<float>(pageSize)
                    )) * pageSize;
                }
            }

            /*
             * Now that the start address and bytes to read have been aligned, we can simply invoke this function
             * for a second time, with the aligned values. Then, return the requested data and discard the rest.
             */
            auto memoryBuffer = this->readMemory(type, alignedAddress, alignedBytesToRead);
            return TargetMemoryBuffer(
                memoryBuffer.begin() + (address - alignedAddress),
                memoryBuffer.begin() + (address - alignedAddress) + bytes
            );
        }

        // We can only read one flash page at a time.
        if (bytes > pageSize) {
            // bytes should always be a multiple of pageSize (given the code above)
            assert(bytes % pageSize == 0);
            int pagesRequired = static_cast<int>(bytes / pageSize);
            TargetMemoryBuffer memoryBuffer;

            for (auto i = 1; i <= pagesRequired; i++) {
                auto pageBuffer = this->readMemory(type, address + (pageSize * i), pageSize);
                memoryBuffer.insert(memoryBuffer.end(), pageBuffer.begin(), pageBuffer.end());
            }

            return memoryBuffer;
        }

    } else {
        /*
         * EDBG AVR8 debug tools behave in a really weird way when responding with more than two packets
         * for a single read (non-flash) memory command. The data they return in this case appears to be of little use.
         *
         * To address this, we make sure we only issue read memory commands that will result in no more than two
         * response packets. For calls that require more than this, we simply split them into numerous calls.
         */

        /*
         * The subtraction of 20 bytes here is just to account for any other bytes included in the response
         * that isn't actually the memory data (like the command ID, version bytes, etc). I could have sought the
         * actual value but who has the time. It won't exceed 20 bytes. Bite me.
         */
        auto singlePacketSize = static_cast<std::uint32_t>(this->edbgInterface.getUsbHidInputReportSize() - 20);
        auto totalResponsePackets = std::ceil(static_cast<float>(bytes) / static_cast<float>(singlePacketSize));
        auto totalReadsRequired = std::ceil(static_cast<float>(totalResponsePackets) / 2);

        if (totalResponsePackets > 2) {
            /*
             * This call to readMemory() will result in more than two response packets, so split it into multiple calls
             * that will result in no more than two response packets per call.
             */
            auto output = std::vector<unsigned char>();

            for (float i = 1; i <= totalReadsRequired; i++) {
                auto bytesToRead = static_cast<std::uint32_t>((bytes - output.size()) > (singlePacketSize * 2) ?
                                                              (singlePacketSize * 2) : bytes - output.size());
                auto data = this->readMemory(type, static_cast<std::uint32_t>(address + output.size()), bytesToRead);
                output.insert(output.end(), data.begin(), data.end());
            }

            return output;
        }
    }

    auto commandFrame = CommandFrames::Avr8Generic::ReadMemory();
    commandFrame.setType(type);
    commandFrame.setAddress(address);
    commandFrame.setBytes(bytes);

    auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
    if (response.getResponseId() == Avr8ResponseId::FAILED) {
        throw Avr8CommandFailure("Read memory AVR8 from target command failed", response);
    }

    return response.getMemoryBuffer();
}

void EdbgAvr8Interface::writeMemory(Avr8MemoryType type, std::uint32_t address, const TargetMemoryBuffer& buffer) {
    if (type == Avr8MemoryType::FLASH_PAGE) {
        // TODO: Implement support for writing to flash
        throw Exception("Cannot write to flash");
    }

    auto commandFrame = CommandFrames::Avr8Generic::WriteMemory();
    commandFrame.setType(type);
    commandFrame.setAddress(address);
    commandFrame.setBuffer(buffer);

    auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
    if (response.getResponseId() == Avr8ResponseId::FAILED) {
        throw Avr8CommandFailure("Write memory AVR8 from target command failed", response);
    }
}

TargetRegisters EdbgAvr8Interface::readGeneralPurposeRegisters(std::set<std::size_t> registerIds) {
    auto output = TargetRegisters();

    auto registers = this->readMemory(
        this->family == Family::XMEGA ? Avr8MemoryType::REGISTER_FILE : Avr8MemoryType::SRAM,
        this->targetParameters.gpRegisterStartAddress.value_or(0x00),
        this->targetParameters.gpRegisterSize.value_or(32)
    );

    for (std::size_t registerIndex = 0; registerIndex < registers.size(); registerIndex++) {
        if (!registerIds.empty() && registerIds.find(registerIndex) == registerIds.end()) {
            continue;
        }

        output.push_back(TargetRegister(registerIndex, {registers[registerIndex]}));
    }

    return output;
}

void EdbgAvr8Interface::writeGeneralPurposeRegisters(const TargetRegisters& registers) {
    auto gpRegisterSize = this->targetParameters.gpRegisterSize.value_or(32);
    auto gpStartAddress = this->targetParameters.gpRegisterStartAddress.value_or(0x00);

    for (const auto& gpRegister : registers) {
        auto descriptor = gpRegister.descriptor;
        if (gpRegister.descriptor.type != TargetRegisterType::GENERAL_PURPOSE_REGISTER) {
            // We are only to update GP registers here
            throw Exception("Cannot write non GP register");
        }

        if (!descriptor.id.has_value()) {
            throw Exception("Missing GP register ID");

        } else if ((descriptor.id.value() + 1) > gpRegisterSize || (descriptor.id.value() + 1) < 0) {
            throw Exception("Invalid GP register ID: " + std::to_string(descriptor.id.value()));
        }

        if (gpRegister.value.size() != 1) {
            throw Exception("Invalid GP register value size");
        }

        // TODO: This can be inefficient when updating many registers, maybe do something a little smarter here.
        this->writeMemory(
            this->configVariant == Avr8ConfigVariant::XMEGA ? Avr8MemoryType::REGISTER_FILE : Avr8MemoryType::SRAM,
            static_cast<std::uint32_t>(gpStartAddress + descriptor.id.value()),
            gpRegister.value
        );
    }
}

TargetMemoryBuffer EdbgAvr8Interface::readMemory(TargetMemoryType memoryType, std::uint32_t startAddress, std::uint32_t bytes) {
    auto avr8MemoryType = Avr8MemoryType::SRAM;

    switch (memoryType) {
        case TargetMemoryType::RAM: {
            avr8MemoryType = Avr8MemoryType::SRAM;
            break;
        }
        case TargetMemoryType::FLASH: {
            if (this->configVariant == Avr8ConfigVariant::DEBUG_WIRE) {
                avr8MemoryType = Avr8MemoryType::FLASH_PAGE;

            } else if (this->configVariant == Avr8ConfigVariant::XMEGA || this->configVariant == Avr8ConfigVariant::UPDI) {
                avr8MemoryType = Avr8MemoryType::APPL_FLASH;

            } else {
                avr8MemoryType = Avr8MemoryType::SPM;
            }
            break;
        }
        case TargetMemoryType::EEPROM: {
            avr8MemoryType = Avr8MemoryType::EEPROM;
        }
    }

    return this->readMemory(avr8MemoryType, startAddress, bytes);
}

void EdbgAvr8Interface::writeMemory(TargetMemoryType memoryType, std::uint32_t startAddress, const TargetMemoryBuffer& buffer) {
    auto avr8MemoryType = Avr8MemoryType::SRAM;

    switch (memoryType) {
        case TargetMemoryType::RAM: {
            avr8MemoryType = Avr8MemoryType::SRAM;
            break;
        }
        case TargetMemoryType::FLASH: {
            if (this->configVariant == Avr8ConfigVariant::DEBUG_WIRE) {
                avr8MemoryType = Avr8MemoryType::FLASH_PAGE;

            } else if (this->configVariant == Avr8ConfigVariant::MEGAJTAG) {
                avr8MemoryType = Avr8MemoryType::FLASH_PAGE;
                // TODO: Enable programming mode

            } else if (this->configVariant == Avr8ConfigVariant::XMEGA || this->configVariant == Avr8ConfigVariant::UPDI) {
                avr8MemoryType = Avr8MemoryType::APPL_FLASH;

            } else {
                avr8MemoryType = Avr8MemoryType::SPM;
            }
            break;
        }
        case TargetMemoryType::EEPROM: {
            avr8MemoryType = Avr8MemoryType::EEPROM;
        }
    }

    return this->writeMemory(avr8MemoryType, startAddress, buffer);
}
