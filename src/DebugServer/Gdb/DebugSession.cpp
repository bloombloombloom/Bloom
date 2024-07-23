#include "DebugSession.hpp"

#include "src/EventManager/EventManager.hpp"

namespace DebugServer::Gdb
{
    DebugSession::DebugSession(
        Connection&& connection,
        const std::set<std::pair<Feature, std::optional<std::string>>>& supportedFeatures,
        const GdbDebugServerConfig& serverConfig
    )
        : connection(std::move(connection))
        , supportedFeatures(supportedFeatures)
        , serverConfig(serverConfig)
    {
        this->supportedFeatures.emplace(
            Feature::PACKET_SIZE,
            std::to_string(Connection::ABSOLUTE_MAXIMUM_PACKET_READ_SIZE)
        );

        EventManager::triggerEvent(std::make_shared<Events::DebugSessionStarted>());
    }

    DebugSession::~DebugSession() {
        EventManager::triggerEvent(std::make_shared<Events::DebugSessionFinished>());
    }

    void DebugSession::setInternalBreakpoint(
        Targets::TargetMemoryAddress address,
        Services::TargetControllerService& targetControllerService
    ) {
        if (this->internalBreakpointsByAddress.contains(address)) {
            return;
        }

        const auto externalBreakpointIt = this->externalBreakpointsByAddress.find(address);
        if (externalBreakpointIt != this->externalBreakpointsByAddress.end()) {
            // We already have an external breakpoint at this address
            this->internalBreakpointsByAddress.emplace(address, externalBreakpointIt->second);
            return;
        }

        this->internalBreakpointsByAddress.emplace(
            address,
            targetControllerService.setBreakpoint(address, Targets::TargetBreakpoint::Type::HARDWARE)
        );
    }

    void DebugSession::removeInternalBreakpoint(
        Targets::TargetMemoryAddress address,
        Services::TargetControllerService& targetControllerService
    ) {
        const auto breakpointIt = this->internalBreakpointsByAddress.find(address);
        if (breakpointIt == this->internalBreakpointsByAddress.end()) {
            return;
        }

        if (!this->externalBreakpointsByAddress.contains(address)) {
            targetControllerService.removeBreakpoint(breakpointIt->second);
        }

        this->internalBreakpointsByAddress.erase(breakpointIt);
    }

    void DebugSession::setExternalBreakpoint(
        Targets::TargetMemoryAddress address,
        Services::TargetControllerService& targetControllerService
    ) {
        if (this->externalBreakpointsByAddress.contains(address)) {
            return;
        }

        const auto internalBreakpointIt = this->internalBreakpointsByAddress.find(address);

        if (internalBreakpointIt != this->internalBreakpointsByAddress.end()) {
            // We already have an internal breakpoint at this address
            this->externalBreakpointsByAddress.emplace(address, internalBreakpointIt->second);
            return;
        }

        this->externalBreakpointsByAddress.emplace(
            address,
            targetControllerService.setBreakpoint(address, Targets::TargetBreakpoint::Type::HARDWARE)
        );
    }

    void DebugSession::removeExternalBreakpoint(
        Targets::TargetMemoryAddress address,
        Services::TargetControllerService& targetControllerService
    ) {
        const auto breakpointIt = this->externalBreakpointsByAddress.find(address);
        if (breakpointIt == this->externalBreakpointsByAddress.end()) {
            return;
        }

        if (!this->internalBreakpointsByAddress.contains(address)) {
            targetControllerService.removeBreakpoint(breakpointIt->second);
        }

        this->externalBreakpointsByAddress.erase(breakpointIt);
    }

    void DebugSession::startRangeSteppingSession(
        RangeSteppingSession&& session,
        Services::TargetControllerService& targetControllerService
    ) {
        for (const auto& interceptAddress : session.interceptedAddresses) {
            this->setInternalBreakpoint(interceptAddress, targetControllerService);
        }

        this->activeRangeSteppingSession = std::move(session);
    }

    void DebugSession::terminateRangeSteppingSession(Services::TargetControllerService& targetControllerService) {
        if (!this->activeRangeSteppingSession.has_value()) {
            return;
        }

        // Clear all intercepting breakpoints
        for (const auto& interceptAddress : this->activeRangeSteppingSession->interceptedAddresses) {
            this->removeInternalBreakpoint(interceptAddress, targetControllerService);
        }

        this->activeRangeSteppingSession.reset();
    }
}
