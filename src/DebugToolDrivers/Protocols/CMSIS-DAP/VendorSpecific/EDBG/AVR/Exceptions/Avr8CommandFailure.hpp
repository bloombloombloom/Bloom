#pragma once

#include <optional>

#include "src/TargetController/Exceptions/TargetOperationFailure.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/ResponseFrames/AVR8Generic/Avr8GenericResponseFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr
{
    enum class Avr8CommandFailureCode: std::uint8_t
    {
        DEBUGWIRE_PHYSICAL_ERROR = 0x10,
        JTAGM_FAILED_TO_INITIALISE = 0x11,
        UNKNOWN_JTAG_ERROR = 0x12,
        JTAG_LOW_LEVEL_ERROR = 0x13,
        UNSUPPORTED_JTAGM_VERSION = 0x14,
        JTAG_MASTER_TIMED_OUT = 0x15,
        JTAG_BIT_BANGER_TIMED_OUT = 0x16,
        PARITY_ERROR_IN_RECEIVED_DATA = 0x17,
        MISSING_EMPTY_BYTE = 0x18,
        PDI_PHYSICAL_TIMED_OUT = 0x19,
        PHYSICAL_LEVEL_COLLISION = 0x1A,
        PDI_ENABLE_FAILED = 0x1B,
        TARGET_NOT_FOUND = 0x20,
        BAUD_INCREASE_FAILURE = 0x21,
        TARGET_POWER_NOT_DETECTED = 0x22,
        MUST_RUN_ATTACH_COMMAND_FIRST = 0x23,
        DEVICES_EXCEED_31 = 0x24,
        CONFIGURED_DEVICE_BITS_DO_NOT_ADD_UP_TO_DETECTED_BITS = 0x25,
        PHYSICAL_NOT_ACTIVATED = 0x31,
        ILLEGAL_RUN_OR_STOPPED_STATE = 0x32,
        INVALID_CONFIG_FOR_ACTIVATE_PHYSICAL = 0x33,
        NOT_A_VALID_MEMTYPE = 0x34,
        TOO_MANY_OR_TOO_FEW_BYTES = 0x35,
        ASKED_FOR_A_BAD_ADDRESS = 0x36,
        ASKED_FOR_BADLY_ALIGNED_DATA = 0x37,
        ADDRESS_NOT_WITHIN_LEGAL_RANGE = 0x38,
        ILLEGAL_VALUE_GIVEN = 0x39,
        ILLEGAL_TARGET_ID = 0x3A,
        CLOCK_VALUE_OUT_OF_RANGE = 0x3B,
        TIMEOUT_OCCURRED = 0x3C,
        READ_AN_ILLEGAL_OCD_STATUS = 0x3D,
        NVM_FAILED_TO_BE_ENABLED = 0x40,
        NVM_FAILED_TO_BE_DISABLED = 0x41,
        ILLEGAL_CONTROL_OR_STATUS_BITS = 0x42,
        CRC_MISMATCH = 0x43,
        FAILED_TO_ENABLE_OCD = 0x44,
        DEVICE_NOT_UNDER_CONTROL = 0x50,
        ERROR_WHEN_READING_PC = 0x60,
        ERROR_WHEN_READING_REGISTER = 0x61,
        ERROR_WHILE_READING = 0x70,
        ERROR_WHILE_WRITING = 0x71,
        TIMEOUT_WHILE_READING = 0x72,
        INVALID_BREAKPOINT_CONFIGURATION = 0x80,
        NOT_ENOUGH_AVAILABLE_RESOURCES = 0x81,
        FEATURE_NOT_AVAILABLE = 0x90,
        UNKNOWN_COMMAND = 0x91,
        UNKNOWN_ERROR = 0xFF,
    };

    class Avr8CommandFailure: public Bloom::Exceptions::TargetOperationFailure
    {
    public:
        std::optional<Avr8CommandFailureCode> code;

        explicit Avr8CommandFailure(const std::string& message): TargetOperationFailure(message) {
            this->message = message;
        }

        explicit Avr8CommandFailure(const char* message): TargetOperationFailure(message) {
            this->message = std::string(message);
        }

        explicit Avr8CommandFailure(
            const std::string& message,
            const ResponseFrames::Avr8Generic::Avr8GenericResponseFrame& responseFrame
        )
            : TargetOperationFailure(message)
        {
            this->message = message;

            if (responseFrame.payload.size() == 3) {
                /*
                 * The response includes a failure code - lookup the corresponding description and append it to the
                 * exception message.
                 */
                const auto failureCode = static_cast<Avr8CommandFailureCode>(responseFrame.payload[2]);
                const auto failureCodeDescriptionIt = this->failureCodeToDescription.find(failureCode);

                if (failureCodeDescriptionIt != this->failureCodeToDescription.end()) {
                    this->code = failureCode;
                    this->message += " - Failure reason: " + failureCodeDescriptionIt->second;
                }
            }
        }

    private:
        static const inline auto failureCodeToDescription = std::map<Avr8CommandFailureCode, std::string>({
            {Avr8CommandFailureCode::DEBUGWIRE_PHYSICAL_ERROR, "debugWIRE physical error"},
            {Avr8CommandFailureCode::JTAGM_FAILED_TO_INITIALISE, "JTAGM failed to initialise"},
            {Avr8CommandFailureCode::UNKNOWN_JTAG_ERROR, "JTAGM did something strange"},
            {Avr8CommandFailureCode::JTAG_LOW_LEVEL_ERROR, "JTAG low level error"},
            {Avr8CommandFailureCode::UNSUPPORTED_JTAGM_VERSION, "Unsupported version of JTAGM"},
            {Avr8CommandFailureCode::JTAG_MASTER_TIMED_OUT, "JTAG master timed out"},
            {Avr8CommandFailureCode::JTAG_BIT_BANGER_TIMED_OUT, "JTAG bit banger timed out"},
            {Avr8CommandFailureCode::PARITY_ERROR_IN_RECEIVED_DATA, "Parity error in received data"},
            {Avr8CommandFailureCode::MISSING_EMPTY_BYTE, "Did not receive EMPTY byte"},
            {Avr8CommandFailureCode::PDI_PHYSICAL_TIMED_OUT, "PDI physical timed out"},
            {Avr8CommandFailureCode::PHYSICAL_LEVEL_COLLISION, "Collision on physical level"},
            {Avr8CommandFailureCode::PDI_ENABLE_FAILED, "PDI enable failed"},
            {Avr8CommandFailureCode::TARGET_NOT_FOUND, "Target not found"},
            {Avr8CommandFailureCode::BAUD_INCREASE_FAILURE, "Failure when increasing baud"},
            {Avr8CommandFailureCode::TARGET_POWER_NOT_DETECTED, "Target power not detected"},
            {Avr8CommandFailureCode::MUST_RUN_ATTACH_COMMAND_FIRST, "Must run attach command first"},
            {Avr8CommandFailureCode::DEVICES_EXCEED_31, "Devices > 31"},
            {Avr8CommandFailureCode::CONFIGURED_DEVICE_BITS_DO_NOT_ADD_UP_TO_DETECTED_BITS, "Configured device bits do not add up to detected bits"},
            {Avr8CommandFailureCode::PHYSICAL_NOT_ACTIVATED, "Physical not activated"},
            {Avr8CommandFailureCode::ILLEGAL_RUN_OR_STOPPED_STATE, "Illegal run / stopped state"},
            {Avr8CommandFailureCode::INVALID_CONFIG_FOR_ACTIVATE_PHYSICAL, "Invalid config for activate physical"},
            {Avr8CommandFailureCode::NOT_A_VALID_MEMTYPE, "Not a valid memtype"},
            {Avr8CommandFailureCode::TOO_MANY_OR_TOO_FEW_BYTES, "Too many or too few bytes"},
            {Avr8CommandFailureCode::ASKED_FOR_A_BAD_ADDRESS, "Asked for a bad address"},
            {Avr8CommandFailureCode::ASKED_FOR_BADLY_ALIGNED_DATA, "Asked for badly aligned data"},
            {Avr8CommandFailureCode::ADDRESS_NOT_WITHIN_LEGAL_RANGE, "Address not within legal range"},
            {Avr8CommandFailureCode::ILLEGAL_VALUE_GIVEN, "Illegal value given"},
            {Avr8CommandFailureCode::ILLEGAL_TARGET_ID, "Illegal target ID"},
            {Avr8CommandFailureCode::CLOCK_VALUE_OUT_OF_RANGE, "Clock value out of range"},
            {Avr8CommandFailureCode::TIMEOUT_OCCURRED, "A timeout occurred"},
            {Avr8CommandFailureCode::READ_AN_ILLEGAL_OCD_STATUS, "Read an illegal OCD status - check OCDEN fuse bit (if applicable)"},
            {Avr8CommandFailureCode::NVM_FAILED_TO_BE_ENABLED, "NVM failed to be enabled"},
            {Avr8CommandFailureCode::NVM_FAILED_TO_BE_DISABLED, "NVM failed to be disabled"},
            {Avr8CommandFailureCode::ILLEGAL_CONTROL_OR_STATUS_BITS, "Illegal control/status bits"},
            {Avr8CommandFailureCode::CRC_MISMATCH, "CRC mismatch"},
            {Avr8CommandFailureCode::FAILED_TO_ENABLE_OCD, "Failed to enable OCD"},
            {Avr8CommandFailureCode::DEVICE_NOT_UNDER_CONTROL, "Device not under control - check OCDEN fuse bit (if applicable)"},
            {Avr8CommandFailureCode::ERROR_WHEN_READING_PC, "Error when reading PC"},
            {Avr8CommandFailureCode::ERROR_WHEN_READING_REGISTER, "Error when reading register"},
            {Avr8CommandFailureCode::ERROR_WHILE_READING, "Error while reading"},
            {Avr8CommandFailureCode::ERROR_WHILE_WRITING, "Error while writing"},
            {Avr8CommandFailureCode::TIMEOUT_WHILE_READING, "Timeout while reading"},
            {Avr8CommandFailureCode::INVALID_BREAKPOINT_CONFIGURATION, "Invalid breakpoint configuration"},
            {Avr8CommandFailureCode::NOT_ENOUGH_AVAILABLE_RESOURCES, "Not enough available resources"},
            {Avr8CommandFailureCode::FEATURE_NOT_AVAILABLE, "Feature not available"},
            {Avr8CommandFailureCode::UNKNOWN_COMMAND, "Command has not been implemented"},
            {Avr8CommandFailureCode::UNKNOWN_ERROR, "Unknown error reported by EDBG device"},
        });
    };
}
