#pragma once

#include "TargetInterfaces/Microchip/AVR/AVR8/Avr8Interface.hpp"

namespace Bloom
{
    using DebugToolDrivers::TargetInterfaces::Microchip::Avr::Avr8::Avr8Interface;

    /**
     * A debug tool can be any device that provides access to the connected target. Debug tools are usually connected
     * to the host machine via USB.
     *
     * Each debug tool must implement this interface. Note that target specific driver code should not be placed here.
     * Each target family will expect the debug tool to provide an interface for that particular group of targets.
     * For an example, see the Avr8Interface class and the DebugTool::getAvr8Interface().
     */
    class DebugTool
    {
    private:
        bool initialised = false;

    protected:
        void setInitialised(bool initialised) {
            this->initialised = initialised;
        }

    public:
        bool isInitialised() const {
            return this->initialised;
        }

        /**
         * Should establish a connection to the device and prepare it for a debug session.
         */
        virtual void init() = 0;

        /**
         * Should disconnect from the device after performing any tasks required to formally end the debug session.
         */
        virtual void close() = 0;

        virtual std::string getName() = 0;

        virtual std::string getSerialNumber() = 0;

        /**
         * All debug tools that support AVR8 targets must provide an implementation of the Avr8Interface
         * class, via this method.
         *
         * For debug tools that do not support AVR8 targets, this method should return a nullptr.
         *
         * Note: the caller of this method will not manage the lifetime of the returned Avr8Interface instance.
         *
         * @return
         */
        virtual Avr8Interface* getAvr8Interface() {
            return nullptr;
        };

        virtual ~DebugTool() = default;
    };
}
