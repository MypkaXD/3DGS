cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME INTERSECT_RAY_ELLIPSOID)
project(${PROJECT_NAME})

set(SDK_DIR ${CMAKE_SOURCE_DIR}/sdk)

add_executable(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/src/main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${SDK_DIR}/glm-master/my_install/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${SDK_DIR}/geomView-main/my_install/include)

find_package(OpenGL REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE ${SDK_DIR}/geomView-main/my_install/lib/libgeom_view.lib OpenGL::GL)