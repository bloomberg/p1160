set(CMAKE_CXX_STANDARD 17)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(stage1 stage1.cpp pstring_stage1.hpp)
target_link_libraries(stage1 pmr_test_resource)

add_executable(stage2 stage2.cpp pstring_stage2.hpp)
target_link_libraries(stage2 pmr_test_resource)

add_executable(stage3 stage3.cpp pstring_stage3.hpp)
target_link_libraries(stage3 pmr_test_resource)

add_executable(stage4 stage4.cpp pstring_stage4.hpp)
target_link_libraries(stage4 pmr_test_resource)

add_executable(stage4a stage4a.cpp pstring_stage4.hpp)
target_link_libraries(stage4a pmr_test_resource)

add_executable(stage5 stage5.cpp pstring_stage5.hpp)
target_link_libraries(stage5 pmr_test_resource)

add_executable(stage6 stage6.cpp pstring_stage6.hpp)
target_link_libraries(stage6 pmr_test_resource)

add_executable(stage7 stage7.cpp pstring_stage7.hpp)
target_link_libraries(stage7 pmr_test_resource)

add_executable(stage7a stage7a.cpp pstring_stage7.hpp)
target_link_libraries(stage7a pmr_test_resource)

add_executable(stage8 stage8.cpp pstring_stage8.hpp)
target_link_libraries(stage8 pmr_test_resource)

add_executable(stage8a stage8a.cpp pstring_stage8.hpp)
target_link_libraries(stage8a pmr_test_resource)

add_executable(stage9 stage9.cpp pstring_stage9.hpp)
target_link_libraries(stage9 pmr_test_resource)

add_executable(last last.cpp pstring_last.hpp)
target_link_libraries(last pmr_test_resource)
