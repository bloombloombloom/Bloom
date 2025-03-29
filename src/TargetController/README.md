## TargetController

The TargetController component possesses full control of the connected hardware (debug tool and target). Execution of
user-space device drivers takes place here. All interactions with the connected hardware go through the
TargetController. It runs on a dedicated thread (see `Applciation::startTargetController()`). The source code for the
TargetController component can be found in src/TargetController. The entry point is `TargetControllerComponent::run()`.

### Lifetime

The TargetController is the second (after the SignalHandler) component to be instantiated at startup, and the second
to last to be destroyed when Bloom shuts down.

The TargetController will outlive both the DebugServer and Insight GUI components.

### Interfacing with the TargetController - The command-response mechanism

Other components within Bloom can interface with the TargetController via the provided command-response mechanism.
Simply put, when another component within Bloom needs to interact with the connected hardware, it will send a command to
the TargetController, and wait for a response. The TargetController will action the command and deliver the necessary
response.

All TargetController commands can be found in [src/TargetController/Commands](./Commands), and are derived from the
[`TargetController::Commands::Command`](./Commands/Command.hpp) base class. Responses can be found in
[src/TargetController/Responses](./Responses), and are derived from the
[`TargetController::Responses::Response`](./Responses/Response.hpp) base class.

**NOTE:** Components within Bloom do not typically concern themselves with the TargetController command-response
mechanism. Instead, they use the `TargetControllerService` class, which encapsulates the command-response mechanism and
provides a simplified means for interaction with the connected hardware. For more, see
[The TargetControllerService class](#the-TargetControllerService-class) section below.

Commands can be sent to the TargetController via the [`TargetController::CommandManager`](./CommandManager.hpp)
class.

For example, to read memory from the connected target, we would send the
[`TargetController::Commands::ReadTargetMemory`](./Commands/ReadTargetMemory.hpp) command:

```c++
auto tcCommandManager = TargetController::CommandManager{};

auto readMemoryCommand = std::make_unique<TargetController::Commands::ReadTargetMemory>(
    addressSpaceDescriptor,
    memorySegmentDecriptor,
    someStartAddress,
    bytesToRead
);

const auto readMemoryResponse = tcCommandManager.sendCommandAndWaitForResponse(
    std::move(readMemoryCommand),
    std::chrono::milliseconds{1000} // Response timeout
);

const auto& data = readMemoryResponse->data;
```

`readMemoryResponse` will be of type `std::unique_ptr<TargetController::Responses::TargetMemoryRead>`.

The `CommandManager::sendCommandAndWaitForResponse<CommandType>(std::unique_ptr<CommandType> command, std::chrono::milliseconds timeout)`
member function is a template function. It will issue the command to the TargetController and wait for a response, or
until a timeout has been reached. Because it is a template function, it is able to resolve the appropriate response
type at compile-time (see the `SuccessResponseType` alias in some command classes). If the TargetController responds
with an error, or the timeout is reached, `CommandManager::sendCommandAndWaitForResponse()` will throw an exception.

#### The TargetControllerService class

The `TargetControllerService` class encapsulates the TargetController's command-response mechanism and provides a
simplified means for other components to interact with the connected hardware. Iterating on the example above, to read
memory from the target:

```c++
const auto tcService = Services::TargetControllerService{};

const auto data = tcService.readMemory(
    addressSpaceDescriptor,
    memorySegmentDecriptor,
    someStartAddress,
    bytesToRead
);
```

The `TargetControllerService` class does not require any dependencies at construction. It can be constructed in
different threads and used freely to gain access to the connected hardware, from any component within Bloom.

All components within Bloom should use the `TargetControllerService` class to interact with the connected hardware. They
**should not** directly issue commands via the `TargetController::CommandManager`, unless there is a very good
reason to do so.

#### Atomic operations

In some instances, we need the TargetController to service a series of commands without any interruptions (servicing of
other commands).

The TargetController allows for operations to be performed within "atomic sessions". Simply put, when the
TargetController starts a new atomic session, any commands that are part of the session will be placed into a dedicated
queue. When an atomic session is active, the TargetController will only process commands in the dedicated queue.
All other commands will be processed once the atomic session has ended.

##### Atomic sessions with the TargetControllerService

The `TargetControllerService` provides an RAII wrapper for atomic sessions.

The `TargetControllerService::makeAtomicSession()` member function returns an `TargetControllerService::AtomicSession`
RAII object, which starts an atomic session with the TargetController, at construction, and ends the session at
destruction. This allows us to perform operations within an atomic session, in an exception-safe manner:

```c++
auto tcService = Services::TargetControllerService{};

{
    const auto atomicSession = tcService.makeAtomicSession();

    /*
     * These operations will take place in the atomic session - the TC will **NOT** service any other commands until
     * these commands have been processed (and the atomic session has ended).
     */
    tcService.writeMemory(...);
    tcService.readMemory(...);
    tcService.getProgramCounter();

    /*
     * Note: The TC does **NOT** support nested atomic sessions. Attempting to start another session in this block will
     * result in an exception being thrown.
     */
    {
        const auto nestedAtomicSession = tcService.makeAtomicSession(); // This will fail - a session is already active.
    }

    /*
     * Also note: When using TargetControllerService::makeAtomicSession(), the returned AtomicSession object is tied to
     * the TargetControllerService object that created it (in this example: tcService).
     *
     * So if you have **another** TargetControllerService object in this block, any operations performed via that
     * object will **NOT** be part of the atomic session, and, they will deadlock the TC. So don't ever do this.
     * You should never need more than one TargetControllerService object in a single block.
     */

    // Don't ever do this.
    auto anotherTcService = Services::TargetControllerService{};

    // These operations will **NOT** be part of the atomic session, and they will cause a deadlock and timeout.
    anotherTcService.writeMemory(...);
    anotherTcService.readMemory(...);

    /*
     * One more thing: The AtomicSession object should **NEVER** outlive the TargetControllerService object that
     * created it.
     *
     * If this happens, the AtomicSession will have a dangling reference, which will result in UB.
     */
}

// At this point, the atomic session will have ended. The TC will now process any other commands in the queue.
tcService.readMemory(...); // Will not be part of the atomic session
```

### Obtaining information on the connected target - target descriptor objects

Most components in Bloom require access to some target information. This information could be related to address spaces,
peripherals, registers, or any other target entity.

In Bloom, target information is held in descriptor objects. These objects are instances of descriptor structs intended
to hold information on a particular target entity. For example, information on a particular address space can be found
in the relevant `TargetAddressSpaceDescriptor` object.

All descriptor structs can be found in [src/Targets/](../Targets).

The `TargetDescriptor` struct is the top-level descriptor struct. Instances of all other descriptor structs can be
accessed via the `TargetDescriptor` struct.

The `GetTargetDescriptor` command returns a const reference to a top-level target descriptor object.
The `TargetControllerService::getTargetDescriptor()` member function can be used to obtain the top-level target
descriptor:

```c++
const auto& targetDescriptor = tcService.getTargetDescriptor();

// Obtain info on the target's "data" address space
const auto& dataAsDescriptor = targetDescriptor.getAddressSpaceDescriptor("data"); // The "data" argument is the address space key
/**
 * - dataAsDescriptor.addressRange.startAddress <- the start address of the address space
 * - dataAsDescriptor.size() <- the size of the address space, in bytes
 * - dataAsDescriptor.endianness <- the endianness of the address space
 *
 * See the TargetAddressSpaceDescriptor struct for more...
 */

// Obtain info on the target's "ADC0" peripheral
const auto& adcPerDescriptor = targetDescriptor.getPeripheralDescriptor("adc0"); // The "adc0" argument is the peripheral key
/**
 * - adcPerDescriptor.name <- the name of the peripheral
 * - adcPerDescriptor.getRegisterGroupDescriptor(...) <- Access a register group within the peripheral
 *
 * See the TargetPeripheralDescriptor struct for more...
 */
```

The copying of descriptor objects is discouraged for two reasons:

- Descriptor objects hold a lot of information, making them expensive to copy
- Descriptor objects can be passed back to the TargetController, in the form of const references, when performing an
  operation on a particular target entity. If the copied object does not outlive the TargetController command, the
  TargetController may attempt to access a dangling reference, resulting in undefined behaviour. For more on this, see
  the [Passing descriptor objects to the TargetController](#passing-descriptor-objects-to-the-targetController) section
  below

The copy constructors and the assignment operators are deleted on all descriptor structs.

#### Passing descriptor objects to the TargetController

For some operations, the TargetController may require the referencing of a particular target entity. For example, to
read memory from the target, the TargetController will need to know which address space and memory segment to read
from. This is why the `ReadTargetMemory` command requires a `const TargetAddressSpaceDescriptor&` and
`const TargetMemorySegmentDescriptor&`. There are many commands that require const references to target descriptor
objects.

It is the invoker's responsibility to ensure that all const references to descriptor objects, passed to the
TargetController, are still valid at the time the TargetController services the command. If the TargetController
encounters a dangling reference, undefined behaviour will occur. See the "master target descriptor" for more.

#### Thread-safe access of the target descriptor - the master target descriptor object

The "master" target descriptor object is simply an instance of the top-level `TargetDescriptor` struct, owned and
managed by the TargetController. The TargetController will return a const reference of the master target descriptor
when servicing the `GetTargetDescriptor` command.

The TargetController will never modify the master target descriptor after creation. There are no mutable members in
any of the target descriptor structs. This means we can access a const reference of the descriptor, from any thread
within Bloom, assuming the thread does not outlive the TargetController.

Because the master target descriptor is owned by the TargetController, it, along with any of its contained objects,
can be passed safely to the TargetController, in any TargetController command.

Both the debug server and Insight GUI components hold a const reference of the master target descriptor, allowing them
to access target information freely.

Do **not** mutate the master target descriptor (via `const_cast` or by any other means). Ever.

### Target state observation

The TargetController provides access to the target's current state, via the `GetTargetState` command, which will return
an instance of the [`TargetState`](../Targets/TargetState.hpp) struct. This struct holds the execution state
(`TargetExecutionState`), the mode (programming/debugging, `TargetMode`), and the program counter.

The `TargetControllerService::getTargetState()` member function should be used to obtain the target's current state:

```c++
const auto targetState = tcService.getTargetState();

if (targetState.executionState == TargetExecutionState::STOPPED) {
    // ...
}
```

#### Real-time, on-demand, thread-safe access to the target's current state - the master target state object

All members of the `TargetState` struct are accessible via atomic operations (that is, all members are of `std::atomic<...>`
type). This means that we can access a single instance of the `TargetState` struct across multiple threads, in a
thread-safe manner.

The "master" target state object is simply an instance of the `TargetState` struct that is owned and managed by the
TargetController (`TargetControllerComponent::targetState`). It holds the current state of the target, at all times.

When servicing the `GetTargetState` command, the TargetController returns a const reference to the master target state
object. This means that, if the caller of `TargetControllerService::getTargetState()` needs real-time, on-demand access
to the target's current state, it can gain this by simply accepting a const reference of the master target state
object:

```c++
const auto& targetState = tcService.getTargetState();

/*
 * In the previous example, we used `const auto targetState = tcService.getTargetState();`, which made a copy of the
 * master TargetState object. That copy would not be managed by the TargetController, and would only hold the state of
 * the target at the point when `tcService.getTargetState()` returned a value.
 *
 * In this example, `targetState` is a const reference to the master TargetState object - it will always hold the
 * target's current state.
 *
 * We can now observe the target's current state, without having to make any more calls to `TargetControllerService::getTargetState()`.
 */

if (targetState.executionState == TargetExecutionState::STOPPED) {
    tcService.resumeTargetExecution();

    /*
     * At this point, targetState.executionState == TargetExecutionState::RUNNING, because the master target state
     * object, which `targetState` references, will have been updated by the TargetController (as a result of the call
     * to `tcService.resumeTargetExecution()` above).
     *
     * This, of course, assumes that the target is still running by the time we get to this assertion...
     */
    assert(targetState.executionState == TargetExecutionState::RUNNING);
}
```

The master target state object can be accessed freely by any other component within Bloom, just as long as the
component doesn't outlive the TargetController.

Many Insight GUI widgets make use of the master target state object, as it allows for immediate access to the target's
current state, without having to bother the TargetController via an InsightWorker task.

#### Target state changed events

When the target state changes, the TargetController will emit a `TargetStateChanged` event. The event object holds
two `TargetState` objects: `TargetStateChanged::newState` and `TargetStateChanged::previousState`. Listeners can use
these to determine what changed:

```c++
void onTargetStateChanged(const Events::TargetStateChanged& event) {
    using Targets::TargetExecutionState;

    if (event.previousState.executionState == event.newState.executionState) {
        // Target execution state has not changed. Probably a mode change
    }

    if (
        event.previousState.executionState == TargetExecutionState::STOPPED
        && event.newState.executionState == TargetExecutionState::RUNNING
    ) {
        // Target has just resumed execution...
    }

    if (
        event.previousState.executionState == TargetExecutionState::STEPPING
        && event.newState.executionState == TargetExecutionState::STOPPED
    ) {
        // Target has just finished stepping...
    }

    // ...
}
```

### Programming mode

When a component needs to write to the target's program memory, it must enable programming mode. This can be done by 
issuing the `EnableProgrammingMode` command to the TargetController (see
`TargetControllerService::enableProgrammingMode()`).

Once programming mode has been enabled, standard debugging operations such as program flow control and RAM access will
become unavailable. The TargetController will reject any commands that involve these operations, until programming mode
has been disabled.

The TargetController will emit a `TargetStateChanged` event when programming mode is enabled/disabled. See 
[`TargetState::mode`](../Targets/TargetState.hpp) for more.

---

TODO: Cover debug tool & target drivers.
