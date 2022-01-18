CMAKE_MINIMUM_REQUIRED(VERSION 3.16.3)

PROJECT(VideoEditor VERSION 0.1.0 LANGUAGES CXX)

IF (WIN32)
	MESSAGE(STATUS "Current platform is WIN32")
ELSEIF (APPLE)
	MESSAGE(FATAL_ERROR "")
ELSEIF (LINUX)
	MESSAGE(FATAL_ERROR "")
ENDIF ()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_GNUtoMS OFF)
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})

if(NOT DEFINED DOSFLAG)
	set(DOSFLAG "")
endif()

set(QT_DIR "C:/Qt/Qt5.12.10/5.12.10" CACHE FILEPATH "")
set(QT_MODULE_CMAKE_SEARCH_DIR "${QT_DIR}/mingw73_64/lib/cmake" CACHE FILEPATH "")
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_PREFIX_PATH ${QT_MODULE_CMAKE_SEARCH_DIR})
set(autogen_include_directories "${CMAKE_BINARY_DIR}/Test_autogen/include_Debug")

find_package(Qt5 COMPONENTS Multimedia Core Widgets REQUIRED)

include_directories("./" "UI")
aux_source_directory("UI" CORE_UI_SRC)
aux_source_directory("./" SRC)
set(MAIN_SRC "main.cpp")

include_directories("../Vendor/win32/ffmpeg/include" "../Vendor/win32/rapidjson/include")
link_directories("../Vendor/win32/ffmpeg/lib" REQUIRED)


add_executable(${PROJECT_NAME} ${DOSFLAG} 
${CORE_SRC} ${CORE_TIME_SRC}  ${CORE_TRACK_SRC} ${CORE_TRACK_SRC} ${CORE_UTILITY_SRC} ${CORE_UI_SRC} ${MAIN_SRC} ${SRC})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES 
${CORE_SRC} ${CORE_TIME_SRC}  ${CORE_TRACK_SRC} ${CORE_TRACK_SRC} ${CORE_UTILITY_SRC} ${CORE_UI_SRC} ${MAIN_SRC} ${SRC}
)

target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Multimedia Qt5::Widgets avcodec avformat avutil swresample swscale spdlog)
target_include_directories(${PROJECT_NAME} PRIVATE autogen_include_directories)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
	add_custom_command( TARGET ${PROJECT_NAME} POST_BUILD COMMAND  ${QT_DIR}/mingw73_64/bin/windeployqt.exe ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.exe)
else()
	add_custom_command( TARGET ${PROJECT_NAME} POST_BUILD COMMAND  ${QT_DIR}/mingw73_64/bin/windeployqt.exe --release ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.exe)
endif()

add_custom_target(copy_assets COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/../Resources ${CMAKE_CURRENT_BINARY_DIR}/../Resources)
add_dependencies(${PROJECT_NAME} copy_assets)

file(GLOB ffmpeg_dlls "../Vendor/win32/ffmpeg/bin/*.dll")
foreach(ffmpeg_dll ${ffmpeg_dlls})
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${ffmpeg_dll} ${CMAKE_CURRENT_BINARY_DIR})
endforeach()


message(STATUS "--------------------------------------------------")
get_cmake_property(_variableNames VARIABLES)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()