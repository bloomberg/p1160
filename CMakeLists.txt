cmake_minimum_required (VERSION 3.5)
project(P1160 CXX)

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

get_filename_component(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} ABSOLUTE)

add_subdirectory(stdpmr)
add_subdirectory(supportlib)
add_subdirectory(pstring)
add_subdirectory(monitoring)
add_subdirectory(exception_testing)
