# Realmz

Realmz is a classic, turn-based RPG, originally developed for early Macintosh computers. It was originally released as shareware, with additional scenarios available for purchase. Tim has graciously agreed to a release of the original code under a non-commercial license (see "License" section below).

# License

<p xmlns:cc="http://creativecommons.org/ns#">Realmz, copyright © 1994 by Tim Phillips. Modified for compatibility with modern systems (see CHANGELOG.md for detailed modification notes). Realmz and its associated software, in both source code and binary formats, its game assets, and its documentation (the Licensed Material), are distributed under the terms of the <a href="https://creativecommons.org/licenses/by-nc-sa/4.0/?ref=chooser-v1" target="_blank" rel="license noopener noreferrer" style="display:inline-block;">Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International<img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/cc.svg?ref=chooser-v1" alt=""><img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/by.svg?ref=chooser-v1" alt=""><img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/nc.svg?ref=chooser-v1" alt=""><img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/sa.svg?ref=chooser-v1" alt=""></a>. The Licensed Material is provided on an as-is basis, with no warranties of any kind.</p>

# Installing

_WARNING: This is a beta release. The game may be unstable, crashes may occur, and save game and character data may become corrupted. If you have saves or character files that you care about, we strongly suggest regularly backing up your user data directory (`%AppData%\Fantasoft\Realmz` on Windows and `~/Library/Application\ Support/Fantasoft/Realmz` on Mac)._

Download the latest release for your system from the releases page. Scroll down to and expand the "Assets" section. Download the `.dmg` file for Mac, and the `.exe` or `.zip` files for Windows.

On Mac, double click the `.dmg` file you downloaded, then click and drag the Realmz bundle into your Applications folder.

On Windows, you can either use the installer wizard for automatic installation, or a ZIP archive for custom installations. To use the installer, double click the `.exe` you downloaded. Accept the license agreement, choose an install location for Realmz, and continue through the "components" section of the installer.

# Reporting Bugs

- Save the crash report file (if possible)
- Zip up your Realmz userdata directory (`%AppData%\Fantasoft\Realmz` on Windows, `~/Library/Application\ Support/Fantasoft/Realmz` on Mac)
- Submit an issue to the Github repository
- Attach the crash report and archive of your userdata directory
- List the steps necessary to reproduce the bug

# Building on Mac

- Download git submodules: `git submodule update --init`
- Download external dependencies: `cmake -P FetchDependencies.cmake`
- Configure: `cmake --preset macOS`
- Build: `cmake --build --preset macOS`

# Cross-compiling for Windows from Mac

- Install [llvm-mingw](https://github.com/mstorsjo/llvm-mingw)
  - Download latest llvm-mingw-$DATE-ucrt-macos-universal.tar.xz
  - Extract the archive
  - `sudo mv ~/Downloads/llvm-mingw-$DATE-ucrt-macos-universal /opt/llvm-mingw`
- Install NSIS for installer generation `brew install nsis`
- Create a [toolchain file](https://cmake.org/cmake/help/book/mastering-cmake/chapter/Cross%20Compiling%20With%20CMake.html#toolchain-files)
- Clone and build phosg, resource_dasm, and zlib dependencies and install to ~/mingw-install
  - `cmake --fresh -B build -D CMAKE_TOOLCHAIN_FILE=~/workspace/TC-mingw.cmake -D CMAKE_INSTALL_PREFIX=~/mingw-install -D CMAKE_BUILD_TYPE=Debug`
- Set up a CMake build directory for windows using the toolchain file
  - `VERBOSE=1 cmake -B build_win -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON -DSDLTTF_VENDORED=ON -DDISABLE_SDL:BOOL=ON -DCMAKE_TOOLCHAIN_FILE=~/TC-mingw.cmake`
- Build for windows using llvm-mingw `cmake --build build_win --target package`
