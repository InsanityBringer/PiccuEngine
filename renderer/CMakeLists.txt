SET (HEADERS 
		dyna_gl.h
		HardwareInternal.h
		RendererConfig.h)
SET (CPPS
		clipper.cpp
		draw.cpp
		globalvars.cpp
		instance.cpp
		opengl.cpp
		points.cpp
		setup.cpp
		transforms.cpp)

IF(UNIX)

  SET (CPPS ${CPPS} lnxscreenmode.cpp)

ENDIF()

# These are excluded.
#opengl.cpp
#renderer.cpp
#Direct3D.cpp

ADD_LIBRARY(renderer STATIC ${HEADERS} ${CPPS})