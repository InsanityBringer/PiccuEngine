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
		renderer/gl_mesh.cpp
		renderer/gl_mesh.h
		renderer/gl_shader.cpp
		renderer/gl_shader.h
		renderer/gl_shadersource.cpp
		
		#legacy GL renderer
		renderer/gl1_draw.cpp
		renderer/gl1_framebuffer.cpp
		renderer/gl1_image.cpp
		renderer/gl1_init.cpp
		renderer/gl1_local.h
		renderer/gl1_main.cpp
		renderer/gl1_shader.cpp
		
		#renderer switchboard
		renderer/renderer.cpp
		renderer/IRenderer.h
		
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

set (RENDERER_SOURCES ${RENDERER_SOURCES} PARENT_SCOPE)
