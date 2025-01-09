cmake_minimum_required(VERSION 3.10)
project(SwiftTranslater)
set(CMAKE_CXX_STANDARD 17)
include(ExternalProject)

set(HOME_PATH $ENV{HOME})
set(COLORCOUT_SOURCE_DIR ${PROJECT_SOURCE_DIR}/ColorCout)
set(COLORCOUT_INCLUDE_DIR ${COLORCOUT_SOURCE_DIR}/include)
set(COLORCOUT_BINARY_DIR ${CMAKE_BINARY_DIR}/ColorCout/build)
# 下载ColorCout
if(NOT EXISTS ${COLORCOUT_SOURCE_DIR})
    message(STATUS "ColorCout source directory does not exist. Cloning from GitHub...")
    ExternalProject_Add(
        ColorCout
        PREFIX ${CMAKE_BINARY_DIR}/ColorCout
        GIT_REPOSITORY https://github.com/Marspacecraft/ColorCout.git
        GIT_TAG main 
        SOURCE_DIR ${COLORCOUT_SOURCE_DIR}
        BINARY_DIR ${COLORCOUT_BINARY_DIR}
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        LOG_DOWNLOAD ON
    )
else()
    message(STATUS "ColorCout source directory exists. Skipping clone.")
endif()

include_directories(${COLORCOUT_INCLUDE_DIR})

find_package(nlohmann_json QUIET)
if(nlohmann_json_FOUND)
    message(STATUS "Found nlohmann-json: ${nlohmann_json_VERSION}-${nlohmann_json_INCLUDE_DIRS}")
    include_directories(${nlohmann_json_INCLUDE_DIRS})
    include_directories("/usr/local/include")
else()
    message(FATAL_ERROR "nlohmann-json not found. You can install it using `brew install nlohmann-json` or download from \"https://github.com/nlohmann/json\".")
endif()

add_library(ccout STATIC ${COLORCOUT_SOURCE_DIR}/src/ccout.cpp)

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

if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    # macOS
    set(SWIFT_JSON_DIR "${HOME_PATH}/Library/swifttranslater/")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    # Linux
    set(SWIFT_JSON_DIR "${HOME_PATH}/Library/swifttranslater/")
else()
    message(FATAL_ERROR "Unsupported operating system")
endif()

# 设置json文件保存路径
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
    IMMEDIATE @ONLY
)
add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
)

add_custom_target(debug
    COMMAND make clean && cmake -DCMAKE_BUILD_TYPE=Debug .. && make &
)

add_custom_target(release
    COMMAND make clean && cmake -DCMAKE_BUILD_TYPE=Release .. && make &
)