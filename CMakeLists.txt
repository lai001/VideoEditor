CMAKE_MINIMUM_REQUIRED(VERSION 3.20.5)

PROJECT(VideoEditor VERSION 1.0 LANGUAGES CXX)

IF (WIN32)
	MESSAGE(STATUS "Current platform is WIN32")
ELSEIF (APPLE)
	MESSAGE(STATUS "Current platform is Apple")
ELSEIF (LINUX)
	MESSAGE(FATAL_ERROR "Not test in LINUX")
ENDIF ()

set(QT_DIR "C:/Qt/Qt5.12.10/5.12.10" CACHE FILEPATH "")

set(QT_MODULE_CMAKE_SEARCH_DIR "${QT_DIR}/mingw73_64/lib/cmake" CACHE FILEPATH "")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

#set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_GNUtoMS OFF)
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_PREFIX_PATH ${QT_MODULE_CMAKE_SEARCH_DIR})

if(NOT DEFINED DOSFLAG)
set(DOSFLAG "")
endif()

include_directories("./Core" "./Core/Time" "./Core/Track" "./Core/Utility" "./Core/Time" "./UI" "./Test")

IF (WIN32)
include_directories("./Vendor/win32/ffmpeg/include" "./Vendor/win32/rapidjson/include")
ELSEIF (APPLE)
include_directories("./Vendor/apple/ffmpeg/include" "./Vendor/apple/rapidjson/include")
ENDIF ()

aux_source_directory("./Core" CORE_SRC)
aux_source_directory("./Core/Time" CORE_TIME_SRC)
aux_source_directory("./Core/Track" CORE_TRACK_SRC)
aux_source_directory("./Core/Utility" CORE_UTILITY_SRC)
aux_source_directory("./UI" CORE_UI_SRC)
aux_source_directory("./Test" TEST_SRC)
aux_source_directory("./" MAIN_SRC)

find_package(Qt5 COMPONENTS Multimedia Core Widgets REQUIRED)

IF (WIN32)
link_directories("./Vendor/win32/ffmpeg/lib" REQUIRED)
ELSEIF (APPLE)
link_directories("./Vendor/apple/ffmpeg/lib" REQUIRED)
ENDIF ()

add_executable(${PROJECT_NAME} ${DOSFLAG} ${CORE_SRC} ${CORE_TIME_SRC}  ${CORE_TRACK_SRC} ${CORE_TRACK_SRC} ${CORE_UTILITY_SRC} ${CORE_UI_SRC} ${TEST_SRC} ${MAIN_SRC})
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Multimedia Qt5::Widgets avcodec avformat avutil swresample swscale)
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_BINARY_DIR}/Test_autogen/include_Debug")

if(CMAKE_BUILD_TYPE STREQUAL Debug)
add_custom_command(
  TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND  ${QT_DIR}/mingw73_64/bin/windeployqt.exe ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.exe
  )
else()
add_custom_command(
TARGET ${PROJECT_NAME}
POST_BUILD
COMMAND  ${QT_DIR}/mingw73_64/bin/windeployqt.exe --release ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.exe
)
endif()

add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/Mat ${CMAKE_CURRENT_BINARY_DIR}/Mat
)
add_dependencies(${PROJECT_NAME} copy_assets)

IF (WIN32)
file(GLOB ffmpeg_dlls "./Vendor/win32/ffmpeg/bin/*.dll")
ELSEIF (APPLE)
file(GLOB ffmpeg_dlls "./Vendor/apple/ffmpeg/bin/*.dylib")
ENDIF ()

foreach(ffmpeg_dll ${ffmpeg_dlls})
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E
                    copy ${ffmpeg_dll} ${CMAKE_CURRENT_BINARY_DIR})
endforeach()
