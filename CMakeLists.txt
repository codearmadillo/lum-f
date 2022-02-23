#######################################################
### Configuration
#######################################################
cmake_minimum_required(VERSION 3.20)
project(Lumiere)

set(CMAKE_CXX_STANDARD 17)
set(STBI_NO_LINEAR TRUE)

#######################################################
### Add executable files
#######################################################
add_executable(${CMAKE_PROJECT_NAME} main.cpp include/lumiere/lua.h include/lumiere/utils/string.h include/lumiere/core.h include/lumiere/logger.h)

#######################################################
### Add subdirectories
#######################################################
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glew)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/lua)

#######################################################
### Include directories
#######################################################
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw/include)
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glew/include)
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb)

#######################################################
### Link libraries
#######################################################
target_link_libraries(${CMAKE_PROJECT_NAME} glfw libglew_static lua_static)