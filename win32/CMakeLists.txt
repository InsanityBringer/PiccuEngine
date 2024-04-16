SET (HEADERS
		win32os.h
		wincontroller.h)
SET (CPPS
		winapp.cpp
		wincontroller.cpp
		wincon.cpp
		wincore.cpp
		windata.cpp
		windebug.cpp
		winmono.cpp
		wintask.cpp)

ADD_LIBRARY(win32 STATIC ${HEADERS} ${CPPS})