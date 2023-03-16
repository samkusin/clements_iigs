# Clemens IIGS

The Clemens IIgs emulator is an attempt at writing a modern emulator for
the Apple IIgs machine.  Its design focuses on developers versus casual play,
providing tools to debug IIgs applications at runtime. My eventual goal is to provide a
solid Apple IIgs backend and debugger with a functional frontend.

![Zany Golf](https://samirsinha.com/images/zany_small.gif)
![Defender of the Crown](https://samirsinha.com/images/dotc-title.gif)

**NOTE** This software should be considered Alpha quality.  The emulator is generally
stable with some missing features which are listed in prioritized order below.  Of course,
help would be appreciated to take on a couple of these.

* Fast disk emulation
* ROM 01 support (only supports ROM 3
* Serial (Zilog SCC) not implemented
* Pass Boot Diagnostic Test
* AppleTalk / Networking
* Printers
* Monochrome modes

## Contributions

Please note this project is released with a Contributor Code of Conduct. By participating
in this project you agree to abide by its terms.

## Emulation

* Super Hires 640/320 modes with VBL and Scanline IRQs
* Ensoniq, Mockingboard C (without SSI voice) and A2 speaker audio
* WOZ 2.0, 2IMG, PO, DO and DSK support
* 5.25", Apple 3.5" and SmartPort hard drive (from the disk port vs. slot 7) emulation
* Mouse and Keyboard
* Gamepad/Joystick
* ROM 3
* All Apple IIe graphics modes (hires is OK, could be better for some titles)

### Debugging

* Instruction level debugging
* Breakpoints on execution at, read from or write to an address
* Break on IRQ
* Execution trace output and export
* Memory bank dump to file

See [The Plan](https://github.com/samkusin/clemens_iigs/blob/main/docs/Plan.md)
for details on the next release's upcoming features.

The [Documentation](https://github.com/samkusin/clemens_iigs/blob/main/docs/) folder
contains some very rough notes on various systems and technical notes for the IIgs.

For an example of the program trace feature, see this trace for the [ROM 3 boot sequence](https://github.com/samkusin/clemens_iigs/blob/main/docs/rom3_reset_trace.txt) generated by this tool.

This information will be moved to [the Wiki](https://github.com/samkusin/clemens_iigs/wiki/The-Clemens-Apple-IIgs-Emulator) in stages.

## Current Screenshots

![Ultima V](https://samirsinha.com/images/u5a2_small.png)

![GS/OS 5](https://samirsinha.com/images/gsos6_small.png)

## Installation

Both Linux and Windows are supported in this release.  Currently only the Windows
build is available as a binary on the releases page.

### Windows

There is no installer and the application is contained in whole within a single exectuable.
Simply unzip the executable into a directory.  Because this executable is not code signed,
you'll get a warning from the OS when running the binary.  Simply select 'More Info' and
'Run Anyway' if your security policy (and personal comfort level) allows for running untrusted
applications.

An alternative to installing the binary is to build the project locally.

### Linux

Currently only builds from source are supported.  A future version will supply
packages for Debian and possibly other distributions.

## Getting Started

1. Copy a ROM 3 file named `gs_rom_3.rom` to the same directory as the executable
2. Run the executable
3. Download disk images from https://archive.org/details/wozaday or https://mirrors.apple2.org.za/ftp.apple.asimov.net/
4. Import the images using one of the four available drives from the UI on the upper left handed pane.
   1. Slot 5 refers to 3.5" disks
   2. Slot 6 refers to 5.25" disks
5. Once imported, you can select the image by selecting the drop down.  The imported image will appear there.

### Emulator Input

To send input to the emulated machine, click the emulator view.  Mouse input
will be locked to the emulator view.  Keyboard events will also be directed to
the emulator.

* To leave the emulator view, on Windows keyboards press the `Ctrl' + 'Left Alt' + 'Right Alt' keys.
* The F12 Key represents the RESET key.
   * To send a Ctrl-RESET to the emulator, press `Ctrl`+`F12`.
   * To reboot press `Ctrl`+`Alt`+`F12`
* The F1 Key functions as an ESCAPE key if either `Ctrl` or `Alt` are pressed for OS compatibility reasons
   * To open the Desktop Manager/Control Panel, press `Ctrl`+`Right Alt`+`F1`
* On Linux because of how the `Ctrl` and `Alt` + FN keys often map to other system functions:
   * Use the Tux/Super key (usually to the left of your left ALT key) in combination with a number key to mock a FN key
   * Tux - Minus = F11
   * Tux - Equal = F12
   * Tux - Ctrl - Alt - Equal to reboot the system
   * Tux - Ctrl - Equal for CTRL-RESET
   * Tux - Ctrl - Left Alt - 1 to enter the Control Panel

### Terminal Commands

The Terminal is currently the primary console interface available to send commands to the emulator.

Type 'help' to list the available commands.  The most important commands to
remember are:

```
reboot                  - reboots the machine
break                   - breaks execution at the current PC
step                    - step instruction at current PC
trace on                - enables program tracing
trace off, filename.txt - outputs the trace to the specified path
save filename           - saves a snapshot of the current emulator state
load filename           - restores the emulator state to the designated snapshot
```

Please consult [the Wiki](https://github.com/samkusin/clemens_iigs/wiki/The-Clemens-Apple-IIgs-Emulator) for more detailed usage instructions.  The
following can be used to begin working with the emulator.

## Build

Both Linux 64-bit and Windows 10 or greater are supported in this release.  On all platforms, [CMake](https://cmake.org/) is required to build the project.

### Linux

#### Prerequisites

The following packages must be installed to successfully build the binary.

* GCC version 8 or later
* libx11-dev
* libxcursor-dev
* libxi-dev
* mesa-common-dev
* libasound-dev
* uuid-dev

#### Commands

Inside the project directory execute the following.

```
cmake -B build -DCMAKE_BUILD_TYPE=<Debug|RelWithDbgInfo|Release> -DBUILD_TESTING=OFF
cmake --build build --config <Debug|RelWithDbgInfo|Release>
cp build/host/clemens_iigs <your_selected_install_directory>
```

### Windows

#### Commands

Inside the project directory execute the following.

```
cmake -B build -DBUILD_TESTING=OFF
cmake --build build --config <Debug|RelWithDbgInfo|Release>
cp build-run/clemens_iigs/<Config>/clemens_iigs.exe <your_selected_install_directory>
```

## Author's Note

This project is not entirely novel as there are finished emulator projects like KEGS and MAME.

I've open-sourced this project for exposure.  Contributions are welcome if they are made.

The best ways to run Apple IIgs software are:

* From a legitimate upgraded Apple IIgs with a [Floppy Emu](https://www.bigmessowires.com/floppy-emu/)
* A KEGS port available on MacOS or Linux (Windows ports are available though some haven't been updated in a while - GSPort, GSPlus, etc.)
* MAME in the browser https://archive.org/details/softwarelibrary_apple2gs
* Sweet16 on MacOS

So why do we need another emulator.  As mentioned in the first paragraphs of this README, my goal is to provide a solid debugger for running emulated IIGS software.  For example, I hope to replicate or improve upon the fine debugger in AppleWin for Apple II software developers.

The Apple IIgs is an odd system.   It's a precursor to the line of Classic Macinitosh machines that ended with the iMac and later Power PC machines (ADB, IWM, Smartport, color GUI.)  It also marked the end of the Apple II line of computers on which I first learned to program in BASIC and assembly.

It's an 16-bit machine on the surface with a 24-bit addressing space and an 8-bit memory bus.  Seemingly it was underclocked on release for two reasons: Apple II compatibilty and (likely) to not compete with the 680x0 Macintosh line.  The sound capabilities are top-notch for its time.  As my computer lab/teacher back in 1987 said - it's a "Toy Mac".

I've attempted to document in source code original reference material for the various ICs and disk systems emulated in Clemens.  At some point I'll host some source reference materials and link to others.   Finally I've attempted to write this more for code readability vs performance.   There are certainly places where the CPU emulation could be sped up.   If I have time I'll take a look at those.

## Third Party Software

External libraries referenced below are used by the `host` project (the debugging front-end.)  The actual Clemens emulator backend does not have any dependencies beyond the C standard library and the two listed dependencies included in the project.

All projects below have permissive enough licenses (MIT, BSD, Apache) to be distributed by this project.

### Clemens Emulator Library

* [mpack](https://github.com/ludocode/mpack) : A single source file MessagePack implementation written in C in the `external` folder used for serialization of a machine
* [unity](https://github.com/ThrowTheSwitch/Unity) : A C testing framework

### Host

* [stb_truetype.h](https://github.com/nothings/stb/blob/master/stb_truetype.h) For font rendering
* [Fonts](https://www.kreativekorp.com/software/fonts/apple2.shtml) From Kreative Korp for 40/80 column text
* [Dear ImGui](https://github.com/ocornut/imgui) A well-known IMGUI C++ library
* [Dear ImGui Markdown](https://github.com/juliettef/imgui_markdown) ImGui Markdown Implementation
* [Sokol](https://github.com/floooh/sokol) A cross-platform minimal rendering backend
* [fmt](https://github.com/fmtlib/fmt) A minimal std::format implementation
* [doctest](https://github.com/doctest/doctest) A minimal C++ single-header testing framework
* [inih](https://github.com/benhoyt/inih) A minimal C based INI file parser

## License

See LICENSE.txt (MIT)
