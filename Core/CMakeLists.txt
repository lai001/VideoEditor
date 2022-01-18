CMAKE_MINIMUM_REQUIRED(VERSION 3.16.3)

PROJECT(VideoEditorCore VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_GNUtoMS OFF)
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})

file(GLOB HEADER_FILE "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "Time/*.h" "Track/*.h" "Utility/*.h"  "ThirdParty/*.h")

include_directories("../" "../Vendor/win32/ffmpeg/include" "../Vendor/win32/rapidjson/include" "../Vendor/win32/spdlog/include")
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} CORE_SRC)
aux_source_directory("Time" TIME_SRC)
aux_source_directory("Track" TRACK_SRC)
aux_source_directory("Utility" UTILITY_SRC)

add_library(${PROJECT_NAME} STATIC ${CORE_SRC} ${TIME_SRC} ${TRACK_SRC} ${UTILITY_SRC} ${HEADER_FILE})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${CORE_SRC} ${TIME_SRC} ${TRACK_SRC} ${UTILITY_SRC} ${HEADER_FILE})

add_subdirectory("../Vendor/win32/spdlog" "spdlog.dir")
add_dependencies(VideoEditorCore spdlog)