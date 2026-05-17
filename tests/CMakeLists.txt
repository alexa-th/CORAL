# Unity testing framework

set(CORAL_DEPENDENCY_UNITY_SOURCE_DIR   ${CORAL_DEPENDENCY_BASE_SOURCE_DIR}/unity)
set(CORAL_DEPENDENCY_UNITY_BINARY_DIR   ${CORAL_DEPENDENCY_BASE_BINARY_DIR}/unity)


if (NOT EXISTS ${CORAL_DEPENDENCY_UNITY_SOURCE_DIR})
    include(FetchContent)

    FetchContent_Declare(unity
                         GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
                         GIT_TAG cbcd08fa7de711053a3deec6339ee89cad5d2697 # v2.6.1
                         SOURCE_DIR ${CORAL_DEPENDENCY_UNITY_SOURCE_DIR}
                         BINARY_DIR ${CORAL_DEPENDENCY_UNITY_BINARY_DIR}
    )

    FetchContent_GetProperties(unity
                               POPULATED CORAL_DEPENDENCY_UNITY_POPULATED
    )

    if (NOT ${CORAL_DEPENDENCY_UNITY_POPULATED})
        FetchContent_Populate(unity)
    endif()
endif()


add_subdirectory(${CORAL_DEPENDENCY_UNITY_SOURCE_DIR}
                 ${CORAL_DEPENDENCY_UNITY_BINARY_DIR}
                 EXCLUDE_FROM_ALL
)

target_compile_definitions(unity
                           PUBLIC
                           UNITY_INCLUDE_DOUBLE
)



# Test source files

set(CORAL_TEST_SOURCE_NAMES
    memory
    containers
    concurrency
    filesystem
    math
    graphics
)



# Test targets

foreach(SOURCE_NAME IN LISTS CORAL_TEST_SOURCE_NAMES)
    add_executable(test_${SOURCE_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_NAME}.c)

    target_link_libraries(test_${SOURCE_NAME}
                          PRIVATE
                          CORAL
                          unity
    )

    add_test(NAME CORAL_TEST_${SOURCE_NAME}
             COMMAND test_${SOURCE_NAME}
    )
endforeach()
