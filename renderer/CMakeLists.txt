SET (RENDERER_SOURCES
		renderer/HardwareInternal.h
		renderer/RendererConfig.h
		renderer/clipper.cpp
		renderer/draw.cpp
		renderer/gl.c
		renderer/globalvars.cpp
		renderer/instance.cpp
		renderer/opengl.cpp
		renderer/points.cpp
		renderer/setup.cpp
		renderer/transforms.cpp
		renderer/wgl.c)

IF(UNIX)
SET (RENDERER_SOURCES ${RENDERER_SOURCES} 
		renderer/lnxscreenmode.cpp)
ENDIF()

set (RENDERER_SOURCES ${RENDERER_SOURCES} PARENT_SCOPE)

# These are excluded.
#opengl.cpp
#renderer.cpp
#Direct3D.cpp
