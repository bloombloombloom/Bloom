## Bloom

Bloom can be downloaded at https://bloom.oscillate.io/download.

Bloom is a debug interface for embedded systems development on GNU/Linux. This is the official repository for Bloom's
source code. For information on how to use Bloom, please visit https://bloom.oscillate.io.

### License
Bloom is released under the LGPLv3 license. See LICENSE.md

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
- G++13 or later
- libusb v1.0 (`libusb-1.0-0-dev`)
- libhidapi (0.11.2 or later) (`libhidapi-dev`)
- yaml-cpp (version 0.7.0 or later) (`libyaml-cpp-dev`)
- libprocps (`libprocps-dev`)
- PHP CLI version 8.3 or later, with the xml and multibyte string extensions (`php8.3-cli`, `php8.3-xml`,
  `php8.3-mbstring`)
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
- We do **not** modify the RUNPATH of release builds. This means that running your release build, directly, may fail,
  due to missing Qt shared libraries. We create an invocation script (`[INSTALL_DIR]/bin/bloom.sh`) upon installation,
  which will set the `LD_LIBRARY_PATH` environment variable. You'll need to use that script to run a release build
  (unless all of your shared libraries can be found in the typical locations).
- For debug builds, we **do** modify the RUNPATH (as it's easier for me, during development), but you should note that
  this is currently hardcoded to point to my Qt installation. See the root CMakeLists.txt for more.
- Once you've installed Bloom, you'll need to create a symlink to Bloom's invocation script, in `/usr/bin/`, to run
  `bloom` without having to supply the full path: `sudo ln -s /opt/bloom/bin/bloom.sh /usr/bin/bloom;`
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
