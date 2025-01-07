#include "EdbgAvrIspInterface.hpp"

#include "src/TargetController/Exceptions/TargetOperationFailure.hpp"
#include "src/Exceptions/InternalFatalErrorException.hpp"
#include "src/Logger/Logger.hpp"

// Command frames
#include "CommandFrames/AvrIsp/EnterProgrammingMode.hpp"
#include "CommandFrames/AvrIsp/LeaveProgrammingMode.hpp"
#include "CommandFrames/AvrIsp/ReadSignature.hpp"
#include "CommandFrames/AvrIsp/ReadFuse.hpp"
#include "CommandFrames/AvrIsp/ReadLock.hpp"
#include "CommandFrames/AvrIsp/ProgramFuse.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr
{
    using namespace Targets::Microchip::Avr8;

    using CommandFrames::AvrIsp::EnterProgrammingMode;
    using CommandFrames::AvrIsp::LeaveProgrammingMode;
    using CommandFrames::AvrIsp::ReadSignature;
    using CommandFrames::AvrIsp::ReadFuse;
    using CommandFrames::AvrIsp::ReadLock;
    using CommandFrames::AvrIsp::ProgramFuse;

    using ResponseFrames::AvrIsp::StatusCode;

    using Exceptions::TargetOperationFailure;

    EdbgAvrIspInterface::EdbgAvrIspInterface(
        EdbgInterface* edbgInterface,
        const TargetDescriptionFile& targetDescriptionFile
    )
        : edbgInterface(edbgInterface)
        , ispParameters(IspParameters{targetDescriptionFile})
    {}

    void EdbgAvrIspInterface::activate() {
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            EnterProgrammingMode{
                this->ispParameters.programModeTimeout,
                this->ispParameters.programModeStabilizationDelay,
                this->ispParameters.programModeCommandExecutionDelay,
                this->ispParameters.programModeSyncLoops,
                this->ispParameters.programModeByteDelay,
                this->ispParameters.programModePollValue,
                this->ispParameters.programModePollIndex
            }
        );

        if (responseFrame.statusCode != StatusCode::OK) {
            throw TargetOperationFailure{
                "Failed to enable programming mode via the ISP interface - check target's SPI connection "
                    "and/or its SPIEN fuse bit."
            };
        }
    }

    void EdbgAvrIspInterface::deactivate() {
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            LeaveProgrammingMode{this->ispParameters.programModePreDelay, this->ispParameters.programModePostDelay}
        );

        if (responseFrame.statusCode != StatusCode::OK) {
            throw TargetOperationFailure{"Failed to disable programming mode via the ISP interface."};
        }
    }

    TargetSignature EdbgAvrIspInterface::getDeviceId() {
        // The read signature command only allows us to read one signature byte at a time.
        return {this->readSignatureByte(0), this->readSignatureByte(1), this->readSignatureByte(2)};
    }

    FuseValue EdbgAvrIspInterface::readFuse(const Targets::TargetRegisterDescriptor& fuseRegisterDescriptor) {
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            ReadFuse{this->resolveFuseType(fuseRegisterDescriptor), this->ispParameters.readFusePollIndex}
        );

        if (
            responseFrame.statusCode != StatusCode::OK
            || responseFrame.payload.size() < 4
            || static_cast<StatusCode>(responseFrame.payload[3]) != StatusCode::OK
        ) {
            throw TargetOperationFailure{
                "Failed to read fuse via ISP - response frame status code/size indicates a failure."
            };
        }

        return static_cast<FuseValue>(responseFrame.payload[2]);
    }

    unsigned char EdbgAvrIspInterface::readLockBitByte() {
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            ReadLock{this->ispParameters.readLockPollIndex}
        );

        if (
            responseFrame.statusCode != StatusCode::OK
            || responseFrame.payload.size() < 4
            || static_cast<StatusCode>(responseFrame.payload[3]) != StatusCode::OK
        ) {
            throw TargetOperationFailure{
                "Failed to read lock bit byte via ISP - response frame status code/size indicates a failure."
            };
        }

        return responseFrame.payload[2];
    }

    void EdbgAvrIspInterface::programFuse(
        const Targets::TargetRegisterDescriptor& fuseRegisterDescriptor,
        FuseValue value
    ) {
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            ProgramFuse{this->resolveFuseType(fuseRegisterDescriptor), value}
        );

        if (responseFrame.statusCode != StatusCode::OK || responseFrame.payload.size() < 2) {
            throw TargetOperationFailure{
                "Failed to program fuse via ISP - response frame status code/size indicates a failure."
            };
        }
    }

    unsigned char EdbgAvrIspInterface::readSignatureByte(std::uint8_t signatureByteAddress) {
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            ReadSignature{signatureByteAddress, this->ispParameters.readSignaturePollIndex}
        );

        if (
            responseFrame.statusCode != StatusCode::OK
            || responseFrame.payload.size() < 4
            || static_cast<StatusCode>(responseFrame.payload[3]) != StatusCode::OK
        ) {
            throw TargetOperationFailure{
                "Failed to read signature byte (address: " + std::to_string(signatureByteAddress)
                    + ") via ISP - response frame status code/size indicates a failure."
            };
        }

        return responseFrame.payload[2];
    }

    FuseType EdbgAvrIspInterface::resolveFuseType(const Targets::TargetRegisterDescriptor& fuseRegisterDescriptor) {
        if (fuseRegisterDescriptor.key == "low") {
            return FuseType::LOW;
        }

        if (fuseRegisterDescriptor.key == "high") {
            return FuseType::HIGH;
        }

        if (fuseRegisterDescriptor.key == "extended") {
            return FuseType::EXTENDED;
        }

        throw Exceptions::InternalFatalErrorException{
            "Could not resolve fuse type from register descriptor (key: \"" + fuseRegisterDescriptor.key + "\")"
        };
    }
}
