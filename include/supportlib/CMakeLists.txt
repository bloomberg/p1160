set(CMAKE_CXX_STANDARD 17)

add_library(supportlib INTERFACE)

target_sources(supportlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/framer.hpp ${CMAKE_CURRENT_SOURCE_DIR}/assert.hpp)

target_include_directories(supportlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/.. )
