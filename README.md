# Realmz

Realmz is a classic, turn-based RPG, originally developed for early Macintosh computers. It was originally released as shareware, with additional scenarios available for purchase. Tim has graciously agreed to a release of the original code under a non-commercial license (see "License" section below).

# License

<p xmlns:cc="http://creativecommons.org/ns#">Realmz, copyright © 1994 by Tim Phillips. Modified for compatibility with modern systems (see CHANGELOG.md for detailed modification notes). Realmz and its associated software, in both source code and binary formats, its game assets, and its documentation (the Licensed Material), are distributed under the terms of the <a href="https://creativecommons.org/licenses/by-nc-sa/4.0/?ref=chooser-v1" target="_blank" rel="license noopener noreferrer" style="display:inline-block;">Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International<img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/cc.svg?ref=chooser-v1" alt=""><img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/by.svg?ref=chooser-v1" alt=""><img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/nc.svg?ref=chooser-v1" alt=""><img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/sa.svg?ref=chooser-v1" alt=""></a>. The Licensed Material is provided on an as-is basis, with no warranties of any kind.</p>

# Installing

_WARNING: This is a beta release. The game may be unstable, crashes may occur, and save game and character data may become corrupted. If you have saves or character files that you care about, we strongly suggest regularly backing up your user data directory (`%AppData%\Fantasoft\Realmz` on Windows and `~/Library/Application\ Support/Fantasoft/Realmz` on Mac)._

Download the latest release for your system from the releases page. Scroll down to and expand the "Assets" section. Download the `.dmg` file for Mac, and the `.zip` file for Windows.

On Mac, double click the `.dmg` file you downloaded, then click and drag the Realmz bundle into your Applications folder. As reported in #171, there is currently an issue with the Mac application bundle. You must build the SDL3, SDL_ttf, and SDL_image shared dylibs, then copy them into `/Applications/Realmz.app/Contents/MacOS` in order for Realmz to successfully launch. This was resolved by #175 and should be fixed with the next release.

On Windows, extract the `.zip` file to your Program Files directory.

# Reporting Bugs

- Save the crash report file (if possible)
- Zip up your Realmz userdata directory (`%AppData%\Fantasoft\Realmz` on Windows, `~/Library/Application\ Support/Fantasoft/Realmz` on Mac)
- Submit an issue to the Github repository
- Attach the crash report and archive of your userdata directory
- List the steps necessary to reproduce the bug

# Building on Mac

- Download dependencies as git submodules
  - `git submodule init`
  - Download external dependencies of SDL_ttf `vendored/SDL_ttf/external/download.sh`
- Download and install [phosg](https://github.com/fuzziqersoftware/phosg) (commit [b2e0c12edb7e274a5e20c460f44eee44f49f57ef](https://github.com/fuzziqersoftware/phosg/tree/b2e0c12edb7e274a5e20c460f44eee44f49f57ef)) and [resource_dasm](https://github.com/fuzziqersoftware/resource_dasm) (commit [27f64c89a5fed855e68c2a5e97b6c6c389d8eb19](https://github.com/fuzziqersoftware/resource_dasm/tree/27f64c89a5fed855e68c2a5e97b6c6c389d8eb19)). Make sure to compile with `-DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"` in order to build Realmz as a fat binary that can run on both architectures. Also use `-DCMAKE_OSX_DEPLOYMENT_TARGET=13.3` to make sure all dependencies and Realmz are targeting the same minimum MacOS SDK.
- `cmake --preset macOS`
- `cmake --build --preset macOS`

# Cross-compiling for Windows from Mac

- Install [llvm-mingw](https://github.com/mstorsjo/llvm-mingw)
  - Download latest llvm-mingw-<date>-ucrt-macos-universal.tar.xz
  - Extract
  - `sudo mv ~/Downloads/llvm-mingw-<date>-ucrt-macos-universal.tar.xz /opt/llvm-mingw`
- Create a [toolchain file](https://cmake.org/cmake/help/book/mastering-cmake/chapter/Cross%20Compiling%20With%20CMake.html#toolchain-files)
- Clone and build phosg, resource_dasm, and zlib dependencies and install to ~/mingw-install
  - `cmake --fresh -B build -D CMAKE_TOOLCHAIN_FILE=~/workspace/TC-mingw.cmake -D CMAKE_INSTALL_PREFIX=~/mingw-install -D CMAKE_BUILD_TYPE=Debug`
- Set up a CMake build directory for windows using the toolchain file
  - `VERBOSE=1 cmake -B build_win -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON -DSDLTTF_VENDORED=ON -DDISABLE_SDL:BOOL=ON -DCMAKE_TOOLCHAIN_FILE=~/TC-mingw.cmake`
- Build for windows using llvm-mingw `cmake --build build_win --target package`
