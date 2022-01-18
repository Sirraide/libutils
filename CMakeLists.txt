cmake_minimum_required(VERSION 3.21)
project(utils VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
set(CMAKE_CXX_COMPILER g++)

file(GLOB SRC src/*.cc src/*.h include/*.h)

add_library(utils STATIC ${SRC})
target_compile_options(utils PRIVATE
        -Wall -Wextra -Wundef -Werror=return-type -Wconversion -Wpedantic
        -Wno-gnu-zero-variadic-macro-arguments -Wno-dollar-in-identifier-extension
        -Wno-gnu-case-range -fPIC -fcoroutines)
if (${CMAKE_CXX_COMPILER} STREQUAL "clang++")
    target_compile_options(utils PRIVATE -Xclang -fcolor-diagnostics)
endif ()
if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_compile_options(utils PRIVATE -O3)
else ()
    target_compile_options(utils PRIVATE -O0 -ggdb)
endif ()

if (NOT APPLE)
    set(CMAKE_INSTALL_RPATH $ORIGIN)
endif ()

set_target_properties(
        utils PROPERTIES
        SOVERSION 1
        VERSION 1.0.0
)

include(GNUInstallDirs)
install(TARGETS utils
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT LibUtils_Runtime
        NAMELINK_COMPONENT LibIO_Development
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT LibUtils_Development
        )
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/utils)
