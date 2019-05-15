# WMIT
Warzone Model Import Tool

Import and export .WZM, [.PIE](https://github.com/Warzone2100/warzone2100/blob/master/doc/PIE.md), and .OBJ files.

For use with: [Warzone 2100](https://github.com/Warzone2100/warzone2100)


## Latest development builds

[![Build Status](https://travis-ci.com/Warzone2100/WMIT.svg?branch=master)](https://travis-ci.com/Warzone2100/WMIT)
[![Windows Build Status](https://ci.appveyor.com/api/projects/status/1efyn5i3qxuw6ppb/branch/master?svg=true)](https://ci.appveyor.com/project/Warzone2100/wmit/branch/master)

- **Windows** (x86): [**Installer**](https://ci.appveyor.com/api/projects/Warzone2100/WMIT/artifacts/wmit-master_x86_installer.zip?branch=master&pr=false&job=Environment%3A%20APPVEYOR_BUILD_WORKER_IMAGE%3DVisual%20Studio%202017%2C%20WZ_JOB_ID%3Drelease_x86)
- **Windows** (x64): [**Installer**](https://ci.appveyor.com/api/projects/Warzone2100/WMIT/artifacts/wmit-master_x64_installer.zip?branch=master&pr=false&job=Environment%3A%20APPVEYOR_BUILD_WORKER_IMAGE%3DVisual%20Studio%202017%2C%20WZ_JOB_ID%3Drelease_x64)


## How to build

### Getting the Source

Clone the Git repo:
  ```
  git clone https://github.com/Warzone2100/WMIT.git
  cd WMIT
  git submodule update --init --recursive
  ```
  > Note: Initializing submodules is required.

### Windows

* Prerequisites
   * **Visual Studio 2017+**
   * **CMake 3.11+** (https://cmake.org/)
   * **QT 5 (ideally, Qt 5.9+)** (https://www.qt.io/)
   * NSIS _[optional] (used to generate the installer package)_ (https://nsis.sourceforge.io/Download)

* **Building:**
   * Open the appropriate pre-configured command prompt from the "Visual Studio 2017" folder in your Start Menu:
      * For a 32-bit (x86) build: "x86 Native Tools Command Prompt for VS 2017"
      * For a 64-bit (x64) build: "x64 Native Tools Command Prompt for VS 2017"
   * Change directory to the WMIT repo directory:
      * `cd C:\src\WMIT` _(substitute the appropriate path to the repo on your system)_
   * Run the `configure_win` script using Powershell:
      * `powershell .\configure_win.ps1`
   * If successful, you should now have a **wmit.sln** Visual Studio project in a new `build/<platform>` folder inside the repo.
   * Simply open the **wmit.sln** file in Visual Studio.
   * Or build from the command-line using something like:
      * `msbuild build/x86/wmit.vcxproj /p:Configuration=Release` (builds wmit.exe)
      * `msbuild build/x86/PACKAGE.vcxproj /p:Configuration=Release` (builds the installer package)

### Linux

* Prerequisites
   * **GCC / Clang**
   * **CMake 3.5+** (https://cmake.org/)
   * **QT 5 (ideally, Qt 5.9+)** (https://www.qt.io/)
* Examples of installing prerequisites:
   * Ubuntu 18.04:
      * `sudo apt-get install git cmake build-essential ninja-build automake qt5-default`
   * Fedora 29:
      * `sudo dnf install git cmake gcc gcc-c++ ninja-build make qt5-devel mesa-libGLU-devel`
* **Building**:
   * Change directory to the WMIT repo directory
      * `cd WMIT`
   * Configure (builds dependencies and generates the Makefile)
      * `./configure_linux.sh`
   * Build
      * `cmake --build build`

### macOS

* Prerequisites
   * **Xcode 8+**
   * **CMake 3.11+** (https://cmake.org/)
   * **QT 5.9.1+** (https://www.qt.io/)
* **Building:**
   * Change directory to the WMIT repo directory
      * `cd WMIT`
   * Configure (builds dependencies and generates an Xcode project)
      * `PATH=/Qt/5.9.7/clang_64/bin:$PATH QT5_DIR="/Qt/5.9.7/" ./configure_macOS.sh`
        * (Substitute the appropriate PATH to Qt5's bin folder and the main Qt5 install folder.)
   * Build
      * Simply open the `build/wmit.xcodeproj`, and build the `wmit` target / scheme.
