CMAKE_MINIMUM_REQUIRED(VERSION 3.16.3)

PROJECT(VideoEditor VERSION 0.1.0 LANGUAGES CXX)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_GNUtoMS OFF)
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})

link_directories("../Vendor/win32/ffmpeg/lib" REQUIRED)

include_directories("../")
set(MAIN_SRC "main.cpp")
add_executable(${PROJECT_NAME} ${MAIN_SRC})
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")

add_dependencies(VideoEditor spdlog)
add_dependencies(VideoEditor VideoEditorCore)
add_subdirectory("../Core" "VideoEditorCore.dir")
add_subdirectory("../Vendor/win32/tclap" "tclap.dir")

include_directories("../Vendor/win32/ffmpeg/include" "../Vendor/win32/rapidjson/include" "../Vendor/win32/spdlog/include" "../Vendor/win32/tclap/include")
target_link_libraries(${PROJECT_NAME} avcodec avformat avutil swresample swscale VideoEditorCore)

add_custom_target(copy_assets COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/../Resources ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/Resources)

file(GLOB ffmpeg_dlls "../Vendor/win32/ffmpeg/bin/*.dll")
foreach(ffmpeg_dll ${ffmpeg_dlls})
  add_custom_command(TARGET copy_assets POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${ffmpeg_dll} ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
endforeach()

message(STATUS "--------------------------------------------------")
get_cmake_property(_variableNames VARIABLES)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()