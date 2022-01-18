CMAKE_MINIMUM_REQUIRED(VERSION 3.16.3)

PROJECT(GLKit VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)


file(GLOB HEADER_FILE "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "ThirdParty/*.h")
include_directories("./" "../Vendor/win32/glm" "../" "../Vendor/win32/glad/include" "../Vendor/win32/glfw/include")

aux_source_directory("./" SRC)

add_library(${PROJECT_NAME} STATIC ${SRC} ${HEADER_FILE})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC} ${HEADER_FILE})


add_subdirectory("../Vendor/win32/glm" "glm.dir")
target_link_libraries(${PROJECT_NAME} PRIVATE glm)


message(STATUS "--------------------------------------------------")
get_cmake_property(_variableNames VARIABLES)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()