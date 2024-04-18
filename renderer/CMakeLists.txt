SET (HEADERS 
		HardwareInternal.h
		RendererConfig.h)
SET (CPPS
		clipper.cpp
		draw.cpp
		gl.c
		globalvars.cpp
		instance.cpp
		opengl.cpp
		points.cpp
		setup.cpp
		transforms.cpp
		wgl.c)

IF(UNIX)

  SET (CPPS ${CPPS} lnxscreenmode.cpp)

ENDIF()

# These are excluded.
#opengl.cpp
#renderer.cpp
#Direct3D.cpp

ADD_LIBRARY(renderer STATIC ${HEADERS} ${CPPS})