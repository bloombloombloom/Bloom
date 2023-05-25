## TargetController

The TargetController component possesses full control of the connected hardware (debug tool and target). Execution of
user-space device drivers takes place here. All interactions with the connected hardware go through the
TargetController. It runs on a dedicated thread (see `Applciation::startTargetController()`). The source code for the
TargetController component can be found in src/TargetController. The entry point is `TargetControllerComponent::run()`.

### Interfacing with the TargetController - The command-response mechanism

Other components within Bloom can interface with the TargetController via the provided command-response mechanism.
Simply put, when another component within Bloom needs to interact with the connected hardware, it will send a command to
the TargetController, and wait for a response. The TargetController will action the command and deliver the necessary
response.

All TargetController commands can be found in [src/TargetController/Commands](./Commands), and are derived from the
[`Bloom::TargetController::Commands::Command`](./Commands/Command.hpp) base class. Responses can be found in
[src/TargetController/Responses](./Responses), and are derived from the
[`Bloom::TargetController::Responses::Response`](./Responses/Response.hpp) base class.

**NOTE:** Components within Bloom do not typically concern themselves with the TargetController command-response
mechanism. Instead, they use the `TargetControllerService` class, which encapsulates the command-response mechanism and
provides a simplified means for interaction with the connected hardware. For more, see
[The TargetControllerService class](#the-TargetControllerService-class) section below.

Commands can be sent to the TargetController via the [`Bloom::TargetController::CommandManager`](./CommandManager.hpp)
class.

For example, to read memory from the connected target, we would send the
[`Bloom::TargetController::Commands::ReadTargetMemory`](./Commands/ReadTargetMemory.hpp) command:

```c++
auto tcCommandManager = TargetController::CommandManager();

auto readMemoryCommand = std::make_unique<TargetController::Commands::ReadTargetMemory>(
    someMemoryType, // Flash, RAM, EEPROM, etc
    someStartAddress,
    someNumberOfBytes
);

const auto readMemoryResponse = tcCommandManager.sendCommandAndWaitForResponse(
    std::move(readMemoryCommand),
    std::chrono::milliseconds(1000) // Response timeout
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
const auto tcService = Services::TargetControllerService();

const auto data = tcService.readMemory(
    someMemoryType, // Flash, RAM, EEPROM, etc
    someStartAddress,
    someNumberOfBytes
);
```

The `TargetControllerService` class does not require any dependencies at construction. It can be constructed in
different threads and used freely to gain access to the connected hardware, from any component within Bloom.

All components within Bloom should use the `TargetControllerService` class to interact with the connected hardware. They
**should not** directly issue commands via the `Bloom::TargetController::CommandManager`, unless there is a very good
reason to do so.

### Programming mode

When a component needs to write to the target's program memory, it must enable programming mode on the target. This can
be done by issuing the `EnableProgrammingMode` command to the TargetController (see
`TargetControllerService::enableProgrammingMode()`). Once programming mode has been enabled, the TargetController will
reject any subsequent commands that involve debug operations (such as `ResumeTargetExecution`, `ReadTargetRegisters`,
etc), until programming mode has been disabled.

The TargetController will emit `ProgrammingModeEnabled` and `ProgrammingModeDisabled` events when it enables/disables
programming mode. Components should listen for these events to ensure that they disable any means for the user to trigger
debugging operations whilst programming mode is enabled. For example, the Insight component will disable much of its
GUI components when programming mode is enabled.

It shouldn't be too much of a problem if a component attempts to perform a debug operation on the target whilst
programming mode is enabled, as the TargetController will just respond with an error. But still, it would be best to
avoid doing this where possible.

---

TODO: Cover debug tool & target drivers.
