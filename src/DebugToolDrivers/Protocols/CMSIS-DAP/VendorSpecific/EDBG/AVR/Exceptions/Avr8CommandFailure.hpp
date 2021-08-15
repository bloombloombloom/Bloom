#pragma once

#include "src/TargetController/Exceptions/TargetOperationFailure.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/ResponseFrames/AVR8Generic/Avr8GenericResponseFrame.hpp"

namespace Bloom::Exceptions
{
    class Avr8CommandFailure: public TargetOperationFailure
    {
    private:
        static inline auto failureCodeToDescription = std::map<unsigned char, std::string>({
            {0x10, "debugWIRE physical error"},
            {0x11, "JTAGM failed to initialise"},
            {0x12, "JTAGM did something strange"},
            {0x13, "JTAG low level error"},
            {0x14, "Unsupported version of JTAGM"},
            {0x15, "JTAG master timed out"},
            {0x16, "JTAG bit banger timed out"},
            {0x17, "Parity error in received data"},
            {0x18, "Did not receive EMPTY byte"},
            {0x19, "PDI physical timed out"},
            {0x1A, "Collision on physical level"},
            {0x1B, "PDI enable failed"},
            {0x20, "Target not found"},
            {0x21, "Failure when increasing baud"},
            {0x22, "Target power not detected"},
            {0x23, "Must run attach command first"},
            {0x24, "Devices > 31"},
            {0x25, "Configured device bits do not add up to detected bits"},
            {0x31, "Physical not activated"},
            {0x32, "Illegal run / stopped state"},
            {0x33, "Invalid config for activate phy"},
            {0x34, "Not a valid memtype"},
            {0x35, "Too many or too few bytes"},
            {0x36, "Asked for a bad address"},
            {0x37, "Asked for badly aligned data"},
            {0x38, "Address not within legal range"},
            {0x39, "Illegal value given"},
            {0x3A, "Illegal target ID"},
            {0x3B, "Clock value out of range"},
            {0x3C, "A timeout occurred"},
            {0x3D, "Read an illegal OCD status"},
            {0x40, "NVM failed to be enabled"},
            {0x41, "NVM failed to be disabled"},
            {0x42, "Illegal control/status bits"},
            {0x43, "CRC mismatch"},
            {0x44, "Failed to enable OCD"},
            {0x50, "Device not under control"},
            {0x60, "Error when reading PC"},
            {0x61, "Error when reading register"},
            {0x70, "Error while reading"},
            {0x71, "Error while writing"},
            {0x72, "Timeout while reading"},
            {0x80, "Invalid breakpoint configuration"},
            {0x81, "Not enough available resources"},
            {0x90, "Feature not available"},
            {0x91, "Command has not been implemented"},
            {0xFF, "Unknown error"},
        });

    public:
        explicit Avr8CommandFailure(const std::string& message): TargetOperationFailure(message) {
            this->message = message;
        }

        explicit Avr8CommandFailure(const char* message): TargetOperationFailure(message) {
            this->message = std::string(message);
        }

        explicit Avr8CommandFailure(
            const std::string& message,
            DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::ResponseFrames::Avr8Generic::Avr8GenericResponseFrame& responseFrame
        ): TargetOperationFailure(message) {
            this->message = message;

            auto responsePayload = responseFrame.getPayload();
            if (responsePayload.size() == 3 && this->failureCodeToDescription.contains(responsePayload[2])) {
                this->message += " - Failure reason: " + this->failureCodeToDescription.find(responsePayload[2])->second;
            }
        }
    };
}
