#pragma once

#include <map>

#include "src/Targets/TargetPhysicalInterface.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr
{
    struct Avr8EdbgParameter
    {
        unsigned char context;
        unsigned char id;

        constexpr Avr8EdbgParameter(unsigned char context, unsigned char id)
            : context(context)
            , id(id)
        {};
    };

    struct Avr8EdbgParameters
    {
        static constexpr Avr8EdbgParameter CONFIG_VARIANT = {0x00, 0x00};
        static constexpr Avr8EdbgParameter CONFIG_FUNCTION = {0x00, 0x01};
        static constexpr Avr8EdbgParameter PHYSICAL_INTERFACE = {0x01, 0x00};
        static constexpr Avr8EdbgParameter DW_CLOCK_DIVISION_FACTOR = {0x01, 0x10};
        static constexpr Avr8EdbgParameter PDI_CLOCK_SPEED = {0x01, 0x31};
        static constexpr Avr8EdbgParameter MEGA_DEBUG_CLOCK = {0x01, 0x21};
        static constexpr Avr8EdbgParameter JTAG_DAISY_CHAIN_SETTINGS = {0x01, 0x01};

        // debugWIRE and JTAG parameters
        static constexpr Avr8EdbgParameter DEVICE_BOOT_START_ADDR = {0x02, 0x0A};
        static constexpr Avr8EdbgParameter DEVICE_FLASH_BASE = {0x02, 0x06};
        static constexpr Avr8EdbgParameter DEVICE_SRAM_START = {0x02, 0x0E};
        static constexpr Avr8EdbgParameter DEVICE_EEPROM_SIZE = {0x02, 0x10};
        static constexpr Avr8EdbgParameter DEVICE_EEPROM_PAGE_SIZE = {0x02, 0x12};
        static constexpr Avr8EdbgParameter DEVICE_FLASH_PAGE_SIZE = {0x02, 0x00};
        static constexpr Avr8EdbgParameter DEVICE_FLASH_SIZE = {0x02, 0x02};
        static constexpr Avr8EdbgParameter DEVICE_OCD_REVISION = {0x02, 0x13};
        static constexpr Avr8EdbgParameter DEVICE_PAGE_BUFFERS_PER_FLASH_BLOCK = {0x02, 0x14};
        static constexpr Avr8EdbgParameter DEVICE_OCD_DATA_REGISTER = {0x02, 0x18};
        static constexpr Avr8EdbgParameter DEVICE_SPMCR_REGISTER = {0x02, 0x1D};
        static constexpr Avr8EdbgParameter DEVICE_OSCCAL_ADDR = {0x02, 0x1E};
        static constexpr Avr8EdbgParameter DEVICE_EEARH_ADDR = {0x02, 0x19};
        static constexpr Avr8EdbgParameter DEVICE_EEARL_ADDR = {0x02, 0x1A};
        static constexpr Avr8EdbgParameter DEVICE_EECR_ADDR = {0x02, 0x1B};
        static constexpr Avr8EdbgParameter DEVICE_EEDR_ADDR = {0x02, 0x1C};

        // PDI/XMega device parameters
        static constexpr Avr8EdbgParameter DEVICE_XMEGA_APPL_BASE_ADDR = {0x02, 0x00};
        static constexpr Avr8EdbgParameter DEVICE_XMEGA_BOOT_BASE_ADDR = {0x02, 0x04};
        static constexpr Avr8EdbgParameter DEVICE_XMEGA_EEPROM_BASE_ADDR = {0x02, 0x08};
        static constexpr Avr8EdbgParameter DEVICE_XMEGA_FUSE_BASE_ADDR = {0x02, 0x0C};
        static constexpr Avr8EdbgParameter DEVICE_XMEGA_LOCKBIT_BASE_ADDR = {0x02, 0x10};
        static constexpr Avr8EdbgParameter DEVICE_XMEGA_USER_SIGN_BASE_ADDR = {0x02, 0x14};
        static constexpr Avr8EdbgParameter DEVICE_XMEGA_PROD_SIGN_BASE_ADDR = {0x02, 0x18};
        static constexpr Avr8EdbgParameter DEVICE_XMEGA_DATA_BASE_ADDR = {0x02, 0x1C};
        static constexpr Avr8EdbgParameter DEVICE_XMEGA_APPLICATION_BYTES = {0x02, 0x20};
        static constexpr Avr8EdbgParameter DEVICE_XMEGA_BOOT_BYTES = {0x02, 0x24};
        static constexpr Avr8EdbgParameter DEVICE_XMEGA_NVM_BASE = {0x02, 0x2B};
        static constexpr Avr8EdbgParameter DEVICE_XMEGA_SIGNATURE_OFFSET = {0x02, 0x2D};
        static constexpr Avr8EdbgParameter DEVICE_XMEGA_FLASH_PAGE_BYTES = {0x02, 0x26};
        static constexpr Avr8EdbgParameter DEVICE_XMEGA_EEPROM_SIZE = {0x02, 0x28};
        static constexpr Avr8EdbgParameter DEVICE_XMEGA_EEPROM_PAGE_SIZE = {0x02, 0x2A};

        // UPDI device parameters
        static constexpr Avr8EdbgParameter DEVICE_UPDI_PROGMEM_BASE_ADDR = {0x02, 0x00};
        static constexpr Avr8EdbgParameter DEVICE_UPDI_FLASH_PAGE_SIZE = {0x02, 0x02};
        static constexpr Avr8EdbgParameter DEVICE_UPDI_EEPROM_PAGE_SIZE = {0x02, 0x03};
        static constexpr Avr8EdbgParameter DEVICE_UPDI_NVMCTRL_ADDR = {0x02, 0x04};
        static constexpr Avr8EdbgParameter DEVICE_UPDI_OCD_ADDR = {0x02, 0x06};
        static constexpr Avr8EdbgParameter DEVICE_UPDI_FLASH_SIZE = {0x02, 0x12};
        static constexpr Avr8EdbgParameter DEVICE_UPDI_EEPROM_SIZE = {0x02, 0x16};
        static constexpr Avr8EdbgParameter DEVICE_UPDI_USER_SIG_SIZE = {0x02, 0x18};
        static constexpr Avr8EdbgParameter DEVICE_UPDI_FUSE_SIZE = {0x02, 0x1A};
        static constexpr Avr8EdbgParameter DEVICE_UPDI_EEPROM_BASE_ADDR = {0x02, 0x20};
        static constexpr Avr8EdbgParameter DEVICE_UPDI_USER_SIG_BASE_ADDR = {0x02, 0x22};
        static constexpr Avr8EdbgParameter DEVICE_UPDI_SIG_BASE_ADDR = {0x02, 0x24};
        static constexpr Avr8EdbgParameter DEVICE_UPDI_FUSE_BASE_ADDR = {0x02, 0x26};
        static constexpr Avr8EdbgParameter DEVICE_UPDI_LOCK_BASE_ADDR = {0x02, 0x28};
        static constexpr Avr8EdbgParameter DEVICE_UPDI_DEVICE_ID = {0x02, 0x2A};
        static constexpr Avr8EdbgParameter DEVICE_UPDI_PROGMEM_BASE_ADDR_MSB = {0x02, 0x2C};
        static constexpr Avr8EdbgParameter DEVICE_UPDI_FLASH_PAGE_SIZE_MSB = {0x02, 0x2D};
        static constexpr Avr8EdbgParameter DEVICE_UPDI_24_BIT_ADDRESSING_ENABLE = {0x02, 0x2E};

        static constexpr Avr8EdbgParameter RUN_TIMERS_WHILST_STOPPED = {0x03, 0x00};
        static constexpr Avr8EdbgParameter ENABLE_HIGH_VOLTAGE_UPDI = {0x03, 0x06};
    };

    enum class Avr8ConfigVariant: unsigned char
    {
        LOOPBACK = 0x00,
        NONE = 0xff,
        DEBUG_WIRE = 0x01,
        MEGAJTAG = 0x02,
        XMEGA = 0x03,
        UPDI = 0x05,
    };

    enum class Avr8ConfigFunction: unsigned char
    {
        NONE = 0x00,
        PROGRAMMING = 0x01,
        DEBUGGING = 0x02,
    };

    enum class Avr8PhysicalInterface: unsigned char
    {
        NONE = 0x00,
        JTAG = 0x04,
        DEBUG_WIRE = 0x05,
        PDI = 0x06,
        PDI_1W = 0x08,
    };

    enum class Avr8MemoryType: unsigned char
    {
        /**
         * The SRAM memory type can be used to read & write to internal memory on the target.
         *
         * It's available with all of the config variants in debugging mode.
         */
        SRAM = 0x20,

        /**
         * The EEPROM memory type can be used to read and write to EEPROM memory on the target.
         *
         * It's available with all of the config variants, in debugging mode.
         */
        EEPROM = 0x22,

        /**
         * The EEPROM_ATOMIC memory type can be used to write to EEPROM memory with automatic page erasing.
         *
         * It's only available for XMEGA and UPDI config variants.
         *
         * Only one EEPROM page can be written at a time.
         */
        EEPROM_ATOMIC = 0xC4,

        /**
         * The EEPROM_PAGE memory type can be used to read and write to EEPROM whilst the target is in
         * programming mode.
         */
        EEPROM_PAGE = 0xB1,

        /**
         * The FLASH_PAGE memory type can be used to read and write full flash pages on the target.
         *
         * Only available with the JTAG and debugWIRE config variants.
         *
         * This memory type is not available with the JTAG config variant in debugging mode. Programming mode will need
         * to be enabled before it can be used with JTAG targets. With the debugWIRE variant, this memory type *can* be
         * used whilst in debugging mode.
         */
        FLASH_PAGE = 0xB0,

        /**
         * The APPL_FLASH memory type can be used to read/write to the application section of the flash memory on the
         * target.
         *
         * Only available with the XMEGA and UPDI config variants.
         *
         * When in debugging mode, only read access is permitted. Programming mode will need to be enabled before
         * any attempts of writing data.
         */
        APPL_FLASH = 0xC0,
        BOOT_FLASH = 0xC1,

        APPL_FLASH_ATOMIC = 0xC2,

        /**
         * The SPM memory type can be used to read memory from the target whilst in debugging mode.
         *
         * Only available with JTAG and debugWIRE config variants.
         */
        SPM = 0xA0,

        /**
         * The REGISTER_FILE memory type can be used to read & write to general purpose registers.
         *
         * Only available in debugging mode and with XMEGA and UPDI config variants. The SRAM memory type can be used
         * to access general purpose registers when other variants are in use.
         */
        REGISTER_FILE = 0xB8,

        /**
         * The FUSES memory type can be used to read and write AVR fuses in programming mode.
         *
         * Not available for the debugWIRE config variant.
         */
        FUSES = 0xB2,
    };

    enum class Avr8ResponseId: unsigned char
    {
        OK = 0x80,
        DATA = 0x84,
        FAILED = 0xA0,
    };

    inline bool operator == (unsigned char rawId, Avr8ResponseId id) {
        return static_cast<unsigned char>(id) == rawId;
    }

    inline bool operator == (Avr8ResponseId id, unsigned char rawId) {
        return rawId == id;
    }

    enum class Avr8EraseMemoryMode: unsigned char
    {
        CHIP = 0x00,
        APPLICATION_SECTION = 0x01,
        BOOT_SECTION = 0x02,
        EEPROM = 0x03,
    };
}
