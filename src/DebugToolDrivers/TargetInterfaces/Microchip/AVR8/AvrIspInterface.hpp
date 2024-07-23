#pragma once

#include <cstdint>

#include "src/Targets/Microchip/AVR8/TargetSignature.hpp"
#include "src/Targets/Microchip/AVR8/IspParameters.hpp"
#include "src/Targets/Microchip/AVR8/Fuse.hpp"

#include "src/ProjectConfig.hpp"

namespace DebugToolDrivers::TargetInterfaces::Microchip::Avr8
{
    /**
     * Many AVRs can be programmed via an SPI interface. Some debug tools provide access to this interface via the AVR
     * In-System Programming (ISP) protocol.
     *
     * This interface class is incomplete - it only provides the ability to read the device ID and access AVR fuses and
     * lockbit bytes (as that's all we need, for now).
     *
     * Currently, Bloom only uses the ISP interface for accessing fuses and lockbits on debugWIRE targets. We can't
     * access fuses via the debugWIRE interface, so we have to use the ISP interface.
     *
     * @see Avr8::updateDwenFuseBit() for more.
     */
    class AvrIspInterface
    {
    public:
        AvrIspInterface() = default;
        virtual ~AvrIspInterface() = default;

        AvrIspInterface(const AvrIspInterface& other) = default;
        AvrIspInterface(AvrIspInterface&& other) = default;

        AvrIspInterface& operator = (const AvrIspInterface& other) = default;
        AvrIspInterface& operator = (AvrIspInterface&& other) = default;

        /**
         * Should initialise and activate the ISP interface between the debug tool and the AVR target.
         */
        virtual void activate() = 0;

        /**
         * Should deactivate the ISP interface between the debug tool and the AVR target.
         */
        virtual void deactivate() = 0;

        /**
         * Should retrieve the AVR signature of the AVR target.
         *
         * @return
         */
        virtual Targets::Microchip::Avr8::TargetSignature getDeviceId() = 0;

        /**
         * Should read a fuse from the AVR target.
         *
         * @param fuseRegisterDescriptor
         * @return
         */
        virtual Targets::Microchip::Avr8::FuseValue readFuse(
            const Targets::TargetRegisterDescriptor& fuseRegisterDescriptor
        ) = 0;

        /**
         * Should read the lock bit byte from the AVR target.
         *
         * @return
         */
        virtual unsigned char readLockBitByte() = 0;

        /**
         * Should program a particular fuse byte.
         *
         * @param fuseRegisterDescriptor
         * @param value
         */
        virtual void programFuse(
            const Targets::TargetRegisterDescriptor& fuseRegisterDescriptor,
            Targets::Microchip::Avr8::FuseValue value
        ) = 0;
    };
}
