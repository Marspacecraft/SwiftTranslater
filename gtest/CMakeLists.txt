# tests/CMakeLists.txt

# 添加 Google Test 子模块
include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
)

# 禁用 Google Test 自带的测试
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# 启用测试
enable_testing()

# 添加测试可执行文件
add_executable(MyTests test_main.cpp)

# 链接 Google Test 库
target_link_libraries(MyTests gtest_main)

# 添加测试
add_test(NAME MyTests COMMAND MyTests)