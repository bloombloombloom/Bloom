#pragma once

#include <memory>
#include <map>
#include <string>
#include <functional>
#include <QJsonObject>
#include <QJsonArray>

#include "src/Helpers/Thread.hpp"
#include "src/Logger/Logger.hpp"
#include "src/EventManager/EventListener.hpp"
#include "src/DebugToolDrivers/DebugTools.hpp"
#include "src/Targets/Target.hpp"
#include "src/Targets/Targets.hpp"
#include "src/EventManager/EventManager.hpp"
#include "src/EventManager/Events/Events.hpp"

namespace Bloom
{
    using namespace Targets;
    using namespace DebugToolDrivers;
    using namespace Targets::Microchip::Avr;
    using Avr8Bit::Avr8;
    using Events::EventPointer;

    /**
     * The TargetController possesses full control of the debugging target and the debug tool.
     *
`    * The TargetController runs on a dedicated thread. Its sole purpose is to handle communication to & from the
     * debug tool and target.
     *
     * The TargetController should be oblivious to any manufacture/device specific functionality. It should
     * only ever interface with the base Target and DebugTool classes.
     */
    class TargetController: public Thread
    {
    private:
        ApplicationConfig applicationConfig;
        EnvironmentConfig environmentConfig;

        /**
         * The TargetController should be the sole owner of the target and debugTool. They are constructed and
         * destroyed within the TargetController. Under no circumstance should ownership of these resources be
         * transferred to any other component within Bloom.
         */
        std::unique_ptr<Targets::Target> target = nullptr;
        std::unique_ptr<DebugTool> debugTool = nullptr;

        EventManager& eventManager;
        EventListenerPointer eventListener = std::make_shared<EventListener>("TargetControllerEventListener");

        /**
         * We keep record of the last known execution state of the target. When the connected target reports a
         * different state to what's stored in lastTargetState, a state change (TargetExecutionStopped/TargetExecutionResumed)
         * event is emitted.
         */
        TargetState lastTargetState = TargetState::UNKNOWN;

        /**
         * Obtaining a TargetDescriptor for the connected target can be quite expensive. We cache it here.
         */
        std::optional<TargetDescriptor> cachedTargetDescriptor;

        /**
         * Constructs a mapping of supported debug tool names to lambdas. The lambdas should *only* instantiate
         * and return an instance to the derived DebugTool class. They should never attempt to establish
         * a connection to the device.
         *
         * @return
         */
        static auto getSupportedDebugTools() {
            return std::map<std::string, std::function<std::unique_ptr<DebugTool>()>> {
                {
                    "atmel-ice",
                    []() {
                        return std::make_unique<AtmelIce>();
                    }
                },
                {
                    "power-debugger",
                    []() {
                        return std::make_unique<PowerDebugger>();
                    }
                },
                {
                    "snap",
                    []() {
                        return std::make_unique<MplabSnap>();
                    }
                },
            };
        }

        /**
         * Constructs a mapping of supported target names to lambdas. The lambdas should instantiate and return an
         * instance to the appropriate Target class.
         *
         * @return
         */
        static auto getSupportedTargets() {
            auto mapping = std::map<std::string, std::function<std::unique_ptr<Targets::Target>()>> {
                {
                    "avr8",
                    []() {
                        return std::make_unique<Avr8>();
                    }
                },
            };

            // Include all targets from AVR8 part description files
            auto avr8PdMapping = Avr8Bit::PartDescriptionFile::getPartDescriptionMapping();

            for (auto mapIt = avr8PdMapping.begin(); mapIt != avr8PdMapping.end(); mapIt++) {
                // Each target signature maps to an array of targets, as numerous targets can possess the same signature.
                auto targets = mapIt.value().toArray();

                for (auto targetIt = targets.begin(); targetIt != targets.end(); targetIt++) {
                    auto targetName = targetIt->toObject().find("targetName").value().toString()
                        .toLower().toStdString();
                    auto targetSignatureHex = mapIt.key().toLower().toStdString();

                    if (!mapping.contains(targetName)) {
                        mapping.insert({
                           targetName,
                           [targetName, targetSignatureHex]() {
                               return std::make_unique<Avr8>(targetName, TargetSignature(targetSignatureHex));
                           }
                       });
                    }
                }
            }

            return mapping;
        }

        void setDebugTool(std::unique_ptr<DebugTool> debugTool) {
            this->debugTool = std::move(debugTool);
        }

        void setTarget(std::unique_ptr<Targets::Target> target) {
            this->target = std::move(target);
        }

        Targets::Target* getTarget() {
            return this->target.get();
        }

        DebugTool* getDebugTool() {
            return this->debugTool.get();
        }

        /**
         * Updates the state of the TargetController and emits a state changed event.
         *
         * @param state
         * @param emitEvent
         */
        void setStateAndEmitEvent(ThreadState state) {
            this->setState(state);
            this->eventManager.triggerEvent(
                std::make_shared<Events::TargetControllerStateChanged>(state)
            );
        };

