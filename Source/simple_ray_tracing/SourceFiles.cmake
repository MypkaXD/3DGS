set (SimpleRayTracing_INC
	include/Camera.h
	include/Shader.h
)

set (SimpleRayTracing_SRC
	src/main.cpp
)

source_group("Source Files" FILES ${SimpleRayTracing_SRC})
source_group("Header Files" FILES ${SimpleRayTracing_INC})