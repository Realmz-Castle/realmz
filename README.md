# Realmz

Realmz is a classic, turn-based RPG, originally developed for early Macintosh computers. It was originally released as shareware, with additional scenarios available for purchase. Tim has graciously agreed to a release of the original code under a non-commercial license (see "License" section below).

# License

<p xmlns:cc="http://creativecommons.org/ns#">Realmz, copyright © 1994 by Tim Phillips. Modified for compatibility with modern systems (see CHANGELOG for detailed modification notes). Realmz and its associated software, in both source code and binary formats, its game assets, and its documentation (the Licensed Material), are distributed under the terms of the <a href="https://creativecommons.org/licenses/by-nc-sa/4.0/?ref=chooser-v1" target="_blank" rel="license noopener noreferrer" style="display:inline-block;">Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International<img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/cc.svg?ref=chooser-v1" alt=""><img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/by.svg?ref=chooser-v1" alt=""><img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/nc.svg?ref=chooser-v1" alt=""><img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/sa.svg?ref=chooser-v1" alt=""></a>. The Licensed Material is provided on an as-is basis, with no warranties of any kind.</p>

# Building on Mac

- Download dependencies as git submodules
  - `git submodule init`
  - Download external dependencies of SDL_ttf `vendored/SDL_ttf/external/download.sh`
- `VERBOSE=1 cmake --fresh  -B build -D CMAKE_BUILD_TYPE=Debug -D CMAKE_VERBOSE_MAKEFILE:BOOL=ON -DSDLTTF_VENDORED=ON -DDISABLE_SDL:BOOL=ON`

# Cross-compiling for Windows from Mac

- Install [llvm-mingw](https://github.com/mstorsjo/llvm-mingw)
  - Download latest llvm-mingw-<date>-ucrt-macos-universal.tar.xz
  - Extract
  - `sudo mv ~/Downloads/llvm-mingw-<date>-ucrt-macos-universal.tar.xz /opt/llvm-mingw`
- Create a [toolchain file](https://cmake.org/cmake/help/book/mastering-cmake/chapter/Cross%20Compiling%20With%20CMake.html#toolchain-files)
- Set up a CMake build directory for windows using the toolchain file
  - `VERBOSE=1 cmake -B build_win -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON -DSDLTTF_VENDORED=ON -DDISABLE_SDL:BOOL=ON -DCMAKE_TOOLCHAIN_FILE=~/TC-mingw.cmake`
- Build for windows using llvm-mingw `cmake --build build_win`
- Generate distributable zip package `cd build_win && cpack && cd -`
