#pragma once

#include <memory>
#include <map>
#include <string>
#include <functional>
#include <QJsonObject>
#include <QJsonArray>

#include "src/Helpers/Thread.hpp"
#include "TargetControllerState.hpp"

#include "src/DebugToolDrivers/DebugTools.hpp"
#include "src/Targets/Target.hpp"
#include "src/Targets/Targets.hpp"
#include "src/Targets/TargetRegister.hpp"
#include "src/Targets/TargetMemory.hpp"

#include "src/EventManager/EventManager.hpp"
#include "src/EventManager/EventListener.hpp"
#include "src/EventManager/Events/Events.hpp"

namespace Bloom
{
    /**
     * The TargetController possesses full control of the debugging target and the debug tool.
     *
     * The TargetController runs on a dedicated thread. Its sole purpose is to handle communication to & from the
     * debug tool and target.
     *
     * The TargetController should be oblivious to any manufacture/device specific functionality. It should
     * only ever interface with the base Target and DebugTool classes.
     */
    class TargetController: public Thread
    {
    public:
        explicit TargetController(EventManager& eventManager): eventManager(eventManager) {};

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

    private:
        /**
         * The TC starts off in a suspended state. TargetController::resume() is invoked from the startup routine.
         */
        TargetControllerState state = TargetControllerState::SUSPENDED;

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
        Targets::TargetState lastTargetState = Targets::TargetState::UNKNOWN;

        /**
         * Obtaining a TargetDescriptor for the connected target can be quite expensive. We cache it here.
         */
        std::optional<Targets::TargetDescriptor> cachedTargetDescriptor;

        /**
         * Target register descriptors mapped by the memory type on which the register is stored.
         */
        std::map<Targets::TargetMemoryType, Targets::TargetRegisterDescriptors> registerDescriptorsByMemoryType;

        /**
         * Memory address ranges for target registers, mapped by the register memory type.
         */
        std::map<Targets::TargetMemoryType, Targets::TargetMemoryAddressRange> registerAddressRangeByMemoryType;

        /**
         * Constructs a mapping of supported debug tool names to lambdas. The lambdas should *only* instantiate
         * and return an instance to the derived DebugTool class. They should not attempt to establish
         * a connection to the device.
         *
         * @return
         */
        static auto getSupportedDebugTools() {
            static auto mapping = std::map<std::string, std::function<std::unique_ptr<DebugTool>()>> {
                {
                    "atmel-ice",
                    [] {
                        return std::make_unique<DebugToolDrivers::AtmelIce>();
                    }
                },
                {
                    "power-debugger",
                    [] {
                        return std::make_unique<DebugToolDrivers::PowerDebugger>();
                    }
                },
                {
                    "snap",
                    [] {
                        return std::make_unique<DebugToolDrivers::MplabSnap>();
                    }
                },
                {
                    "pickit-4",
                    [] {
                        return std::make_unique<DebugToolDrivers::MplabPickit4>();
                    }
                },
                {
                    "xplained-pro",
                    [] {
                        return std::make_unique<DebugToolDrivers::XplainedPro>();
                    }
                },
                {
                    "xplained-mini",
                    [] {
                        return std::make_unique<DebugToolDrivers::XplainedMini>();
                    }
                },
                {
                    "xplained-nano",
                    [] {
                        return std::make_unique<DebugToolDrivers::XplainedNano>();
                    }
                },
                {
                    "curiosity-nano",
                    [] {
                        return std::make_unique<DebugToolDrivers::CuriosityNano>();
                    }
                },
            };

            return mapping;
        }

