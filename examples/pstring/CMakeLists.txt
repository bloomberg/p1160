set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

list(APPEND pmr_examples
    stage1
    stage2
    stage3
    stage4
    stage4a
    stage5
    stage6
    stage7
    stage7a
    stage8
    stage8a
    stage9
    last
)

foreach(example ${pmr_examples})
    add_executable(${example} ${example}.cpp)
    target_link_libraries(${example} PRIVATE pmr_test_resource)
endforeach()
