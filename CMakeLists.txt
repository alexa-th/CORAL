cmake_minimum_required(VERSION 3.24)

set(CMAKE_C_STANDARD_REQUIRED   ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD            17)
set(CMAKE_CXX_STANDARD          20)

project(CORAL
        VERSION 1.0.0
        LANGUAGES C CXX
)



# Default behaviour

set(CMAKE_BUILD_TYPE
    Release
    CACHE
    STRING
    "Build type"
)

set(CORAL_BUILD_TESTS
    OFF
    CACHE
    BOOL
    "Build all tests"
)

set(CORAL_USE_IPO
    ON
    CACHE
    BOOL
    "Try to use IPO (Interprocedural Optimization)"
)

set(CORAL_USE_SANITIZATION
    OFF
    CACHE
    BOOL
    "Build with sanitization"
)

set(CORAL_MATH_EXCLUDE_TYPES
    "INT;UNSIGNED"
    CACHE
    STRING
    "Semicolon seperated list of types to exclude from the math submodule"
)

set(CORAL_GRAPHICS_RENDERING_IMPL
    "RT_CPU"
    CACHE
    STRING
    "The rendering implementation to use; may be \"RT_CPU\" or \"NULL\""
)

set(CORAL_GRAPHICS_FLOAT_TYPE
    "FLOAT"
    CACHE
    STRING
    "The type to use for math operations in the graphics submodule; may be \"FLOAT\" or \"DOUBLE\""
)

set(CORAL_MSVC_MAX_ALIGN_FIX
    ON
    CACHE
    BOOL
    "Fix the missing max_align_t definition when using MSVC"
)


set(CORAL_BASE_DIR                      ${CMAKE_CURRENT_SOURCE_DIR})
set(CORAL_SOURCE_DIR                    ${CORAL_BASE_DIR}/src)
set(CORAL_INCLUDE_DIR                   ${CORAL_BASE_DIR}/include)
set(CORAL_INTERNAL_INCLUDE_DIR          ${CORAL_SOURCE_DIR})

set(CORAL_DEPENDENCY_BASE_SOURCE_DIR    ${CMAKE_SOURCE_DIR}/dependencies)
set(CORAL_DEPENDENCY_BASE_BINARY_DIR    ${CMAKE_BINARY_DIR}/dependencies)


include(GNUInstallDirs)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})

set(CORAL_INSTALL_RUNTIME_DIR   ${CMAKE_INSTALL_BINDIR})
set(CORAL_INSTALL_LIBRARY_DIR   ${CMAKE_INSTALL_LIBDIR})
set(CORAL_INSTALL_ARCHIVE_DIR   ${CORAL_INSTALL_LIBRARY_DIR})
set(CORAL_INSTALL_INCLUDE_DIR   ${CMAKE_INSTALL_INCLUDEDIR}/CORAL)


set(CORAL_PUBLIC_COMPILE_DEFINITIONS    "")
set(CORAL_PRIVATE_COMPILE_DEFINITIONS   "")

set(CORAL_PUBLIC_COMPILE_OPTIONS    "")
set(CORAL_PRIVATE_COMPILE_OPTIONS   "")

set(CORAL_PUBLIC_LINK_OPTIONS   "")
set(CORAL_PRIVATE_LINK_OPTIONS  "")



# Enforcing global default behaviour

if (CORAL_USE_IPO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_SUPPORTED_OUTPUT)

    if (IPO_SUPPORTED)
        message(STATUS "Using IPO (Interprocedural Optimization)")
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(WARNING "IPO (Interprocedural Optimization) is not supported: ${IPO_SUPPORTED_OUTPUT}")
    endif()
endif()


if (CORAL_USE_SANITIZATION)
    message(STATUS "Using sanitizers")

    if (MSVC)
        message(STATUS "Using MSVC sanitization options")
        set(CORAL_SANIIIZATION_OPTIONS "/fsanitize=address,undefined")
    else()
        set(CORAL_SANITIZATION_OPTIONS "-fsanitize=address,undefined")
    endif()

    list(APPEND
         CORAL_PUBLIC_COMPILE_OPTIONS
         ${CORAL_SANITIZATION_OPTIONS}
    )

    list(APPEND
         CORAL_PUBLIC_LINK_OPTIONS
         ${CORAL_SANITIZATION_OPTIONS}
    )
