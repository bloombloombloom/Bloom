#pragma once

#include <memory>

#include "Event.hpp"
#include "DebugSessionStarted.hpp"
#include "DebugSessionFinished.hpp"
#include "TargetControllerThreadStateChanged.hpp"
#include "ShutdownTargetController.hpp"
#include "TargetControllerErrorOccurred.hpp"
#include "ShutdownApplication.hpp"
#include "DebugServerThreadStateChanged.hpp"
#include "ShutdownDebugServer.hpp"
#include "RegistersWrittenToTarget.hpp"
#include "TargetStateChanged.hpp"
#include "MemoryWrittenToTarget.hpp"
#include "TargetReset.hpp"

#ifndef EXCLUDE_INSIGHT
#include "InsightActivationRequested.hpp"
#include "InsightMainWindowClosed.hpp"
#endif

namespace Events
{
    template <class EventType>
    using SharedEventPointer = std::shared_ptr<const EventType>;

    using SharedGenericEventPointer = SharedEventPointer<Event>;
}
