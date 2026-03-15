cmake_minimum_required(VERSION 4.2)

set(PROJECT_NAME GaussiansViewer)

SET(IMGUI_INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sdk/imgui-master CACHE PATH "Set Dear ImGui dir.")
SET(GLFW_INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sdk/glfw-master/myinstall CACHE PATH "Set GLFW dir.")
SET(GLAD_INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sdk/glad CACHE PATH "Set GLAD dir.")
SET(GLM_INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sdk/glm-master CACHE PATH "Set GLM dir.")

project(${PROJECT_NAME})

include(SourceFiles.cmake)

add_executable(${PROJECT_NAME} ${GaussiansViewer_SRC} ${GaussiansViewer_INC})

set(INCLUDES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES_DIR})

set(glm_DIR ${GLM_INSTALL_DIR}/myinstall/share/glm)
find_package(glm CONFIG REQUIRED)

add_library(3rdparty STATIC 
#Dear ImGui
	${IMGUI_INSTALL_DIR}/imgui.cpp
    ${IMGUI_INSTALL_DIR}/imgui_draw.cpp
    ${IMGUI_INSTALL_DIR}/imgui_widgets.cpp
    ${IMGUI_INSTALL_DIR}/imgui_tables.cpp
    ${IMGUI_INSTALL_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_INSTALL_DIR}/backends/imgui_impl_opengl3.cpp
#GLAD
	${GLAD_INSTALL_DIR}/src/glad.c
)
	
target_include_directories(3rdparty PUBLIC
	${IMGUI_INSTALL_DIR}
	${IMGUI_INSTALL_DIR}/backends
	${GLFW_INSTALL_DIR}/include
	${GLAD_INSTALL_DIR}/include
)

target_link_libraries(3rdparty PUBLIC 
	${GLFW_INSTALL_DIR}/lib/glfw3.lib
	glm::glm-header-only
)

target_link_libraries(${PROJECT_NAME} PUBLIC 3rdparty)
