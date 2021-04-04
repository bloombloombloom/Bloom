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
`    * The TargetController runs on a dedicated thread. Its sole purpose is to handle communication to  &from the
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

        std::unique_ptr<Targets::Target> target = nullptr;
        std::unique_ptr<DebugTool> debugTool = nullptr;

        EventManager& eventManager;
        EventListenerPointer eventListener = std::make_shared<EventListener>("TargetControllerEventListener");

        TargetState lastTargetState = TargetState::UNKNOWN;
        std::optional<TargetDescriptor> cachedTargetDescriptor;

        /**
         * Constructs a mapping of supported debug tool names to lambdas. The lambdas should *only* instantiate
         * and return an instance to the derived DebugTool class. They should never attempt to establish
         * a connection to the device.
         *
         * Currently, the only debug tool we support is the Atmel-ICE.
         *
         * @return
         */
        static auto getSupportedDebugTools() {
            return std::map<std::string, std::function<std::unique_ptr<DebugTool>()>> {
                {
                    "atmel-ice",
                    []() -> std::unique_ptr<DebugTool> {
                        return std::make_unique<AtmelIce>();
                    }
                },
                {
                    "power-debugger",
                    []() -> std::unique_ptr<DebugTool> {
                        return std::make_unique<PowerDebugger>();
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
                    []() -> std::unique_ptr<Targets::Target> {
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

                    if (mapping.find(targetName) == mapping.end()) {
                        mapping.insert({
                           targetName,
                           [targetName]() -> std::unique_ptr<Targets::Target> {
                               return std::make_unique<Avr8>(targetName);
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

        void emitErrorEvent(int correlationId);
    public:
        TargetController(EventManager& eventManager) : eventManager(eventManager) {};

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

        void onExtractTargetDescriptor(EventPointer<Events::ExtractTargetDescriptor> event);

        /**
         * Callback for StopTargetExecution event.
         *
         * Will attempt to stop the target and emit a TargetExecutionStopped event.
         */
        void onStopTargetExecutionEvent(EventPointer<Events::StopTargetExecution> event);

        void onStepTargetExecutionEvent(EventPointer<Events::StepTargetExecution> event);

        /**
         * Callback for ResumeTargetExecution event.
         */
        void onResumeTargetExecutionEvent(EventPointer<Events::ResumeTargetExecution> event);

        /**
         * Callback for ShutdownTargetController event.
         */
        void onShutdownTargetControllerEvent(EventPointer<Events::ShutdownTargetController> event);

        void onReadRegistersEvent(EventPointer<Events::RetrieveRegistersFromTarget> event);
        void onWriteRegistersEvent(EventPointer<Events::WriteRegistersToTarget> event);
        void onReadMemoryEvent(EventPointer<Events::RetrieveMemoryFromTarget> event);
        void onWriteMemoryEvent(EventPointer<Events::WriteMemoryToTarget> event);
        void onSetBreakpointEvent(EventPointer<Events::SetBreakpointOnTarget> event);
        void onRemoveBreakpointEvent(EventPointer<Events::RemoveBreakpointOnTarget> event);
        void onDebugSessionStartedEvent(EventPointer<Events::DebugSessionStarted> event);
        void onDebugSessionFinishedEvent(EventPointer<Events::DebugSessionFinished> event);
        void onSetProgramCounterEvent(EventPointer<Events::SetProgramCounterOnTarget> event);
        void onInsightStateChangedEvent(EventPointer<Events::InsightStateChanged> event);
        void onRetrieveTargetPinStatesEvent(EventPointer<Events::RetrieveTargetPinStates> event);
        void onSetPinStateEvent(EventPointer<Events::SetTargetPinState> event);
    };
}
