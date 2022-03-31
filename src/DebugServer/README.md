## DebugServer

The DebugServer component exposes an interface to the connected target, for third-party programs such as IDEs. The 
DebugServer run a dedicated thread. The entry point is `DebugServerComponent::run()`. Bloom's main thread will start 
the DebugServer thread once the TargetController has been started. See `Applciation::startDebugServer()` for more.

The DebugServer is designed to accommodate numerous server implementations. The interface exposed by the server is 
implementation-defined. For example, the [AVR GDB server](./GdbRsp/AvrGdb/AvrGdbRsp.hpp) exposes an interface to the 
connected AVR target, by implementing the 
[GDB Remote Serial Protocol](https://sourceware.org/gdb/onlinedocs/gdb/Remote-Protocol.html), over a TCP/IP connection.
Each server must implement the interface defined in the [ServerInterface class](./ServerInterface.hpp).

At startup, the DebugServer will select the appropriate server implementation, based on the user's project 
configuration (bloom.json - see [`DebugServerConfig`](../ProjectConfig.hpp)). Each server implementation is mapped to a 
config name, which is to be used in the project configuration file. For the mapping, see 
`DebugServerComponent::getAvailableServersByName()`. After initialising the server (via `ServerInterface::init()`), 
control of the DebugServer thread will then be handed over to the server implementation (via `ServerInterface::run()`). 
For more on this, see `DebugServerComponent::startup()` and `DebugServerComponent::run()`.

#### Servicing events

During startup, the DebugServer will register event handlers for certain events. Once control of the DebugServer thread
has been handed over to the selected server implementation, the server must ensure that any incoming events are 
processed ASAP. How this is done is implementation-defined. A reference to the DebugServer's event listener 
(`DebugServerComponent::eventListener`) can be passed to the server if need be (see 
`DebugServerComponent::getAvailableServersByName()` for an example of this).

The server implementation does not have to service events - it can simply return from `ServerInterface::run()` upon 
an event being triggered. Returning from that function will allow control of the thread to be handed back to 
`DebugServerComponent::run()`, where any pending events will be processed. Afterwards, if the DebugServer is still 
running (it hasn't received an event which triggers a shutdown), the `ServerInterface::run()` function will be called 
again, and control will be handed back to the server implementation. The AVR GDB server implementation employs this 
method to ensure that events are processed ASAP. See the relevant documentation for more.

### Server implementations

Currently, there is only one server implementation. Other may be added upon request.

Note: If you're considering adding a new implementation yourself, please discuss this with me (Nav) before you begin.
Creating a new server implementation is not an easy or quick job.

| Server Name    | Brief Description | Documentation |
|----------------|-------------------|---------------|
| AVR GDB Server | An implementation of the GDB Remote Serial Protocol over TCP/IP.      | To follow       |