endif()


foreach(TYPE IN LISTS CORAL_MATH_EXCLUDE_TYPES)
    message(STATUS "Excluding type \"${TYPE}\" from the math submodule")

    list(APPEND
         CORAL_PUBLIC_COMPILE_DEFINITIONS
         "CORAL_MATH_EXCLUDE_${TYPE}=1"
    )
endforeach()


list(APPEND
     CORAL_PRIVATE_COMPILE_DEFINITIONS
     "CORAL_GRAPHICS_RENDERING_IMPL_${CORAL_GRAPHICS_RENDERING_IMPL}"
)

message(STATUS "Using the ${CORAL_GRAPHICS_RENDERING_IMPL} rendering implementation.")


list(APPEND
     CORAL_PUBLIC_COMPILE_DEFINITIONS
     "CORAL_GRAPHICS_FLOAT_TYPE=CORAL_MATH_TYPE_${CORAL_GRAPHICS_FLOAT_TYPE}"
)

message(STATUS "Using CORAL_MATH_TYPE_${CORAL_GRAPHICS_FLOAT_TYPE} as float type for the graphics submodule.")



# Compiler specific behaviour

if (MSVC)
    list(APPEND
         CORAL_PRIVATE_COMPILE_OPTIONS
         "/experimental:c11atomics"
         "/Zc:preprocessor"
    )

    list(APPEND
         CORAL_PUBLIC_COMPILE_DEFINITIONS
         "CORAL_CONCURRENCY_ATOMIC_FORCE_INCLUDE=1"
    )

    if (CORAL_MSVC_MAX_ALIGN_FIX)
        list(APPEND
             CORAL_PRIVATE_COMPILE_DEFINITIONS
             "CORAL_MSVC_MAX_ALIGN_FIX=1"
        )
    endif()
endif()



# Source and header files

set(CORAL_SOURCES
    src/common/random.c
    src/common/utils.c
    src/memory/allocators.c
    src/containers/string.c
    src/containers/buffer.c
    src/containers/linkedList.c
    src/filesystem/path.c
    src/filesystem/file.c
    src/filesystem/io.c
    # src/log/log.c
    src/concurrency/signal_win32.c
    src/concurrency/signal_linux.c
    src/concurrency/mutex.c
    src/concurrency/thread_win32.c
    src/concurrency/thread_linux.c
    src/concurrency/threadPool.c
    src/math/vec.c
    src/math/mat.c
    src/graphics/graphics.c
    src/graphics/math.c
    src/graphics/mesh.c
    src/graphics/object.c
    src/graphics/scene.c
    src/graphics/windowing.c
    src/graphics/windowingImplementations/windowing_null.c
    src/graphics/windowingImplementations/windowing_win32.cpp
    src/graphics/renderingImplementations/rendering_null.c
    src/graphics/renderingImplementations/rendering_rt_cpu.c
    src/graphics/renderingImplementations/rendering_rt_cpu_bvh.c
    src/graphics/renderingImplementations/rendering_rt_cpu_ray.c
)

set(CORAL_PUBLIC_HEADERS
    include/coral.h
    include/common/common.h
    include/common/api.h
    include/common/targets.h
    include/common/compilers.h
    include/common/atomic.h
    include/common/noreturn.h
    include/common/error.h
    include/common/random.h
    include/common/utils.h
    include/memory/memory.h
    include/memory/dynalloc.h
    include/memory/allocators.h
    include/containers/containers.h
    include/containers/string.h
    include/containers/buffer.h
    include/containers/linkedList.h
    include/filesystem/filesystem.h
    include/filesystem/error.h
    include/filesystem/path.h
    include/filesystem/file.h
    include/filesystem/io.h
    # include/log/log.h
    include/concurrency/concurrency.h
    include/concurrency/signal.h
    include/concurrency/mutex.h
    include/concurrency/threadDefinition.h
    include/concurrency/threadDefinition_win32.h
    include/concurrency/threadDefinition_linux.h
    include/concurrency/thread.h
    include/concurrency/threadPool.h
    include/math/math.h
    include/math/definitions.h
    include/math/baseTypes.h
    include/math/vecTypes.h
    include/math/matTypes.h
    include/math/vec.h
    include/math/mat.h
    include/graphics/graphics.h
    include/graphics/math.h
    include/graphics/mesh.h
    include/graphics/object.h
    include/graphics/scene.h
    include/graphics/input.h
    include/graphics/windowing.h
    include/graphics/rendering.h
)

