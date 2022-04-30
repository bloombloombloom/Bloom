#pragma once

#include <memory>

#include "Event.hpp"
#include "DebugSessionStarted.hpp"
#include "DebugSessionFinished.hpp"
#include "TargetControllerThreadStateChanged.hpp"
#include "TargetControllerStateChanged.hpp"
#include "ShutdownTargetController.hpp"
#include "TargetControllerErrorOccurred.hpp"
#include "ShutdownApplication.hpp"
#include "DebugServerThreadStateChanged.hpp"
#include "ShutdownDebugServer.hpp"
#include "RegistersWrittenToTarget.hpp"
#include "TargetExecutionResumed.hpp"
#include "TargetExecutionStopped.hpp"
#include "MemoryWrittenToTarget.hpp"
#include "SetBreakpointOnTarget.hpp"
#include "RemoveBreakpointOnTarget.hpp"
#include "BreakpointSetOnTarget.hpp"
#include "BreakpointRemovedOnTarget.hpp"
#include "SetProgramCounterOnTarget.hpp"
#include "ProgramCounterSetOnTarget.hpp"
#include "ExtractTargetDescriptor.hpp"
#include "TargetDescriptorExtracted.hpp"
#include "InsightThreadStateChanged.hpp"
#include "RetrieveTargetPinStates.hpp"
#include "TargetPinStatesRetrieved.hpp"
#include "SetTargetPinState.hpp"
#include "RetrieveStackPointerFromTarget.hpp"
#include "StackPointerRetrievedFromTarget.hpp"
#include "TargetReset.hpp"

namespace Bloom::Events
{
    template <class EventType>
    using SharedEventPointer = std::shared_ptr<const EventType>;

    template <class EventType>
    using SharedEventPointerNonConst = std::shared_ptr<EventType>;

    using SharedGenericEventPointer = SharedEventPointer<Event>;
}
