## DebugServer

The DebugServer component exposes an interface to the connected target, for third-party programs such as IDEs. The 
DebugServer runs on a dedicated thread. The entry point is `DebugServerComponent::run()`. Bloom's main thread will start 
the DebugServer thread once the TargetController has been started. See `Applciation::startDebugServer()` for more.

The DebugServer is designed to accommodate numerous server implementations. The interface exposed by the server is 
implementation-defined. For example, the [AVR GDB server](./Gdb/AvrGdb/AvrGdbRsp.hpp) exposes an interface to the 
connected AVR target, by implementing the 
[GDB Remote Serial Protocol](https://sourceware.org/gdb/onlinedocs/gdb/Remote-Protocol.html), over a TCP/IP connection.
Each server must implement the interface defined in the [`ServerInterface` class](./ServerInterface.hpp).

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

A server implementation does not have to service events - it can simply return from `ServerInterface::run()` upon 
an event being triggered. Returning from that function will allow control of the thread to be handed back to 
`DebugServerComponent::run()`, where any pending events will be processed. Afterwards, if the DebugServer is still 
running (it hasn't received an event which triggers a shutdown), the `ServerInterface::run()` function will be called 
again, and control will be handed back to the server implementation. The AVR GDB server implementation employs this 
method to ensure that events are processed ASAP. See the relevant documentation for more.

---

Any blocking I/O employed by a server implementation can support event interrupts using an 
[`EventNotifier`](../Helpers/EventNotifier.hpp) and [`EpollInstance`](../Helpers/EpollInstance.hpp). 

Key points: 
- The `EventNotifier` class is an RAII wrapper for a Linux 
  [eventfd object](https://man7.org/linux/man-pages/man2/eventfd.2.html). An event can be recorded against the eventfd 
  object via a call to `EventNotifier::notify()`.
- The [`EventListener`](../EventManager/EventListener.hpp) class can accept an `EventNotifier` object via
  `EventListener::setInterruptEventNotifier()`. If an `EventNotifier` has been set on an `EventListener`, the
  `EventListener` will call `EventNotifer::notify()` everytime an event is registered for that listener. 
- The `EpollInstance` class is an RAII wrapper for a Linux 
  [epoll instance](https://man7.org/linux/man-pages/man7/epoll.7.html). It allows us to wait for any activity on a set 
  of file descriptors. File descriptors can be added and removed from the epoll instance via `EpollInstance::addEntry()` 
  and `EpollInstance::removeEntry()`. Calling `EpollInstance::waitForEvent()` will block until there is activity on at 
  least one of the file descriptors, or a timeout has been reached.
  
With an `EventNotifer` and `EpollInstance`, one can perform a blocking I/O operation which can be interrupted by an 
event. For an example of this, see the AVR GDB server implementation - it employs the method described above to allow 
the interruption of blocking I/O operations when an event is triggered. Specifically, 
[`GdbRspDebugServer::waitForConnection()`](./Gdb/GdbRspDebugServer.hpp) or
[`Gdb::Connection::read()`](./Gdb/Connection.hpp).

---

### Server implementations

Currently, there is only one server implementation. Others may be added upon request.

| Server Name    | Brief Description                                                             | Documentation                                     |
|----------------|-------------------------------------------------------------------------------|---------------------------------------------------|
| AVR GDB Server | An AVR-specific implementation of the GDB Remote Serial Protocol over TCP/IP. | [/src/DebugServer/Gdb/README.md](./Gdb/README.md) |

#### Adding new server implementations

If you're considering adding a new server implementation, please discuss this with me (Nav) before you begin. Creating 
a new server implementation is not a trivial task.
