#pragma once

#include <cstdint>

#include "src/Targets/Microchip/AVR/TargetSignature.hpp"
#include "src/Targets/Microchip/AVR/IspParameters.hpp"
#include "src/Targets/Microchip/AVR/Fuse.hpp"

#include "src/ProjectConfig.hpp"

namespace DebugToolDrivers::TargetInterfaces::Microchip::Avr
{
    /**
     * Many AVRs can be programmed via an SPI interface. Some debug tools provide access to this interface via the AVR
     * In-System Programming (ISP) protocol.
     *
     * This interface class is incomplete - it only provides the ability to read the device ID and access AVR fuses and
     * lockbit bytes (as that's all we need, for now).
     *
     * Currently, Bloom only uses the ISP interface for accessing fuses and lockbits on debugWire targets. We can't
     * access fuses via the debugWire interface, so we have to use the ISP interface.
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
         * Configures the ISP interface with user-provided config parameters.
         *
         * @param targetConfig
         */
        virtual void configure(const TargetConfig& targetConfig) = 0;

        /**
         * Configures the ISP interface with the target's ISP parameters.
         *
         * @param ispParameters
         */
        virtual void setIspParameters(const Targets::Microchip::Avr::IspParameters& ispParameters) = 0;

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
        virtual Targets::Microchip::Avr::TargetSignature getDeviceId() = 0;

        /**
         * Should read a fuse from the AVR target.
         *
         * @param fuseType
         * @return
         */
        virtual Targets::Microchip::Avr::Fuse readFuse(Targets::Microchip::Avr::FuseType fuseType) = 0;

        /**
         * Should read the lock bit byte from the AVR target.
         *
         * @return
         */
        virtual unsigned char readLockBitByte() = 0;

        /**
         * Should program a particular fuse byte.
         *
         * @param fuse
         */
        virtual void programFuse(Targets::Microchip::Avr::Fuse fuse) = 0;
    };
}
