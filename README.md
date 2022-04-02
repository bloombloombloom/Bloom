## Bloom

Bloom can be downloaded at https://bloom.oscillate.io/download.

Bloom is a debug interface for embedded systems development on Linux. This is the official repository for Bloom's 
source code. For information on how to use Bloom, please visit https://bloom.oscillate.io.

Bloom implements a number of user-space device drivers, enabling support for many debug tools (such as the Atmel-ICE, 
Power Debugger, MPLAB SNAP, etc). Bloom exposes an interface to the connected target, via a GDB RSP server. This allows
any IDE with GDB RSP client capabilities to interface with Bloom and gain full access to the target.

Currently, Bloom only supports AVR8 targets from Microchip. Bloom was designed to accommodate targets from different 
families and architectures. Support for other target families will be considered as and when requested.

### License
Bloom is released under the LGPLv3 license. See LICENSE.md

---

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
Insight is a graphical user interface that provides insight into the connected target. It presents the target's 
memories, GPIO pin states & registers, along with the ability to manipulate them. Insight occupies Bloom's main thread 
and employs a single worker thread for background tasks. Unlike other components within Bloom, Insight relies heavily 
on the Qt framework for its GUI capabilities and other useful utilities. See source code in src/Insight/ for more.

##### SignalHandler
The SignalHandler is responsible for handling any UNIX signals issued to Bloom. It runs on a dedicated thread. All
other threads within Bloom do not accept any UNIX signals.
See source code in src/SignalHandler/ for more.

#### Inter-component communication
The components described above interact with each other using an event-based mechanism. More on this to follow.

---

### Compiling Bloom
To compile Bloom, the following dependencies must be resolved. The accompanying installation commands require support 
for apt-get.

#### CMake version 3.22 or later:
This can be installed via `sudo apt-get install cmake`, provided the appropriate version is available in your OS package
repositories. Otherwise, you'll need to download CMake from the official website.

#### G++10 or later
Bloom uses features that are only available in C++20. G++10 is (likely) the minimum version Bloom will compile with.
Also, build-essential (`sudo apt-get install build-essential`).

#### libusb v1.0 & libhidapi
`sudo apt-get install libusb-1.0-0-dev libhidapi-dev`

#### PHP version 8 or later, with the xml extension
Some of Bloom's build scripts are written in PHP.

```
sudo apt-get install software-properties-common;
sudo add-apt-repository ppa:ondrej/php;
sudo apt-get install php8.0-cli php8.0-xml;
```

#### Qt Version 6.1.2 or later
It's best to install this via the Qt installer: https://www.qt.io/download

You may also need to install mesa-common-dev and libglu1-mesa-dev (Qt dependencies):
`sudo apt install mesa-common-dev libglu1-mesa-dev`

If you already have another version of Qt installed, you may need to temporarily point qtchooser to the right place
(unless you can figure out another way to make cmake use the right Qt binaries (specifically, the moc)):
```
# Set the appropriate paths to your Qt installation in here. You may want to backup this file incase you wish to revert the changes
# You may need to change the path below
sudo nano /usr/lib/x86_64-linux-gnu/qt-default/qtchooser/default.conf
```

#### Notes on compiling:

- If CMake fails to find the Qt packages, you may need to tell it where to look:
`export CMAKE_PREFIX_PATH=/path/to/Qt-installation/6.1.2/gcc_64/`
- Use the build directory build/cmake-build-debug, when generating the build system for the debug build as it's already 
  gitingored. (You'll have to create it)
- Use the build directory build/cmake-build-release, when generating the build system for the release build as it's 
  already gitingored. (You'll have to create it)
- To generate the build system with cmake (for debug build)
  ```
  # For release build, change cmake-build-debug to cmake-build-release and -DCMAKE_BUILD_TYPE=Debug to -DCMAKE_BUILD_TYPE=Release
  # You may also need to change the path to the compiler
  # You may also need to supply an absolute path to the source (cmake gets a bit weird about this, sometimes)
  cd /path/to/Bloom/build/cmake-build-debug/;
  cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=/usr/bin/g++-10 ../../;
  ```
- To build Bloom (debug):
  ```
  cd /path/to/Bloom;
  cmake --build ./build/cmake-build-debug --target Bloom;
  ```
- To run the clean target:
  ```
  cd /path/to/Bloom;
  cmake --build ./build/cmake-build-debug --target clean;
  ```
- To use Gammaray for GUI debugging, be sure to build Bloom with the debug configuration. Your local installation of
  Gammaray will likely be incompatible with the distributed Qt binaries, which ld will use if you've built with the
  release config. Building with the debug config will disable the RPATH and prevent Qt from loading any plugins from 
  the distribution directory. NOTE: Since upgrading to Qt6, Gammaray has not been of much use. It doesn't currently
  support Qt6, but I've tried building the `work/qt6-support` branch from https://github.com/KDAB/GammaRay and it 
  *kind of* works. It crashes a lot.

More documentation to follow.