set(CORAL_INTERNAL_HEADERS
    src/concurrency/concurrency_implementation.h
    src/graphics/object_internal.h
    src/graphics/windowing_internal.h
    src/graphics/windowing_implementation.h
    src/graphics/rendering_internal.h
    src/graphics/renderingImplementations/rendering_rt_cpu_internal.h
    src/graphics/renderingImplementations/rendering_rt_cpu_internal_bvh.h
    src/graphics/renderingImplementations/rendering_rt_cpu_internal_ray.h
)



# Main target

add_library(CORAL
            SHARED
            ${CORAL_SOURCES}
            ${CORAL_INTERNAL_HEADERS}
)

target_sources(CORAL
               PUBLIC
               FILE_SET
                   public_headers
                   TYPE HEADERS
                   BASE_DIRS ${CORAL_INCLUDE_DIR}
                   FILES ${CORAL_PUBLIC_HEADERS}
)

target_sources(CORAL
               PRIVATE
               FILE_SET
                   internal_headers
                   TYPE HEADERS
                   BASE_DIRS ${CORAL_INTERNAL_INCLUDE_DIR}
                   FILES ${CORAL_INTERNAL_HEADERS}
)

target_compile_definitions(CORAL
                           PUBLIC   ${CORAL_PUBLIC_COMPILE_DEFINITIONS}
                           PRIVATE  ${CORAL_PRIVATE_COMPILE_DEFINITIONS}
)

target_compile_options(CORAL
                       PUBLIC   ${CORAL_PUBLIC_COMPILE_OPTIONS}
                       PRIVATE  ${CORAL_PRIVATE_COMPILE_OPTIONS}
)

target_link_options(CORAL
                    PUBLIC  ${CORAL_PUBLIC_LINK_OPTIONS}
                    PRIVATE ${CORAL_PRIVATE_LINK_OPTIONS}
)


set(THREADS_PREFER_PTHREAD_FLAG TRUE)

find_package(Threads REQUIRED)

target_link_libraries(CORAL
                      PRIVATE
                      Threads::Threads
)

if (NOT CMAKE_THREAD_LIBS_INIT)
    message(STATUS "Using no further threading library")
else()
    message(STATUS "Using threading library: ${CMAKE_THREAD_LIBS_INIT}")
endif()


if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    target_link_libraries(CORAL
                          PRIVATE
                          synchronization.lib
                          d2d1.lib
                          Dwmapi.lib
    )
endif()


install(TARGETS CORAL
        EXPORT CORAL
        RUNTIME
            DESTINATION ${CORAL_INSTALL_RUNTIME_DIR}
            COMPONENT Main
        LIBRARY
            DESTINATION ${CORAL_INSTALL_LIBRARY_DIR}
            COMPONENT Main
        ARCHIVE
            DESTINATION ${CORAL_INSTALL_ARCHIVE_DIR}
            COMPONENT Main
        FILE_SET public_headers
            DESTINATION ${CORAL_INSTALL_INCLUDE_DIR}
            COMPONENT Main
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)



# Tests

if (CORAL_BUILD_TESTS)
    include(CTest)
    enable_testing()

    add_subdirectory(tests)
endif()



# Installation

include(CMakePackageConfigHelpers)

configure_package_config_file(${CORAL_BASE_DIR}/CORALConfig.cmake.in
                              ${CORAL_BINARY_DIR}/CORALConfig.cmake
                              INSTALL_DESTINATION CMake
                              PATH_VARS
                                  CORAL_INSTALL_RUNTIME_DIR
                                  CORAL_INSTALL_INCLUDE_DIR
)

write_basic_package_version_file(${CORAL_BINARY_DIR}/CORALConfigVersion.cmake
                                 COMPATIBILITY SameMinorVersion
)

install(FILES
            ${CORAL_BINARY_DIR}/CORALConfig.cmake
            ${CORAL_BINARY_DIR}/CORALConfigVersion.cmake
        DESTINATION CMake
        COMPONENT Main
)

install(EXPORT CORAL
        DESTINATION CMake
        NAMESPACE CORAL::
)
