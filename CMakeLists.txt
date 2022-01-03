cmake_minimum_required(VERSION 3.21)
project(utils VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

file(GLOB SRC src/*.cc src/*.h include/*.h)

add_library(utils STATIC ${SRC})
if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_compile_options(utils PRIVATE
            -Wall -Wextra -Wundef -Werror=return-type -Wconversion -Wpedantic
            -fno-exceptions -Wno-gnu-zero-variadic-macro-arguments -Wno-dollar-in-identifier-extension
            -O3 -Xclang -fcolor-diagnostics)
else ()
    target_compile_options(utils PRIVATE
            -Wall -Wextra -Wundef -Werror=return-type -Wconversion -Wpedantic
            -fno-exceptions -Wno-gnu-zero-variadic-macro-arguments -Wno-dollar-in-identifier-extension
            -O0 -ggdb -Xclang -fcolor-diagnostics)
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
