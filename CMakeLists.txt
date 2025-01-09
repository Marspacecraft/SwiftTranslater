cmake_minimum_required(VERSION 3.10)
project(SwiftTranslater)
set(CMAKE_CXX_STANDARD 17)
include(ExternalProject)

set(HOME_PATH $ENV{HOME})

# 下载ColorCout
set(COLORCOUT_SOURCE_DIR ${PROJECT_SOURCE_DIR}/ColorCout)
if(NOT EXISTS ${COLORCOUT_SOURCE_DIR})
    message(STATUS "Cloning ColorCout from GitHub...")
    execute_process(COMMAND git clone "https://github.com/Marspacecraft/ColorCout.git" "${COLORCOUT_SOURCE_DIR}")
endif()
# 检查是否安装nlohmann-json
find_package(nlohmann_json QUIET)
if(nlohmann_json_FOUND)
    message(STATUS "Found nlohmann-json: ${nlohmann_json_VERSION}-${nlohmann_json_INCLUDE_DIRS}")
    include_directories(${nlohmann_json_INCLUDE_DIRS})
    include_directories("/usr/local/include")
else()
    message(FATAL_ERROR "nlohmann-json not found. You can install it using `brew install nlohmann-json` or download from \"https://github.com/nlohmann/json\".")
endif()

# 检查是否安装Python3
find_package(Python3 REQUIRED COMPONENTS Interpreter)
if (Python3_FOUND)
    message(STATUS "Python3 found: ${Python3_EXECUTABLE}")
    message(STATUS "Python3 version: ${Python3_VERSION}")
else()
    message(FATAL_ERROR "Python3 not found. Please install Python3 and try again.")
endif()

# 配置编译文件
add_subdirectory(ColorCout)
include_directories(${PROJECT_SOURCE_DIR}/include)
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
add_executable(swifttrans swifttranslater.cpp ${SOURCES})
target_link_libraries(swifttrans ccout)

# 设置编译选项
target_compile_options(swifttrans PRIVATE $<$<CONFIG:Debug>:-g> $<$<CONFIG:Release>:-O3>)

# 设置默认构建类型为 Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# 设置json文件保存路径
if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    # macOS
    set(SWIFT_JSON_DIR "${HOME_PATH}/Library/swifttranslater/")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    # Linux
    set(SWIFT_JSON_DIR "${HOME_PATH}/Library/swifttranslater/")
else()
    message(FATAL_ERROR "Unsupported operating system")
endif()
add_definitions(-DSWIFT_JSON_DIRECTORY="${SWIFT_JSON_DIR}")

# 添加自动安装部署
set(CMAKE_INSTALL_PREFIX "/usr/local")
install(TARGETS swifttrans RUNTIME DESTINATION bin)
install(FILES ${PROJECT_SOURCE_DIR}/GoogleTranslate.py DESTINATION ${SWIFT_JSON_DIR}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

# 添加自动卸载目标
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)
add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

# 添加make选项
add_custom_target(debug
    COMMAND make clean && cmake -DCMAKE_BUILD_TYPE=Debug .. && make &)
add_custom_target(release
    COMMAND make clean && cmake -DCMAKE_BUILD_TYPE=Release .. && make &)