if (WIN32) #still embed the icon
	SET (SDL3_SOURCES
		sdl/sdlapp.cpp
		sdl/sdlcontroller.cpp
		sdl/sdlcon.cpp
		sdl/sdldatabase.cpp
		sdl/sdlmono.cpp
		sdl/sdlcontroller.h
		sdl/registry.cpp
		sdl/registry.h
		win32/win32res.h
		win32/win32res.rc
		win32/piccu_station_256.ico
		win32/piccu_station_32.ico
		PARENT_SCOPE)
else()
	SET (SDL3_SOURCES
		sdl/sdlapp.cpp
		sdl/sdlcontroller.cpp
		sdl/sdlcon.cpp
		sdl/sdldatabase.cpp
		sdl/sdlmono.cpp
		sdl/sdlcontroller.h
		sdl/registry.cpp
		sdl/registry.h
		PARENT_SCOPE)
endif()
