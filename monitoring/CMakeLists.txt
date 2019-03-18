set(CMAKE_CXX_STANDARD 17)

if (MSVC)
    add_definitions (
        # Disable Microsoft's Secure STL.
        /D_ITERATOR_DEBUG_LEVEL=0
        # Use multiple processes for compiling.
        /MP
    )

add_definitions (
        # "qualifier applied to function type has no meaning; ignored"
        /wd4180
        #  integral constant overflow
        /wd4307
        # "'function': was declared deprecated" (referring to STL functions)
        /wd4996
    )

endif()

add_executable(monitor monitoring.cpp)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

target_link_libraries(monitor stdpmr supportlib)
