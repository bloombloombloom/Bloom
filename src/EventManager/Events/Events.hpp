#pragma once

#include <memory>

#include "Event.hpp"
#include "StopTargetExecution.hpp"
#include "ResumeTargetExecution.hpp"
#include "ResetTarget.hpp"
#include "DebugSessionStarted.hpp"
#include "DebugSessionFinished.hpp"
#include "TargetControllerThreadStateChanged.hpp"
#include "ReportTargetControllerState.hpp"
#include "TargetControllerStateReported.hpp"
#include "ShutdownTargetController.hpp"
#include "TargetControllerErrorOccurred.hpp"
#include "ShutdownApplication.hpp"
#include "DebugServerThreadStateChanged.hpp"
#include "ShutdownDebugServer.hpp"
#include "RetrieveRegistersFromTarget.hpp"
#include "RegistersRetrievedFromTarget.hpp"
#include "WriteRegistersToTarget.hpp"
#include "RegistersWrittenToTarget.hpp"
#include "TargetExecutionResumed.hpp"
#include "TargetExecutionStopped.hpp"
#include "RetrieveMemoryFromTarget.hpp"
#include "MemoryRetrievedFromTarget.hpp"
#include "WriteMemoryToTarget.hpp"
#include "MemoryWrittenToTarget.hpp"
#include "SetBreakpointOnTarget.hpp"
#include "RemoveBreakpointOnTarget.hpp"
#include "BreakpointSetOnTarget.hpp"
#include "BreakpointRemovedOnTarget.hpp"
#include "StepTargetExecution.hpp"
#include "SetProgramCounterOnTarget.hpp"
#include "ProgramCounterSetOnTarget.hpp"
#include "ExtractTargetDescriptor.hpp"
#include "TargetDescriptorExtracted.hpp"
#include "InsightThreadStateChanged.hpp"
#include "RetrieveTargetPinStates.hpp"
#include "TargetPinStatesRetrieved.hpp"
#include "SetTargetPinState.hpp"
#include "TargetIoPortsUpdated.hpp"

namespace Bloom::Events
{
    template <class EventType>
    using SharedEventPointer = std::shared_ptr<const EventType>;

    template <class EventType>
    using EventRef = const EventType&;

    using SharedGenericEventPointer = SharedEventPointer<Event>;
    using GenericEventRef = EventRef<Event>;
}
