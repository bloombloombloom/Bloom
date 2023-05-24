## Bloom

Bloom can be downloaded at https://bloom.oscillate.io/download.

Bloom is a debug interface for embedded systems development on GNU/Linux. This is the official repository for Bloom's
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
It exposes an interface to the connected hardware via a command-response mechanism. The TargetController runs on a
dedicated thread. See the [TargetController documentation](./src/TargetController/README.md) and source code in
src/TargetController/ for more.

##### DebugServer
The DebugServer exposes an interface to the connected target, for third-party programs such as GDB and IDEs (GDB
front-ends). Currently, Bloom only supports one server - the AVR GDB RSP server. With this server, any AVR compatible
version of GDB (or any IDE with GDB RSP support) can interface with Bloom and thus the connected AVR target. The
DebugServer runs on a dedicated thread. See the
[DebugServer documentation](./src/DebugServer/README.md) and source code in src/DebugServer/ for more.

##### Insight
Insight is a graphical user interface that provides insight into the connected target. It presents the target's
memories, GPIO pin states & registers, along with the ability to manipulate them. Insight occupies Bloom's main thread
and employs several worker threads for background tasks. Unlike other components within Bloom, Insight relies heavily
on the Qt framework for its GUI capabilities and other useful utilities. See source code in src/Insight/ for more.

##### SignalHandler
The SignalHandler is responsible for handling any UNIX signals issued to Bloom. It runs on a dedicated thread. All
other threads within Bloom do not accept any UNIX signals.
See source code in src/SignalHandler/ for more.

---

### Building Bloom from source

> Bloom is typically distributed via binary packages, which can be downloaded via the
> [downloads page](https://bloom.oscillate.io/download). These packages contain a prebuilt binary of Bloom, as well as
> some of its dependencies (dynamically linked libraries). These dependencies are distributed with Bloom because they
> work well with it. This is known because countless hours have been spent, testing them, on many Linux distros. If you
> choose to use a binary package, you can be fairly confident that you won't run into any compatability issues.
>
> However, if you choose to build Bloom from source, it will use your system's shared libraries, which may be of a
> different version to the one that was tested against. You will be more likely to face compatability issues.
>
> In an ideal world, using a shared library with a different minor version won't break anything, but this is not always
> the reality.

#### Dependencies

To compile Bloom, the following dependencies must be resolved:

The accompanying package names are from the Debian (APT) package repositories - package names will vary across package 
repositories.

- CMake version 3.22 or later
- G++10 or later
- libusb v1.0 (`libusb-1.0-0-dev`)
- libhidapi (0.11.2 or later) (`libhidapi-dev`)
- yaml-cpp (version 0.7.0 or later) (`libyaml-cpp-dev`)
- libprocps (`libprocps-dev`)
- PHP CLI version 8 or later, with the xml extension (`php8.0-cli`, `php8.0-xml`)
- Qt Version 6.2.4 or later (see note below)

When installing Qt, it's best to install via the Qt installer: https://www.qt.io/download

You may also need to install mesa-common-dev and libglu1-mesa-dev (Qt dependencies):

If you already have another version of Qt installed, you may need to temporarily point qtchooser to the right place
(unless you can figure out another way to make cmake use the right Qt binaries (specifically, the moc)):
```
# Set the appropriate paths to your Qt installation in here. You may want to backup this file incase you wish to revert the changes
# You may need to change the path below
sudo nano /usr/lib/x86_64-linux-gnu/qt-default/qtchooser/default.conf
```

#### Example build

```shell
# Build Bloom in [SOME_BUILD_DIR]
mkdir [SOME_BUILD_DIR];
cd [SOME_BUILD_DIR];

cmake [PATH_TO_BLOOM_SOURCE] -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=[PATH_TO_QT_INSTALLATION]/gcc_64/;
cmake --build ./;

# Install Bloom to /opt/bloom
sudo cmake --install ./;

# OR, install Bloom to [SOME_OTHER_INSTALLATION_DIR]
sudo cmake --install ./ --prefix [SOME_OTHER_INSTALLATION_DIR];
```
#### Other notes on building from source:

- To specify the compiler path, use `-DCMAKE_CXX_COMPILER=...`:
  ```
  cmake [PATH_TO_BLOOM_SOURCE] -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=[PATH_TO_QT_INSTALLATION]/gcc_64/ -DCMAKE_CXX_COMPILER=/usr/bin/g++-10;
  ```
- If Qt's shared objects cannot be found when running Bloom, you can either:
  1. Set `LD_LIBRARY_PATH` before running Bloom: `export LD_LIBRARY_PATH=[PATH_TO_QT_INSTALLATION]/gcc_64/lib;` OR:
  2. Update Bloom's RUNPATH (with a tool like `patchelf`)
- Once you've installed Bloom, you'll need to create a symlink to Bloom's binary, in `/usr/bin/`, to run `bloom` without
  having to supply the full path: `sudo ln -s /opt/bloom/bin/bloom /usr/bin/;`
- If you're installing on Ubuntu 20.04 or older, you may need to move the installed udev rules, as they're expected
  to reside in `/lib/udev/rules.d` on those systems. Move them via: `sudo mv /usr/lib/udev/rules.d/99-bloom.rules /lib/udev/rules.d/;`

### Excluding Insight at build time

The Insight component can be excluded at build time, via the `EXCLUDE_INSIGHT` flag.
You'll still need to satisfy the Qt dependency on the build machine, as other parts of Bloom use Qt, but you'll no
longer need to install GUI dependencies on every machine you intend to run Bloom on. Just build it once and distribute
the binary, along with the necessary shared objects.

To identify the necessary shared objects, copy the binary to one of those machines and run `ldd [PATH_TO_BLOOM_BINARY]`.
That will output a list of all the dependencies that `ld` couldn't satisfy - those will be what you need to distribute
(or manually install on each machine).

#### Example build excluding Insight

```shell
# Build Bloom in [SOME_BUILD_DIR]
mkdir [SOME_BUILD_DIR];
cd [SOME_BUILD_DIR];

cmake [PATH_TO_BLOOM_SOURCE] -DEXCLUDE_INSIGHT=1 -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=[PATH_TO_QT_INSTALLATION]/gcc_64/;
cmake --build ./;

...
```