        /**
         * Installs Bloom's udev rules on user's machine. Rules are copied from build/Distribution/Resources/UdevRules
         * to /etc/udev/rules.d/. This method will report an error if Bloom isn't running as root (as root privileges
         * are required for writing to files in /etc/udev).
         */
        void checkUdevRules();

        /**
         * Because the TargetController hogs the thread, this method must be called in a dedicated thread.
         */
        void startup();

        /**
         * Exit point - must be called before the TargetController thread is terminated.
         *
         * Handles deactivating the target among other clean-up related things.
         */
        void shutdown();

        /**
         * Should fire any events queued on the target.
         */
        void fireTargetEvents();

        /**
         * When the TargetController fails to handle an event, a TargetControllerErrorOccurred event is emitted, with
         * a correlation ID matching the ID of the event that triggered the handler.
         *
         * @param correlationId
         */
        void emitErrorEvent(int correlationId);

    public:
        TargetController(EventManager& eventManager): eventManager(eventManager) {};

        void setApplicationConfig(const ApplicationConfig& applicationConfig) {
            this->applicationConfig = applicationConfig;
        }

        void setEnvironmentConfig(const EnvironmentConfig& environmentConfig) {
            this->environmentConfig = environmentConfig;
        }

        /**
         * Entry point for the TargetController.
         */
        void run();

        /**
         * Obtains a TargetDescriptor from the target and includes it in a TargetDescriptorExtracted event.
         *
         * @param event
         */
        void onExtractTargetDescriptor(EventPointer<Events::ExtractTargetDescriptor> event);

        /**
         * Will attempt to stop execution on the target and emit a TargetExecutionStopped event.
         *
         * @param event
         */
        void onStopTargetExecutionEvent(EventPointer<Events::StopTargetExecution> event);

        /**
         * Will attempt to step execution on the target and emit a TargetExecutionResumed event.
         *
         * @param event
         */
        void onStepTargetExecutionEvent(EventPointer<Events::StepTargetExecution> event);

        /**
         * Will attempt to resume execution on the target and emit a TargetExecutionResumed event.
         *
         * @param event
         */
        void onResumeTargetExecutionEvent(EventPointer<Events::ResumeTargetExecution> event);

        /**
         * Invokes a shutdown.
         *
         * @param event
         */
        void onShutdownTargetControllerEvent(EventPointer<Events::ShutdownTargetController> event);

        /**
         * Will attempt to read the requested registers and emit a RegistersRetrievedFromTarget event.
         *
         * @param event
         */
        void onReadRegistersEvent(EventPointer<Events::RetrieveRegistersFromTarget> event);

        /**
         * Will attempt to write the specified register values and emit a RegistersWrittenToTarget event.
         *
         * @param event
         */
        void onWriteRegistersEvent(EventPointer<Events::WriteRegistersToTarget> event);

        /**
         * Will attempt to read memory from the target and include the data in a MemoryRetrievedFromTarget event.
         *
         * @param event
         */
        void onReadMemoryEvent(EventPointer<Events::RetrieveMemoryFromTarget> event);

        /**
         * Will attempt to write memory to the target. On success, a MemoryWrittenToTarget event is emitted.
         *
         * @param event
         */
        void onWriteMemoryEvent(EventPointer<Events::WriteMemoryToTarget> event);

        /**
         * Will attempt to set the specific breakpoint on the target. On success, the BreakpointSetOnTarget event will
         * be emitted.
         *
         * @param event
         */
        void onSetBreakpointEvent(EventPointer<Events::SetBreakpointOnTarget> event);

        /**
         * Will attempt to remove a breakpoint at the specified address, on the target. On success, the
         * BreakpointRemovedOnTarget event is emitted.
         *
         * @param event
         */
        void onRemoveBreakpointEvent(EventPointer<Events::RemoveBreakpointOnTarget> event);

        /**
         * Will hold the target stopped at it's current state.
         *
         * @param event
         */
        void onDebugSessionStartedEvent(EventPointer<Events::DebugSessionStarted> event);

        /**
         * Will simply kick off execution on the target.
         *
         * @param event
         */
        void onDebugSessionFinishedEvent(EventPointer<Events::DebugSessionFinished> event);

        /**
         * Will update the program counter value on the target. On success, a ProgramCounterSetOnTarget event is
         * emitted.
         *
         * @param event
         */
        void onSetProgramCounterEvent(EventPointer<Events::SetProgramCounterOnTarget> event);

        /**
         * Will automatically fire a target state update event.
         * @TODO: get rid of this - Insight should request this itself.
         *
         * @param event
         */
        void onInsightStateChangedEvent(EventPointer<Events::InsightStateChanged> event);

        /**
         * Will attempt to obtain the pin states from the target. Will emit a TargetPinStatesRetrieved event on success.
         *
         * @param event
         */
        void onRetrieveTargetPinStatesEvent(EventPointer<Events::RetrieveTargetPinStates> event);

        /**
         * Will update a pin state for a particular pin. Will emit a TargetPinStatesRetrieved with the new pin
         * state, on success.
         *
         * @param event
         */
        void onSetPinStateEvent(EventPointer<Events::SetTargetPinState> event);
    };
}