        /**
         * Constructs a mapping of supported target names to lambdas. The lambdas should instantiate and return an
         * instance to the appropriate Target class.
         *
         * @return
         */
        static auto getSupportedTargets() {
            static auto mapping = std::map<std::string, std::function<std::unique_ptr<Targets::Target>()>>();

            if (mapping.empty()) {
                mapping = {
                    {
                        "avr8",
                        [] {
                            return std::make_unique<Targets::Microchip::Avr::Avr8Bit::Avr8>();
                        }
                    },
                };

                // Include all targets from AVR8 target description files
                auto avr8PdMapping =
                    Targets::Microchip::Avr::Avr8Bit::TargetDescription::TargetDescriptionFile::getTargetDescriptionMapping();

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
                                [targetName, targetSignatureHex] {
                                    return std::make_unique<Targets::Microchip::Avr::Avr8Bit::Avr8>(
                                        targetName,
                                        Targets::Microchip::Avr::TargetSignature(targetSignatureHex)
                                    );
                                }
                            });
                        }
                    }
                }
            }

            return mapping;
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
        void setThreadStateAndEmitEvent(ThreadState state) {
            this->setThreadState(state);
            this->eventManager.triggerEvent(
                std::make_shared<Events::TargetControllerThreadStateChanged>(state)
            );
        }

        /**
         * Because the TargetController hogs the thread, this method must be called in a dedicated thread.
         */
        void startup();

        /**
         * Installs Bloom's udev rules on user's machine. Rules are copied from build/Distribution/Resources/UdevRules
         * to /etc/udev/rules.d/. This method will report an error if Bloom isn't running as root (as root privileges
         * are required for writing to files in /etc/udev).
         */
        static void checkUdevRules();

        /**
         * Exit point - must be called before the TargetController thread is terminated.
         *
         * Handles releasing the hardware among other clean-up related things.
         */
        void shutdown();

        /**
         * Puts the TargetController into the suspended state.
         *
         * In this state, the hardware is released and the TargetController will only handle a subset of events.
         */
        void suspend();

        /**
         * Wakes the TargetController from the suspended state.
         */
        void resume();

        /**
         * Establishes a connection with the debug tool and target. Prepares the hardware for a debug session.
         */
        void acquireHardware();

        /**
         * Attempts to gracefully disconnect from the debug tool and the target. All control of the debug tool and
         * target will cease.
         */
        void releaseHardware();

        /**
         * Extracts address ranges and groups target register descriptors.
         */
        void loadRegisterDescriptors();

        /**
         * Resolves the descriptors of all target registers found within the given address range and memory type.
         *
         * @param startAddress
         * @param endAddress
         * @param memoryType
         * @return
         */
        Targets::TargetRegisterDescriptors getRegisterDescriptorsWithinAddressRange(
            std::uint32_t startAddress,
            std::uint32_t endAddress,
            Targets::TargetMemoryType memoryType
        );

        /**
         * Should fire any events queued on the target.
         */
        void fireTargetEvents();

        /**
         * When the TargetController fails to handle an event, a TargetControllerErrorOccurred event is emitted, with
         * a correlation ID matching the ID of the event that triggered the handler.
         *
         * @param correlationId
         * @param errorMessage
         */
        void emitErrorEvent(int correlationId, const std::string& errorMessage);

        Targets::TargetDescriptor& getTargetDescriptor();

        /**
         * Invokes a shutdown.
         *
         * @param event
         */
        void onShutdownTargetControllerEvent(const Events::ShutdownTargetController& event);

        /**
         * Reports the current state of the TargetController.
         *
         * @param event
         */
        void onStateReportRequest(const Events::ReportTargetControllerState& event);

        /**
         * Obtains a TargetDescriptor from the target and includes it in a TargetDescriptorExtracted event.
         *
         * @param event
         */
        void onExtractTargetDescriptor(const Events::ExtractTargetDescriptor& event);

        /**
         * Will hold the target stopped at it's current state.
         *
         * @param event
         */
        void onDebugSessionStartedEvent(const Events::DebugSessionStarted& event);

        /**
         * Will simply kick off execution on the target.
         *
         * @param event
         */
        void onDebugSessionFinishedEvent(const Events::DebugSessionFinished& event);

        /**
         * Will attempt to stop execution on the target and emit a TargetExecutionStopped event.
         *
         * @param event
         */
        void onStopTargetExecutionEvent(const Events::StopTargetExecution& event);

        /**
         * Will attempt to step execution on the target and emit a TargetExecutionResumed event.
         *
         * @param event
         */
        void onStepTargetExecutionEvent(const Events::StepTargetExecution& event);

        /**
         * Will attempt to resume execution on the target and emit a TargetExecutionResumed event.
         *
         * @param event
         */
        void onResumeTargetExecutionEvent(const Events::ResumeTargetExecution& event);

        /**
         * Will attempt to read the requested registers and emit a RegistersRetrievedFromTarget event.
         *
         * @param event
         */
        void onReadRegistersEvent(const Events::RetrieveRegistersFromTarget& event);

        /**
         * Will attempt to write the specified register values and emit a RegistersWrittenToTarget event.
         *
         * @param event
         */
        void onWriteRegistersEvent(const Events::WriteRegistersToTarget& event);

        /**
         * Will attempt to read memory from the target and include the data in a MemoryRetrievedFromTarget event.
         *
         * @param event
         */
        void onReadMemoryEvent(const Events::RetrieveMemoryFromTarget& event);

        /**
         * Will attempt to write memory to the target. On success, a MemoryWrittenToTarget event is emitted.
         *
         * @param event
         */
        void onWriteMemoryEvent(const Events::WriteMemoryToTarget& event);

        /**
         * Will attempt to set the specific breakpoint on the target. On success, the BreakpointSetOnTarget event will
         * be emitted.
         *
         * @param event
         */
        void onSetBreakpointEvent(const Events::SetBreakpointOnTarget& event);

        /**
         * Will attempt to remove a breakpoint at the specified address, on the target. On success, the
         * BreakpointRemovedOnTarget event is emitted.
         *
         * @param event
         */
        void onRemoveBreakpointEvent(const Events::RemoveBreakpointOnTarget& event);

        /**
         * Will update the program counter value on the target. On success, a ProgramCounterSetOnTarget event is
         * emitted.
         *
         * @param event
         */
        void onSetProgramCounterEvent(const Events::SetProgramCounterOnTarget& event);

        /**
         * Will automatically fire a target state update event.
         * @TODO: get rid of this - Insight should request this itself.
         *
         * @param event
         */
        void onInsightStateChangedEvent(const Events::InsightThreadStateChanged& event);

        /**
         * Will attempt to obtain the pin states from the target. Will emit a TargetPinStatesRetrieved event on success.
         *
         * @param event
         */
        void onRetrieveTargetPinStatesEvent(const Events::RetrieveTargetPinStates& event);

        /**
         * Will update a pin state for a particular pin. Will emit a TargetPinStatesRetrieved with the new pin
         * state, on success.
         *
         * @param event
         */
        void onSetPinStateEvent(const Events::SetTargetPinState& event);

        /**
         * Will retrieve the current stack pointer from the target. Will emit a StackPointerRetrievedFromTarget event
         * containing the retrieved stack pointer value.
         *
         * @param event
         */
        void onRetrieveStackPointerEvent(const Events::RetrieveStackPointerFromTarget& event);
    };
}
