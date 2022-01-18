CMAKE_MINIMUM_REQUIRED(VERSION 3.16.3)

PROJECT(VideoEditor VERSION 0.1.0 LANGUAGES CXX)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VideoEditor)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_GNUtoMS OFF)
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})


link_directories("build/SDL2-2.0.18.dir/$<CONFIG>" "build/GLKit.dir/$<CONFIG>" "../Vendor/win32/ffmpeg/lib"  "${CMAKE_CURRENT_BINARY_DIR}/glfw.dir/src/$<CONFIG>" "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>" REQUIRED)

include_directories("../")
set(MAIN_SRC "main.cpp" )
add_executable(${PROJECT_NAME} ${MAIN_SRC})

add_dependencies(VideoEditor spdlog)
add_dependencies(VideoEditor VideoEditorCore)
add_dependencies(VideoEditor GLKit)
add_dependencies(VideoEditor glm_dummy)
add_dependencies(VideoEditor glfw)
add_dependencies(VideoEditor SDL2-static)
add_dependencies(VideoEditor SDL2main)

add_subdirectory("../Core" "VideoEditorCore.dir")
add_subdirectory("../GLKit" "GLKit.dir")

SET(SDL_LIBC ON CACHE BOOL "")
SET(SDL_SHARED OFF CACHE BOOL "")
add_subdirectory("../Vendor/win32/SDL2-2.0.18" "SDL2-2.0.18.dir")

file(GLOB imgui_Src "../Vendor/win32/imgui/*.cpp")
file(GLOB imgui_Header "../Vendor/win32/imgui/*.h")
add_library(imgui STATIC 
${imgui_Src} 
${imgui_Header}
"../Vendor/win32/imgui/backends/imgui_impl_glfw.cpp"
"../Vendor/win32/imgui/backends/imgui_impl_glfw.h"
"../Vendor/win32/imgui/backends/imgui_impl_opengl3.cpp"
"../Vendor/win32/imgui/backends/imgui_impl_opengl3.h"
"../Vendor/win32/imgui/backends/imgui_impl_opengl3_loader.h")

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory("../Vendor/win32/glfw" "glfw.dir")

file(GLOB_RECURSE glad_Src "../Vendor/win32/glad/**.c")
file(GLOB glad_Header "../Vendor/win32/glad/*.h")
add_library(glad STATIC ${glad_Src} ${glad_Header})

find_package(OpenGL REQUIRED)

include_directories(
"../Vendor/win32/ffmpeg/include"
"../Vendor/win32/rapidjson/include" 
"../Vendor/win32/spdlog/include" 
"../Vendor/win32/imgui"
"../Vendor/win32/glad/include"
"../Vendor/win32/glfw/include"
"../GLKit"
"../Vendor/win32/glm"
"../Vendor/win32/SDL2-2.0.18/include"
)
target_link_libraries(${PROJECT_NAME} avcodec avformat avutil swresample swscale VideoEditorCore GLKit imgui OpenGL::GL glfw3 glad SDL2d SDL2maind winmm version setupapi)

set_property(TARGET ${PROJECT_NAME}  PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set_property(TARGET glfw  PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set_property(TARGET imgui  PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set_property(TARGET VideoEditorCore  PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set_property(TARGET spdlog  PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set_property(TARGET GLKit  PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set_property(TARGET SDL2-static  PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set_property(TARGET SDL2main  PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set_property(TARGET glm_dummy  PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set_property(TARGET glad  PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_custom_target(copy_assets COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_LIST_DIR}/../Resources" "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/Resources")

file(GLOB ffmpeg_dlls "../Vendor/win32/ffmpeg/bin/*.dll")
foreach(ffmpeg_dll ${ffmpeg_dlls})
  add_custom_command(TARGET copy_assets POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${ffmpeg_dll} ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
endforeach()

message(STATUS "--------------------------------------------------")
get_cmake_property(_variableNames VARIABLES)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()