cmake_minimum_required (VERSION 3.7)
project(P1160 CXX)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
    set(P1160_TRY_CXX_STD_PREFIX "/std:c++")
else()
    set(P1160_TRY_CXX_STD_PREFIX "-std=c++")
endif()

include(CheckCXXSourceRuns)

set(CMAKE_REQUIRED_FLAGS "${CMAKE_CXX_FLAGS} ${P1160_TRY_CXX_STD_PREFIX}${CMAKE_CXX_STANDARD}")

check_cxx_source_runs(
   "#include <memory_resource>
    #include <vector>
    #include <deque>
    #include <string>

    int main()
    {
        typedef std::pmr::vector<int> ivec;
        typedef std::pmr::deque<int>  idec;
        typedef std::pmr::string      pmrstring;

        std::pmr::memory_resource *x = std::pmr::get_default_resource();
        return x == nullptr;
    }
   "

  P1160_HAS_CPP17_MEMORY_RESOURCE
)

check_cxx_source_runs(
   "#include <experimental/memory_resource>
    #include <experimental/vector>
    #include <experimental/deque>
    #include <experimental/string>

    int main()
    {
        typedef std::experimental::pmr::vector<int> ivec;
        typedef std::experimental::pmr::deque<int>  idec;
        typedef std::experimental::pmr::string      pmrstring;

        std::experimental::pmr::memory_resource *x =
                                std::experimental::pmr::get_default_resource();
        return x == nullptr;
    }
   "

  P1160_HAS_EXPERIMENTAL_MEMORY_RESOURCE
)

if(P1160_HAS_CPP17_MEMORY_RESOURCE)
  # nothing to do
elseif(P1160_HAS_EXPERIMENTAL_MEMORY_RESOURCE)
  message("INCLUDE_DIRECTORIES = ${INCLUDE_DIRECTORIES}")
  include_directories( patchpmr )
  message("INCLUDE_DIRECTORIES = ${INCLUDE_DIRECTORIES}")
else()
  message( FATAL_ERROR "No pmr support in compiler, CMake will exit." )
endif()

get_filename_component(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} ABSOLUTE)

add_subdirectory(src)
# add_subdirectory(include)
add_subdirectory(examples)
