set (GaussiansViewer_INC
	include/AABB.h
	include/App.h
	include/BVH.h
	include/Camera.h
	include/Ellipsoid.h
	include/SceneLoader.h
	include/Shader.h
	include/UV.h
)

set (GaussiansViewer_SRC
	src/main.cpp
)

source_group("Source Files" FILES ${GaussiansViewer_SRC})
source_group("Header Files" FILES ${GaussiansViewer_INC})