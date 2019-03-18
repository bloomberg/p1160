set(CMAKE_CXX_STANDARD 17)

add_library(supportlib INTERFACE)

target_sources(supportlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/framer.h ${CMAKE_CURRENT_SOURCE_DIR}/assert.h)

target_include_directories(supportlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/.. )


