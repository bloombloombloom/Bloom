#include "DebugSession.hpp"

#include "src/EventManager/EventManager.hpp"

namespace DebugServer::Gdb
{
    DebugSession::DebugSession(
        Connection&& connection,
        const std::set<std::pair<Feature, std::optional<std::string>>>& supportedFeatures,
        const TargetDescriptor& targetDescriptor,
        const GdbDebugServerConfig& serverConfig
    )
        : connection(std::move(connection))
        , supportedFeatures(supportedFeatures)
        , gdbTargetDescriptor(targetDescriptor)
        , serverConfig(serverConfig)
    {
        this->supportedFeatures.insert({
            Feature::PACKET_SIZE, std::to_string(Connection::ABSOLUTE_MAXIMUM_PACKET_READ_SIZE)
        });

        EventManager::triggerEvent(std::make_shared<Events::DebugSessionStarted>());
    }

    DebugSession::~DebugSession() {
        EventManager::triggerEvent(std::make_shared<Events::DebugSessionFinished>());
    }

    void DebugSession::setInternalBreakpoint(
        const Targets::TargetBreakpoint& breakpoint,
        Services::TargetControllerService& targetControllerService
    ) {
        if (this->internalBreakpointAddresses.contains(breakpoint.address)) {
            return;
        }

        if (!this->externalBreakpointAddresses.contains(breakpoint.address)) {
            targetControllerService.setBreakpoint(breakpoint);
        }

        this->internalBreakpointAddresses.insert(breakpoint.address);
    }

    void DebugSession::removeInternalBreakpoint(
        const Targets::TargetBreakpoint& breakpoint,
        Services::TargetControllerService& targetControllerService
    ) {
        if (!this->internalBreakpointAddresses.contains(breakpoint.address)) {
            return;
        }

        if (!this->externalBreakpointAddresses.contains(breakpoint.address)) {
            targetControllerService.removeBreakpoint(breakpoint);
        }

        this->internalBreakpointAddresses.erase(breakpoint.address);
    }

    void DebugSession::setExternalBreakpoint(
        const Targets::TargetBreakpoint& breakpoint,
        Services::TargetControllerService& targetControllerService
    ) {
        if (this->externalBreakpointAddresses.contains(breakpoint.address)) {
            return;
        }

        if (!this->internalBreakpointAddresses.contains(breakpoint.address)) {
            targetControllerService.setBreakpoint(breakpoint);
        }

        this->externalBreakpointAddresses.insert(breakpoint.address);
    }

    void DebugSession::removeExternalBreakpoint(
        const Targets::TargetBreakpoint& breakpoint,
        Services::TargetControllerService& targetControllerService
    ) {
        if (!this->externalBreakpointAddresses.contains(breakpoint.address)) {
            return;
        }

        if (!this->internalBreakpointAddresses.contains(breakpoint.address)) {
            targetControllerService.removeBreakpoint(breakpoint);
        }

        this->externalBreakpointAddresses.erase(breakpoint.address);
    }

    void DebugSession::startRangeSteppingSession(
        RangeSteppingSession&& session,
        Services::TargetControllerService& targetControllerService
    ) {
        for (const auto& interceptAddress : session.interceptedAddresses) {
            this->setInternalBreakpoint(Targets::TargetBreakpoint(interceptAddress), targetControllerService);
        }

        this->activeRangeSteppingSession = std::move(session);
    }

    void DebugSession::terminateRangeSteppingSession(Services::TargetControllerService& targetControllerService) {
        if (!this->activeRangeSteppingSession.has_value()) {
            return;
        }

        // Clear all intercepting breakpoints
        for (const auto& interceptAddress : this->activeRangeSteppingSession->interceptedAddresses) {
            this->removeInternalBreakpoint(Targets::TargetBreakpoint(interceptAddress), targetControllerService);
        }

        this->activeRangeSteppingSession.reset();
    }
}
