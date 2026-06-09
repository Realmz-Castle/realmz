include("vendored/FetchDependency/FetchDependency.cmake")

# Run the SDL update script.
execute_process(COMMAND "vendored/SDL_ttf/external/download.sh")

set(Generator "Unix Makefiles")
set(FETCH_DEPENDENCY_DEFAULT_SOURCE_ROOT "build/vendor/src")
set(FETCH_DEPENDENCY_DEFAULT_BINARY_ROOT "build/vendor/bin")

fetch_dependency(phosg
  GIT_SOURCE https://github.com/fuzziqersoftware/phosg.git
  VERSION b2e0c12edb7e274a5e20c460f44eee44f49f57ef
  GENERATOR "${Generator}"
  CONFIGURATION Debug
  LIST_SEPARATOR "<s>"
  CONFIGURE_OPTIONS -DCMAKE_OSX_ARCHITECTURES="x86_64<s>arm64" -DCMAKE_OSX_DEPLOYMENT_TARGET=13.3
)

fetch_dependency(resource_dasm
  PACKAGE_NAME resource_file
  GIT_SOURCE https://github.com/fuzziqersoftware/resource_dasm.git
  VERSION 27f64c89a5fed855e68c2a5e97b6c6c389d8eb19
  GENERATOR "${Generator}"
  CONFIGURATION Debug
  LIST_SEPARATOR "<s>"
  CONFIGURE_OPTIONS -DCMAKE_OSX_ARCHITECTURES="x86_64<s>arm64" -DCMAKE_OSX_DEPLOYMENT_TARGET=13.3
)

export_dependencies(PATH "build/vendor/import.cmake")
