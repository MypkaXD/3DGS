set (GaussiansViewer_INC
	include/Ellipsoid.h
	include/Shader.h
	include/AABB.h
	include/Camera.h
)

set (GaussiansViewer_SRC
	src/main.cpp
)

source_group("Source Files" FILES ${GaussiansViewer_SRC})
source_group("Header Files" FILES ${GaussiansViewer_INC})