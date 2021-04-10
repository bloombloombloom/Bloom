## Bloom

** First Beta release to be published by end of April 2021 **

Bloom is a debug interface for embedded systems development on Linux. This is the official repository for Bloom's 
source code. For information on how to use Bloom, please visit https://bloom.oscillate.io.

Bloom implements a number of user-space device drivers, enabling support for many debug tools (such as the Atmel-ICE, 
the Power Debugger and the MPLAB SNAP). Bloom exposes an interface to the connected target, via a GDB 
RSP server. This allows any IDE with GDB RSP client capabilities to interface with Bloom and gain full
access to the target.

Currently, Bloom only supports AVR8 targets from Microchip. Bloom was designed to accommodate targets from different 
families and architectures. Support for other target families will be considered as and when requested.

### License
Bloom is released under the LGPLv3 license. See LICENSE.txt

### Bloom Architecture

Bloom is a multithreaded event-driven program written in C++. It consists of four components:

- TargetController
- DebugServer
- Insight
- SignalHandler

##### TargetController
The TargetController possesses full control of the connected debug tool and target. Execution of user-space 
device drivers takes place here. All interaction with the connected hardware goes through the TargetController.
It exposes an interface to the connected hardware via events. The TargetController runs on a dedicated thread.
See source code in src/TargetController/ for more.

##### DebugServer
The DebugServer exposes an interface to the connected target, for third-party programs such as IDEs. Currently, Bloom
only supports one DebugServer - the GDB RSP server. With this server, any IDE with GDB RSP support can interface with
Bloom and thus the connected target. The DebugServer runs on a dedicated thread.
See source code in src/DebugServer/ for more.

##### Insight
Insight is a graphical user interface that provides insight of the target's GPIO pin states. It also enables GPIO
pin manipulation. Insight occupies Bloom's main thread and employs a single worker thread for background tasks. 
Unlike other components within Bloom, Insight relies heavily on the Qt framework for its GUI capabilities and 
other useful utilities.
See source code in src/Insight/ for more.

##### SignalHandler
The SignalHandler is responsible for handling any UNIX signals issued to Bloom. It runs on a dedicated thread. All
other threads within Bloom do not accept any UNIX signals.
See source code in src/SignalHandler/ for more.

#### Inter-component communication
The components described above interact with each other using an event-based mechanism.

More documentation to follow.
