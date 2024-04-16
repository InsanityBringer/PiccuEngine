SET (HEADERS 
		ds3dlib_internal.h)
SET (CPPS
		ddsoundload.cpp
		llsopenal.cpp
		ssl_lib.cpp)

ADD_LIBRARY(dd_sndlib STATIC ${HEADERS} ${CPPS})