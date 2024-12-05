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
        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        Targets::TargetMemoryAddress address,
        Targets::TargetMemorySize size,
        Services::TargetControllerService& targetControllerService
    ) {
        if (this->internalBreakpointRegistry.contains(addressSpaceDescriptor.id, address)) {
            return;
        }

        const auto externalBreakpoint = this->externalBreakpointRegistry.find(addressSpaceDescriptor.id, address);
        if (externalBreakpoint.has_value()) {
            // We already have an external breakpoint at this address
            this->internalBreakpointRegistry.insert(externalBreakpoint->get());
            return;
        }

        this->internalBreakpointRegistry.insert(
            targetControllerService.setProgramBreakpointAnyType(
                addressSpaceDescriptor,
                memorySegmentDescriptor,
                address,
                size
            )
        );
    }

    void DebugSession::removeInternalBreakpoint(
        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        Targets::TargetMemoryAddress address,
        Services::TargetControllerService& targetControllerService
    ) {
        const auto breakpoint = this->internalBreakpointRegistry.find(addressSpaceDescriptor.id, address);
        if (!breakpoint.has_value()) {
            return;
        }

        if (!this->externalBreakpointRegistry.contains(addressSpaceDescriptor.id, address)) {
            targetControllerService.removeProgramBreakpoint(breakpoint->get());
        }

        this->internalBreakpointRegistry.remove(addressSpaceDescriptor.id, address);
    }

    void DebugSession::setExternalBreakpoint(
        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        Targets::TargetMemoryAddress address,
        Targets::TargetMemorySize size,
        Services::TargetControllerService& targetControllerService
    ) {
        if (this->externalBreakpointRegistry.contains(addressSpaceDescriptor.id, address)) {
            return;
        }

        const auto internalBreakpoint = this->internalBreakpointRegistry.find(addressSpaceDescriptor.id, address);
        if (internalBreakpoint.has_value()) {
            // We already have an internal breakpoint at this address
            this->externalBreakpointRegistry.insert(internalBreakpoint->get());
            return;
        }

        this->externalBreakpointRegistry.insert(
            targetControllerService.setProgramBreakpointAnyType(
                addressSpaceDescriptor,
                memorySegmentDescriptor,
                address,
                size
            )
        );
    }

    void DebugSession::removeExternalBreakpoint(
        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        Targets::TargetMemoryAddress address,
        Services::TargetControllerService& targetControllerService
    ) {
        const auto breakpoint = this->externalBreakpointRegistry.find(addressSpaceDescriptor.id, address);
        if (!breakpoint.has_value()) {
            return;
        }

        if (!this->internalBreakpointRegistry.contains(addressSpaceDescriptor.id, address)) {
            targetControllerService.removeProgramBreakpoint(breakpoint->get());
        }

        this->externalBreakpointRegistry.remove(addressSpaceDescriptor.id, address);
    }

    void DebugSession::startRangeSteppingSession(
        RangeSteppingSession&& session,
        Services::TargetControllerService& targetControllerService
    ) {
        for (const auto& interceptAddress : session.interceptedAddresses) {
            /*
             * Have hard-coded the breakpoint size here, as range stepping is only supported on AVR targets.
             *
             * TODO: Review this after v2.0.0. Maybe move this range-stepping code out to an AVR-specific DebugSession
             *       struct. Or, refactor the range stepping session object to accommodate breakpoint sizes.
             */
            this->setInternalBreakpoint(
                session.addressSpaceDescriptor,
                session.memorySegmentDescriptor,
                interceptAddress,
                2,
                targetControllerService
            );
        }

        this->activeRangeSteppingSession.emplace(std::move(session));
    }

    void DebugSession::terminateRangeSteppingSession(Services::TargetControllerService& targetControllerService) {
        if (!this->activeRangeSteppingSession.has_value()) {
            return;
        }

        // Clear all intercepting breakpoints
        for (const auto& interceptAddress : this->activeRangeSteppingSession->interceptedAddresses) {
            this->removeInternalBreakpoint(
                this->activeRangeSteppingSession->addressSpaceDescriptor,
                interceptAddress,
                targetControllerService
            );
        }

        this->activeRangeSteppingSession.reset();
    }
}
