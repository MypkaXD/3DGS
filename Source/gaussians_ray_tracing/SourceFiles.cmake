set (GaussianRayTracing_INC
	include/AABB.h
	include/App.h
	include/BVH.h
	include/Camera.h
	include/Ellipsoid.h
	include/SceneLoader.h
	include/Shader.h
	include/UV.h
)

set (GaussianRayTracing_SRC
	src/main.cpp
)

source_group("Source Files" FILES ${GaussianRayTracing_SRC})
source_group("Header Files" FILES ${GaussianRayTracing_INC})