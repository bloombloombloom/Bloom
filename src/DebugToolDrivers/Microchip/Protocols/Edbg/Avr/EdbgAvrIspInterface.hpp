#pragma once

#include <cstdint>
#include <chrono>
#include <thread>
#include <cassert>

#include "src/DebugToolDrivers/TargetInterfaces/Microchip/Avr8/AvrIspInterface.hpp"
#include "src/DebugToolDrivers/Microchip/Protocols/Edbg/EdbgInterface.hpp"
#include "src/Targets/Microchip/Avr8/TargetDescriptionFile.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr
{
    /**
     * The EdbgAvrIspInterface implements the AVRISP EDBG/CMSIS-DAP protocol, as an AvrIspInterface.
     *
     * See the "AVR ISP Protocol" section in the DS50002630A document by Microchip, for more information on the
     * protocol.
     *
     * This implementation should work with any Microchip EDBG-based CMSIS-DAP debug tool with ISP support (such as
     * the Atmel-ICE, Power Debugger, the MPLAB SNAP debugger (in "AVR mode"), etc).
     */
    class EdbgAvrIspInterface: public TargetInterfaces::Microchip::Avr8::AvrIspInterface
    {
    public:
        explicit EdbgAvrIspInterface(
            EdbgInterface* edbgInterface,
            const Targets::Microchip::Avr8::TargetDescriptionFile& targetDescriptionFile
        );

        /**
         * Initialises the ISP interface by enabling "programming mode" on the debug tool. This will activate the
         * physical (SPI) interface between the debug tool and AVR target.
         */
        void activate() override;

        /**
         * Disables "programming mode" on the debug tool, which subsequently deactivates the SPI interface between the
         * debug tool and AVR target.
         */
        void deactivate() override;

        /**
         * Obtains the AVR signature from the connected AVR.
         *
         * @return
         */
        Targets::Microchip::Avr8::TargetSignature getDeviceId() override;

        /**
         * Reads a particular fuse byte from the AVR target.
         *
         * @param fuseRegisterDescriptor
         * @return
         */
        Targets::Microchip::Avr8::FuseValue readFuse(
            const Targets::TargetRegisterDescriptor& fuseRegisterDescriptor
        ) override;

        /**
         * Reads the lock bit byte from the AVR target.
         *
         * @return
         */
        unsigned char readLockBitByte() override;

        /**
         * Programs a particular fuse on the AVR target.
         *
         * @param fuseRegisterDescriptor
         * @param value
         */
        void programFuse(
            const Targets::TargetRegisterDescriptor& fuseRegisterDescriptor,
            Targets::Microchip::Avr8::FuseValue value
        ) override;

    private:
        /**
         * The AVRISP protocol is a sub-protocol of the EDBG AVR protocol, which is served via CMSIS-DAP vendor
         * commands.
         *
         * Every EDBG based debug tool that utilises this implementation must provide access to its EDBG interface.
         */
        EdbgInterface* edbgInterface;

        Targets::Microchip::Avr8::IspParameters ispParameters;

        /**
         * The EDBG AVRISP protocol only allows us to read a single signature byte at a time.
         * This function will read a single signature byte. See implementation of EdbgAvrIspInterface::getDeviceId()
         * for more.
         *
         * @param signatureByteAddress
         * @return
         */
        [[nodiscard]] unsigned char readSignatureByte(std::uint8_t signatureByteAddress);

        Targets::Microchip::Avr8::FuseType resolveFuseType(
            const Targets::TargetRegisterDescriptor& fuseRegisterDescriptor
        );
    };
}
