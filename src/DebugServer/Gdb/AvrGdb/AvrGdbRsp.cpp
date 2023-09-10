#include "AvrGdbRsp.hpp"

#include "src/Services/StringService.hpp"

// Command packets
#include "CommandPackets/ReadRegister.hpp"
#include "CommandPackets/ReadRegisters.hpp"
#include "CommandPackets/WriteRegister.hpp"
#include "CommandPackets/ReadMemory.hpp"
#include "CommandPackets/WriteMemory.hpp"
#include "CommandPackets/ReadMemoryMap.hpp"
#include "CommandPackets/FlashErase.hpp"
#include "CommandPackets/FlashWrite.hpp"
#include "CommandPackets/FlashDone.hpp"
#include "CommandPackets/VContSupportedActionsQuery.hpp"
#include "CommandPackets/VContContinueExecution.hpp"
#include "CommandPackets/VContStepExecution.hpp"
#include "CommandPackets/VContRangeStep.hpp"

namespace DebugServer::Gdb::AvrGdb
{
    using namespace Exceptions;

    using Targets::TargetRegisterDescriptor;
    using Targets::TargetRegisterType;

    AvrGdbRsp::AvrGdbRsp(
        const DebugServerConfig& debugServerConfig,
        EventListener& eventListener,
        EventFdNotifier& eventNotifier
    )
        : GdbRspDebugServer(debugServerConfig, eventListener, eventNotifier)
        , gdbTargetDescriptor(TargetDescriptor(this->targetControllerService.getTargetDescriptor()))
    {}

    DebugSession* AvrGdbRsp::startDebugSession(Connection&& connection) {
        this->activeDebugSession.emplace(
            std::move(connection),
            this->getSupportedFeatures(),
            this->gdbTargetDescriptor,
            this->debugServerConfig
        );

        return &*(this->activeDebugSession);
    }

    void AvrGdbRsp::endDebugSession() {
        this->activeDebugSession.reset();
    }

    const Gdb::TargetDescriptor& AvrGdbRsp::getGdbTargetDescriptor() {
        return this->gdbTargetDescriptor;
    }

    DebugSession* AvrGdbRsp::getActiveDebugSession() {
        return this->activeDebugSession.has_value() ? &*(this->activeDebugSession) : nullptr;
    }

    std::unique_ptr<Gdb::CommandPackets::CommandPacket> AvrGdbRsp::resolveCommandPacket(
        const RawPacket& rawPacket
    ) {
        using AvrGdb::CommandPackets::ReadRegister;
        using AvrGdb::CommandPackets::ReadRegisters;
        using AvrGdb::CommandPackets::WriteRegister;
        using AvrGdb::CommandPackets::ReadMemory;
        using AvrGdb::CommandPackets::WriteMemory;
        using AvrGdb::CommandPackets::ReadMemoryMap;
        using AvrGdb::CommandPackets::FlashErase;
        using AvrGdb::CommandPackets::FlashWrite;
        using AvrGdb::CommandPackets::FlashDone;
        using AvrGdb::CommandPackets::VContSupportedActionsQuery;
        using AvrGdb::CommandPackets::VContContinueExecution;
        using AvrGdb::CommandPackets::VContStepExecution;
        using AvrGdb::CommandPackets::VContRangeStep;

        if (rawPacket.size() >= 2) {
            if (rawPacket[1] == 'p') {
                return std::make_unique<ReadRegister>(rawPacket);
            }

            if (rawPacket[1] == 'g') {
                return std::make_unique<ReadRegisters>(rawPacket);
            }

            if (rawPacket[1] == 'P') {
                return std::make_unique<WriteRegister>(rawPacket);
            }

            if (rawPacket[1] == 'm') {
                return std::make_unique<ReadMemory>(rawPacket, this->gdbTargetDescriptor);
            }

            if (rawPacket[1] == 'M') {
                return std::make_unique<WriteMemory>(rawPacket, this->gdbTargetDescriptor);
            }

            const auto rawPacketString = std::string(rawPacket.begin() + 1, rawPacket.end());

            if (rawPacketString.find("qXfer:memory-map:read::") == 0) {
                return std::make_unique<ReadMemoryMap>(rawPacket);
            }

            if (rawPacketString.find("vFlashErase") == 0) {
                return std::make_unique<FlashErase>(rawPacket);
            }

            if (rawPacketString.find("vFlashWrite") == 0) {
                return std::make_unique<FlashWrite>(rawPacket);
            }

            if (rawPacketString.find("vFlashDone") == 0) {
                return std::make_unique<FlashDone>(rawPacket);
            }

            if (rawPacketString.find("vCont?") == 0) {
                return std::make_unique<VContSupportedActionsQuery>(rawPacket);
            }

            if (rawPacketString.find("vCont;c") == 0 || rawPacketString.find("vCont;C") == 0) {
                return std::make_unique<VContContinueExecution>(rawPacket);
            }

            if (rawPacketString.find("vCont;s") == 0 || rawPacketString.find("vCont;S") == 0) {
                return std::make_unique<VContStepExecution>(rawPacket);
            }

            if (this->debugServerConfig.rangeSteppingEnabled) {
                if (rawPacketString.find("vCont;r") == 0) {
                    return std::make_unique<VContRangeStep>(rawPacket);
                }
            }
        }

        return GdbRspDebugServer::resolveCommandPacket(rawPacket);
    }

