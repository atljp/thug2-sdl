# PARTYMOD for THUG2

Place d3d9.dll in the game folder and it will be loaded automatically with any THUG2.exe (except for Clownjob'd THUGTWO).<br>

This mod is still under development!

## FEATURES

- Unofficial port of [partymod-thaw](https://github.com/PARTYMANX/partymod-thaw)
- Replaced input system entirely with new, modern system using the SDL2 library
- Support for the original Ps2 controller layout
- Improved window handling allowing for custom resolutions and configurable windowing
- Replaced configuration files with new INI-based system (see partymod.ini)
- Option to disable intro movies
- Blur fix
- Selectable languages (English, French, German)
- Selectable button fonts (PC, Ps2, Xbox or Ngc)
- Stability fixes (e.g. for level merging)
- Fixed RNG
- THUG Airdrift
- Online play
- Unlimited three-axes scaling in Create-A-Skater
- Increased ranges for color sliders in Create-A-Skater
- Adjustable Drop down key(s)
- Improved graphics (shadows and edges are slightly sharper, most noticeably on high resolutions)
- Fixed graphical bug during screen flashes (e.g. Frankling Grind)
- Fixed camera shake effect not working while skater is off board (e.g. LA earthquake)

## TODO

- Change GameSpy to OpenSpy
- Fix menu controls
- Console text logging to file (append / overwrite)
- Error manager with crash reports
- Move / resize borderless window without locking the thread
- Textinput for more keyboard layouts: FR, UK, GER, RUS (new font?)
- Main menu 21:9 and 21:10 aspect ratio support
- More special text characters
- CAP surgeon mod
- Vanilla / speedrun mode
  - Ability to bind ENTER to X / A button (Arcade menu)
- THUG levels
- Minimal launcher
- Patcher with option to make custom executable without relying on the DLL being called d3d9.dll
- Fix window focus being lost when pressing CTRL-ALT-DELETE
- Player2 input for splitscreen
- Option to move gamecontrollerdb into data folder
- Fix broken savefiles having _ in the name
- Menu to load online preferences
- Ped items in Create-A-Skater
- Include leftover files as sprays
- Empty sticker texture
- Colored text input
- Prevent chat reset in netgames when starting/ending game or changing levels
- Quick bail getup
- Fun stuff: Boostplants, BHRA, low sticker slaps, wallride anywhere, jank drops, steep skating angles, high acid drops, etc..

## BUILDING

### Prerequisites
The build requires the following tools:
- [CMake](https://cmake.org/)
- [Visual Studio](https://visualstudio.microsoft.com/)
- [vcpkg](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-cmd)
- [Recommended: git](https://git-scm.com/download/win)
- [SDL2](https://www.libsdl.org/)

First download and install Visual Studio and CMake like any other GUI application.<br>
Install git via the Installer from the website or directly in Powershell:
- `winget install --id Git.Git -e --source winget`

Then install vcpkg via git as described in the Microsoft documentation:
- `git clone https://github.com/microsoft/vcpkg.git`
- Run the bootstrap script `cd vcpkg && .\bootstrap-vcpkg.bat`

Install SDL2 via vcpkg: `.\vcpkg.exe install SDL2`, for x86: `.\vcpkg.exe install SDL2:x86-windows`

### Generating the project file
Create a thug2-sdl/build directory. Then build the VS Solution with CMake from that directory via Powershell:
```
cmake .. -A win32 -DCMAKE_TOOLCHAIN_FILE=C:/[vcpkg directory]/scripts/buildsystems/vcpkg.cmake
```
Set the optimization for the partymod dll to O0 (disable optimization) because MSVC seems to break certain functions when optimization is enabled.
<br><br>
The `.sln` file is in the `build` directory. Make sure to select win32 and Release as the project's build options in Visual Studio. Build the actual DLL with `Ctrl+Shift+B`.

## CREDITS

- PARTYMANX: Original development of Partymod. Thanks for the help and patience!
- %.gone.: Clownjob'd mod for THUG2 which provided useful insights
- Zedek the Plague Doctor ™: LazyStruct and LazyArray classes, Logger and lots of script-related helper code
- Hardronix: DirectX9 and general code support
- Thanks to everyone who playtested and gave feedback!