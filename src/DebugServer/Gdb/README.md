## GDB Remote Serial Protocol (RSP) debug server

The GDB RSP debug server implements GDB's
[Remote Serial Protocol](https://sourceware.org/gdb/onlinedocs/gdb/Remote-Protocol.html) over a TCP/IP connection.
With this debug server, users can perform debugging operations on the connected target via GDB. This can be done via
GDB's command line interface, or via an IDE that supports remote GDB capabilities (See
[GDB's machine interface](https://sourceware.org/gdb/onlinedocs/gdb/GDB_002fMI.html) for more on how GDB is integrated
into IDEs).

The implementation can be found in the `GdbRspDebugServer` class. This class is an abstract class - it does not
implement any target architecture specific functionality.
See ['Target architecture specific functionality'](#target-architecture-specific-functionality) section below for more.

---

### Commands and responses

The objective of the GDB server is to provide GDB with an interface to the connected target, to enable debugging
operations such as memory access and program flow control. The GDB server achieves this by actioning commands sent by
the client (GDB) and returning responses.

Simply put, once GDB has established a connection with the GDB server, it will begin to send commands to the server.
These commands are to be actioned by the GDB server, with the appropriate response being returned, if necessary.
For example, when the GDB user runs the command `b main.cpp:22` in the GDB CLI, instructing GDB to insert a breakpoint
at line 22 in main.cpp, GDB will send a command to the GDB server, requesting that a breakpoint be added at the
appropriate address in program memory. The GDB server will action this command, and then return a response indicating
success or failure.

Commands and responses are delivered in packets. The
[`DebugServer::Gdb::CommandPackets::CommandPacket`](./CommandPackets/CommandPacket.hpp) and
[`DebugServer::Gdb::ResponsePackets::ResponsePacket`](./ResponsePackets/ResponsePacket.hpp) classes are base
classes for these packets. For most GDB commands supported by this server implementation, there is a specific command
packet class that can be found in [/src/DebugServer/Gdb/CommandPackets](./CommandPackets). When the server receives a
command packet from the GDB client, the appropriate (`CommandPacket` derived) object is constructed, which encapsulates
all of the relevant information for the particular command.

Consider the [`DebugServer::Gdb::CommandPackets::SetBreakpoint`](./CommandPackets/SetBreakpoint.hpp) command
packet class:

```c++
class SetBreakpoint: public CommandPacket
{
public:
    /**
     * Breakpoint type (Software or Hardware)
     */
    BreakpointType type = BreakpointType::UNKNOWN;

    /**
     * Address at which the breakpoint should be located.
     */
    std::uint32_t address = 0;

    explicit SetBreakpoint(const RawPacket& rawPacket);

    void handle(DebugSession& debugSession, TargetControllerService& targetControllerService) override;
};
```

The `SetBreakpoint` class consists of two public fields; The `address` (at which the breakpoint is to be set). And the
`type` (of breakpoint to set). During object construction, these two fields are initialised from the raw command packet
(received by the GDB client).

#### Command handling

Upon receiving a command packet from the GDB client, the command must be handled and the appropriate response delivered.
Each command packet class implements a `handle()` member function. This function is called upon receipt of the command
and is expected to handle the command and deliver any necessary response to the client. Two parameters are passed to the
`handle()` member function - a reference to the active `DebugSession` object, and a reference to a
`TargetControllerService` object. The `DebugSession` object provides access to the current connection with the GDB
client, as well as other debug session specific information. The `TargetControllerService` object provides an interface
to the `TargetController`, for any GDB commands that need to interface with the connected target (see the
[TargetController documentation](../../TargetController/README.md) for more on this).

Handling the `SetBreakpoint` command packet:

```c++
void SetBreakpoint::handle(DebugSession& debugSession, TargetControllerService& targetControllerService) {
    /*
     * I know the breakpoint type (this->type) isn't used in here - this is because the current implementation only
     * supports software breakpoints, so we don't do anything with this->type, for now.
     */

    Logger::debug("Handling SetBreakpoint packet");

    try {
        targetControllerService.setBreakpoint(TargetBreakpoint(this->address));
        debugSession.connection.writePacket(OkResponsePacket());

    } catch (const Exception& exception) {
        Logger::error("Failed to set breakpoint on target - " + exception.getMessage());
        debugSession.connection.writePacket(ErrorResponsePacket());
    }
}
```

Some GDB commands are handled in the base `CommandPacket` class (see
[`CommandPacket::handle()`](./CommandPackets/CommandPacket.cpp)). Either these commands were too trivial to justify a
new command packet class, or I was too lazy to create one.

---

### Target architecture specific functionality

Within the source code of GDB, there are some hardcoded concepts that are specific to GDB and a particular target
architecture. For example, the AVR implementation of GDB defines a mapping of AVR registers to GDB register numbers.
The GDB register numbers are just identifiers for AVR registers - they allow GDB to refer to a particular AVR register.

| AVR Register Name                         | GDB Register Number |
|-------------------------------------------|---------------------|
| R0 -> R31 (general purpose CPU registers) | 0 -> 31             |
| Status Register (SREG)                    | 32                  |
| Stack Pointer (SP)                        | 33                  |
| Program Counter (PC)                      | 34                  |

GDB expects the server to be aware of these predefined concepts. This is why the `GdbRspDebugServer` class is an
abstract class - it implements the generic GDB server functionality, but leaves the target architecture specific
functionality to be implemented in derived classes.

#### GDB target descriptor

The [`DebugServer::Gdb::TargetDescriptor`](./TargetDescriptor.hpp) abstract class provides access to any
information that is specific to the target architecture and GDB. For example, the register mapping described above, for
AVR targets, is implemented in [`DebugServer::Gdb::AvrGdb::TargetDescriptor`](./AvrGdb/TargetDescriptor.hpp).
That class is derived from the abstract `TargetDescriptor` class. It implements the AVR specific concepts that the
server is expected to be aware of.
