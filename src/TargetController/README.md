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
mechanism. Instead, they use the `TargetControllerConsole` class, which encapsulates the command-response mechanism and
provides a simplified means for other components to interact with the connected hardware. For more, see
[The TargetControllerConsole class](#the-targetcontrollerconsole-class) section below.

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

const auto& data = readmemoryResponse->data;
```

`readMemoryResponse` will be of type `std::unique_ptr<TargetController::Responses::TargetMemoryRead>`.

The `CommandManager::sendCommandAndWaitForResponse<CommandType>(std::unique_ptr<CommandType> command, std::chrono::milliseconds timeout)`
member function is a template function. It will issue the command to the TargetController and wait for a response, or
until a timeout has been reached. Because `CommandManager::sendCommandAndWaitForResponse()` is a template function, it
is able to resolve the appropriate response type at compile-time (see the `SuccessResponseType` alias in some command
classes). If the TargetController responds with an error, or the timeout is reached,
`CommandManager::sendCommandAndWaitForResponse()` will throw an exception.

#### The TargetControllerConsole class

The `TargetControllerConsole` class encapsulates the TargetController's command-response mechanism and provides a
simplified means for other components to interact with the connected hardware. Iterating on the example above, to read
memory from the target:

```c++
auto tcConsole = TargetController::TargetControllerConsole();

const auto data = tcConsole.readMemory(
    someMemoryType, // Flash, RAM, EEPROM, etc
    someStartAddress,
    someNumberOfBytes
);
```

The `TargetControllerConsole` class does not require any dependencies at construction. It can be constructed in
different threads and used freely to gain access to the connected hardware.

All components within Bloom should use the `TargetControllerConsole` class to interact with the connected hardware. They
**should not** directly issue commands via the [`Bloom::TargetController::CommandManager`](./CommandManager.hpp) unless
there is a very good reason to do so.
`

TODO: Cover debug tool & target drivers.