    std::set<std::pair<Feature, std::optional<std::string>>> AvrGdbRsp::getSupportedFeatures() {
        return {
            {Feature::SOFTWARE_BREAKPOINTS, std::nullopt},
            {Feature::MEMORY_MAP_READ, std::nullopt},
            {Feature::VCONT_ACTIONS_QUERY, std::nullopt},
        };
    }

    void AvrGdbRsp::handleTargetStoppedGdbResponse(Targets::TargetProgramCounter programAddress) {
        using Services::StringService;

        Logger::debug("Target stopped at byte address: 0x" + StringService::toHex(programAddress));

        auto& activeRangeSteppingSession = this->activeDebugSession->activeRangeSteppingSession;

        if (
            activeRangeSteppingSession.has_value()
            && programAddress >= activeRangeSteppingSession->range.startAddress
            && programAddress < activeRangeSteppingSession->range.endAddress
        ) {
            /*
             * The target stopped within the stepping range of an active range stepping session.
             *
             * We need to figure out why, and determine whether the stop should be reported to GDB.
             */
            if (this->activeDebugSession->externalBreakpointAddresses.contains(programAddress)) {
                /*
                 * The target stopped due to an external breakpoint, set by GDB.
                 *
                 * We have to end the range stepping session and report this to GDB.
                 */
                Logger::debug("Reached external breakpoint within stepping range");

                this->activeDebugSession->terminateRangeSteppingSession(this->targetControllerService);
                return GdbRspDebugServer::handleTargetStoppedGdbResponse(programAddress);
            }

            if (activeRangeSteppingSession->interceptedAddresses.contains(programAddress)) {
                /*
                 * The target stopped due to an intercepting breakpoint, but we're still within the stepping range,
                 * which can only mean that we weren't sure where this instruction would lead to.
                 *
                 * We must perform a single step and see what happens.
                 */
                Logger::debug("Reached intercepting breakpoint within stepping range");
                Logger::debug("Attempting single step from 0x" + StringService::toHex(programAddress));

                activeRangeSteppingSession->singleStepping = true;
                this->targetControllerService.stepTargetExecution(std::nullopt);
                return;
            }

            if (activeRangeSteppingSession->singleStepping) {
                /*
                 * We performed a single step once and we're still within the stepping range, so we're good to
                 * continue the range stepping session.
                 */
                Logger::debug("Completed single step from an intercepted address - PC still within stepping range");
                Logger::debug("Continuing range stepping");

                activeRangeSteppingSession->singleStepping = false;
                this->targetControllerService.continueTargetExecution(std::nullopt, std::nullopt);
                return;
            }

            /*
             * If we get here, the target stopped for an unknown reason that doesn't seem to have anything to do with
             * the active range stepping session.
             *
             * This could be due to a permanent breakpoint in the target's program code.
             *
             * We have to end the range stepping session and report the stop to GDB.
             */
            Logger::debug("Target stopped within stepping range, but for an unknown reason");

            this->activeDebugSession->terminateRangeSteppingSession(this->targetControllerService);
            return GdbRspDebugServer::handleTargetStoppedGdbResponse(programAddress);
        }

        // Report the stop to GDB
        return GdbRspDebugServer::handleTargetStoppedGdbResponse(programAddress);
    }
}
