SET (RENDERER_SOURCES
		renderer/HardwareInternal.h
		renderer/RendererConfig.h
		renderer/globalvars.cpp
		renderer/gl_draw.cpp
		renderer/gl_framebuffer.cpp
		renderer/gl_image.cpp
		renderer/gl_init.cpp
		renderer/gl_local.h
		renderer/gl_main.cpp
		renderer/gl_shader.cpp
		renderer/gl_shader.h
		renderer/gl_shadersource.cpp
		
		#GLAD gl loader
		renderer/gl.c
		renderer/wgl.c
		
		#3D system
		renderer/clipper.cpp
		renderer/draw.cpp
		renderer/instance.cpp
		renderer/points.cpp
		renderer/setup.cpp
		renderer/transforms.cpp)

IF(UNIX)
SET (RENDERER_SOURCES ${RENDERER_SOURCES} 
		renderer/lnxscreenmode.cpp)
ENDIF()

set (RENDERER_SOURCES ${RENDERER_SOURCES} PARENT_SCOPE)

# These are excluded.
#opengl.cpp
#renderer.cpp
#Direct3D.cpp
