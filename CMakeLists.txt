CMAKE_MINIMUM_REQUIRED( VERSION 3.11 )

MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)
# allow more human readable "if then else" constructs
SET( CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE )

PROJECT(Descent3)

function(add_hog_file HOG_NAME HOG_SRC_DIR)
	string( REPLACE "." "_" HOG_TARGET ${HOG_NAME} )
	
	#message(${HOG_SRC_DIR})
	
	#Iterate all the files in the hog source dir, for usage in a target.
	file(GLOB_RECURSE HOG_SOURCES ${HOG_SRC_DIR}/*)
	
	#Create the command that actually invokes hogdir to build the hogfile.
	add_custom_command( OUTPUT ${CMAKE_BINARY_DIR}/${HOG_NAME}
		COMMAND hogdir ${CMAKE_BINARY_DIR}/${HOG_NAME} ${HOG_SRC_DIR}
		DEPENDS hogdir )
		
	#Create the target that uses this command to build the hogfile.
	#From GZDoom: Touch the hogdir binary to ensure that this rule always rebuilds
	add_custom_target( ${HOG_TARGET} ALL
		COMMAND ${CMAKE_COMMAND} -E touch $<TARGET_FILE:hogdir>
		DEPENDS ${CMAKE_BINARY_DIR}/${HOG_NAME}
		SOURCES ${HOG_SOURCES})
endfunction()

IF (UNIX)
SET (D3_GAMEDIR "~/Descent3/")

  SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -Wno-write-strings -m32")
  SET(CMAKE_CXX_COMPILER "g++")
  SET(CMAKE_CXX_FLAGS "-O0 -g -Wno-write-strings -Wno-multichar -m32")
  SET(CMAKE_C_FLAGS "-O0 -g -m32")
  SET(CMAKE_C_COMPILER "gcc")
  SET(CMAKE_FIND_LIBRARY_PREFIXES "lib")
  SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
  FIND_PACKAGE( SDL REQUIRED )
  FIND_PACKAGE( SDL_image REQUIRED )
MESSAGE( "SDL Include Dir is " ${SDL_INCLUDE_DIR} )
ENDIF()

IF (UNIX AND NOT APPLE)
  MESSAGE("Building for Linux")

  ADD_DEFINITIONS( -D_DEBUG -D__LINUX__ -DLINUX -D_MAX_PATH=260 -D_MAX_FNAME=256 -D_REENRANT -DMONO -D__32BIT__ -DHAVEALLOCA_H -D_USE_OGL_ACTIVE_TEXTURES)
  SET(PLATFORM_INCLUDES "lib/linux" ${SDL_INCLUDE_DIR} )
ENDIF()

IF (APPLE)
	SET(D3_GAMEDIR "~/Descent3X")	
  MESSAGE("Building for MAC OSX")
  ADD_DEFINITIONS( -DMONO -D_DEBUG -D__LINUX__ -DLINUX -D_MAX_PATH=260 -D_MAX_FNAME=256 -D_REENRANT -DMACOSX=1 -D_USE_OGL_ACTIVE_TEXTURES)
  SET(PLATFORM_INCLUDES "lib/linux" ${SDL_INCLUDE_DIR} "/usr/X11/include" )
ENDIF()

IF (WIN32)
	set(USE_SDL OFF CACHE BOOL "Builds with SDL rather than the Win32 code.")
	SET(D3_GAMEDIR "c:/games/Descent3/")	
	set (CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "lib/win" "lib/win/directx" "thirdparty/OpenAL/libs/Win32") 
	SET(CMAKE_CXX_FLAGS_DEBUG "/Od /EHsc /RTC1 /MTd /MP /W3 /nologo /c /Zi /errorReport:prompt")
	SET(CMAKE_CXX_FLAGS_RELEASE "/O2 /FD /EHsc /MT /MP /W3 /nologo /c /Zi /errorReport:prompt")
	
	set(CMAKE_MODULE_LINKER_FLAGS "/SAFESEH:NO /SUBSYSTEM:WINDOWS /NODEFAULTLIB:LIBC")
	set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "/DEBUG")
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE " /DEBUG") #[ISB] Generate debug information
	ADD_DEFINITIONS (-DIS_WINDOWS -D_CRT_SECURE_NO_WARNINGS -DWIN32 -D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
	if (USE_SDL)
		find_package(sdl3 REQUIRED)
		add_definitions(-DSDL3)
	endif()
	
	SET(PLATFORM_INCLUDES "lib/win/directx" "lib/win" "thirdparty/OpenAL/include" )
	
	SET(CMAKE_FIND_LIBRARY_PREFIXES "")
	SET(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
	
	FIND_LIBRARY(DINPUT_LIBRARY NAMES dinput "${CMAKE_SOURCE_DIR}/lib/win" "${CMAKE_SOURCE_DIR}/lib/win/directx")
	FIND_LIBRARY(DXGUID_LIBRARY NAMES dxguid "${CMAKE_SOURCE_DIR}/lib/win" "${CMAKE_SOURCE_DIR}/lib/win/directx")
	#TODO: May need to find OpenAL64 eventually
	FIND_LIBRARY(OPENAL_LIBRARY NAMES openal32 "${CMAKE_SOURCE_DIR}/thirdparty/OpenAL/libs" "${CMAKE_SOURCE_DIR}/thirdparty/OpenAL/libs/Win32")
ENDIF ()

MESSAGE("Install will copy files to ${D3_GAMEDIR}")

include_directories("lib" "Descent3" "netcon/includes" ${PLATFORM_INCLUDES})

IF (WIN32)
	SET (PLATFORM_LIBS wsock32.lib winmm.lib Glu32.lib OpenGL32.lib dbghelp.lib wininet.lib
		${DINPUT_LIBRARY} ${DXGUID_LIBRARY} ${OPENAL_LIBRARY})
	SET (MIN_PLATFORM_LIBS wsock32.lib wininet.lib)
ENDIF()
IF (UNIX AND NOT APPLE)
	SET (PLATFORM_LIBS ${SDL_LIBRARY} caca asound audio esd aa directfb m dl GLU)
	SET (MIN_PLATFORM_LIBS ${PLATFORM_LIBS}) #todo
ENDIF()
IF (APPLE)
	SET (PLATFORM_LIBS ${SDL_LIBRARY} ncurses )
	SET (MIN_PLATFORM_LIBS ${PLATFORM_LIBS}) #todo
ENDIF()


#Hogdir stuff
add_subdirectory(tools)
add_subdirectory(hogfiles)

ADD_SUBDIRECTORY (AudioEncode)
ADD_SUBDIRECTORY (bitmap)
ADD_SUBDIRECTORY (cfile)
ADD_SUBDIRECTORY (d3music)

IF (WIN32)
	if(USE_SDL)
		ADD_SUBDIRECTORY (ddio_sdl)
		ADD_SUBDIRECTORY (sdl)
	else()
		ADD_SUBDIRECTORY (ddio_win)
		ADD_SUBDIRECTORY (win32)
	endif()
	ADD_SUBDIRECTORY (dd_sndlib)
ENDIF()

IF (UNIX)
ADD_SUBDIRECTORY(linux)
ADD_SUBDIRECTORY(dd_lnxsound)
ADD_SUBDIRECTORY(lnxcontroller)
ADD_SUBDIRECTORY(ddio_lnx)
ENDIF()

ADD_SUBDIRECTORY (ddio_common)
ADD_SUBDIRECTORY (fix)
ADD_SUBDIRECTORY (grtext)
ADD_SUBDIRECTORY (manage)
ADD_SUBDIRECTORY (mem)
ADD_SUBDIRECTORY (misc)
ADD_SUBDIRECTORY (model)
ADD_SUBDIRECTORY (module)
ADD_SUBDIRECTORY (movie)
ADD_SUBDIRECTORY (music)
ADD_SUBDIRECTORY (mve)
ADD_SUBDIRECTORY (networking)
ADD_SUBDIRECTORY (physics)
ADD_SUBDIRECTORY (renderer)
ADD_SUBDIRECTORY (rtperformance)
ADD_SUBDIRECTORY (sndlib)
ADD_SUBDIRECTORY (stream_audio)
ADD_SUBDIRECTORY (ui)
ADD_SUBDIRECTORY (unzip)
ADD_SUBDIRECTORY (vecmat)
# ADD_SUBDIRECTORY (libmve)
ADD_SUBDIRECTORY (md5)
ADD_SUBDIRECTORY (libacm)
ADD_SUBDIRECTORY (netgames)
ADD_SUBDIRECTORY (Descent3)
ADD_SUBDIRECTORY (netcon/inetfile)
#dependent on above
ADD_SUBDIRECTORY (netcon)

#[ISB] TODO: Needs fixing for new build system
#IF(UNIX)
#ADD_SUBDIRECTORY (scripts)
#ENDIF()

#From GZDOOM
add_custom_target( revision_check ALL
	COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_SOURCE_DIR}/tools/UpdateRevision.cmake" Descent3/gitinfo.h
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# set default cmake build type to Debug (None Debug Release RelWithDebInfo MinSizeRel)
IF( NOT CMAKE_BUILD_TYPE )
SET( CMAKE_BUILD_TYPE "Debug" )
ENDIF()

SET (DESCENT3_SOURCES
		${AUDIOENCODE_SORUCES}
		${BITMAP_SOURCES}
		${CFILE_SOURCES}
		${D3MUSIC_SOURCES}
		${DDIO_COMMON_SOURCES}
		${FIX_SOURCES}
		${GRTEXT_SOURCES}
		${MANAGE_SOURCES}
		${MEM_SOURCES}
		${MISC_SOURCES}
		${MODEL_SOURCES}
		${MODULE_SOURCES}
		${MOVIE_SOURCES}
		${MUSIC_SOURCES}
		${MVE_SOURCES}
		${NETWORKING_SOURCES}
		${PHYSICS_SOURCES}
		${RENDERER_SOURCES}
		${RTPERFORMANCE_SOURCES}
		${SNDLIB_SOURCES}
		${STREAM_AUDIO_SOURCES}
		${UI_SOURCES}
		${UNZIP_SOURCES}
		${VECMAT_SOURCES}
		${MD5_SOURCES}
		${LIBACM_SOURCES}
		${INETFILE_SOURCES}
		${MAIN_SOURCES})
		
IF (WIN32)
if (USE_SDL)
SET (DESCENT3_SOURCES
	${DESCENT3_SOURCES}
	${DDIO_SDL_SOURCES}
	${SDL_SOURCES}
	${DD_SNDLIB_SOURCES}
	Descent3.manifest)
else()
SET (DESCENT3_SOURCES
	${DESCENT3_SOURCES}
	${DDIO_WIN_SOURCES}
	${WIN32_SOURCES}
	Descent3.manifest)
endif()

#shared sources
SET (DESCENT3_SOURCES
	${DESCENT3_SOURCES}
	${DD_SNDLIB_SOURCES}
	Descent3.manifest)
ENDIF()

IF (UNIX)
SET (DESCENT3_SOURCES
	${DESCENT3_SOURCES}
	${LINUX_SOURCES}
	${DD_LNXSOUND_SOURCES}
	${DDIO_SOURCES})
ENDIF()

add_executable(PiccuEngine ${DESCENT3_SOURCES})
	
IF (WIN32)
	if (USE_SDL) #for convenience SDL will compile as a console app, and dump mono there
		target_link_options(PiccuEngine PUBLIC /SAFESEH:NO /NODEFAULTLIB:LIBC)
	else()
		target_link_options(PiccuEngine PUBLIC /SAFESEH:NO /SUBSYSTEM:WINDOWS /NODEFAULTLIB:LIBC)
	endif()
ENDIF()

IF (UNIX AND NOT APPLE)
	target_link_options(PiccuEngine PUBLIC /usr/lib/libpulse-simple.so.0)
ENDIF()
	
IF (APPLE)
	target_link_options(PiccuEngine PUBLIC -framework IOKit -framework Cocoa -framework OpenGL -framework Carbon)
ENDIF()

target_link_libraries(PiccuEngine ${PLATFORM_LIBS})
	
target_compile_definitions(PiccuEngine PUBLIC -DMONO "$<$<CONFIG:RELEASE>:RELEASE>")
target_compile_definitions(PiccuEngine PUBLIC -DMONO "$<$<CONFIG:MINSIZEREL>:RELEASE>")
target_compile_definitions(PiccuEngine PUBLIC -DMONO "$<$<CONFIG:RELWITHDEBINFO>:RELEASE>")

add_library(dmfc SHARED ${DMFC_SOURCES})
target_compile_definitions(dmfc PUBLIC -DOUTRAGE_VERSION -DDMFC_DLL)
target_include_directories(dmfc PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/netgames/includes")
add_dependencies(PiccuEngine revision_check piccuengine_hog TCP-IP Tracker dmfc)
	
install(TARGETS PiccuEngine DESTINATION ${D3_GAMEDIR})

source_group("AudioEncode" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/AudioEncode/.+")
source_group("bitmap" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/bitmap/.+")
source_group("cfile" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/cfile/.+")
source_group("d3music" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/d3music/.+")
source_group("dd_lnxsound" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/dd_lnxsound/.+")
source_group("dd_sndlib" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/dd_sndlib/.+")
source_group("ddio_common" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/ddio_common/.+")
source_group("ddio_lnx" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/ddio_lnx/.+")
source_group("ddio_win" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/ddio_win/.+")
source_group("main" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/Descent3/.+")
source_group("fix" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/fix/.+")
source_group("grtext" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/grtext/.+")
source_group("libacm" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/libacm/.+")
source_group("linux" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/linux/.+")
source_group("manage" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/manage/.+")
source_group("md5" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/md5/.+")
source_group("mem" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/mem/.+")
source_group("misc" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/misc/.+")
source_group("model" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/model/.+")
source_group("module" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/module/.+")
source_group("movie" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/movie/.+")
source_group("music" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/music/.+")
source_group("mve" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/mve/.+")
source_group("networking" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/networking/.+")
source_group("netcon" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/netcon/.+")
source_group("netcon/inetfile" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/netcon/inetfile/.+")
source_group("netcon/lanclient" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/netcon/lanclient/.+")
source_group("netgames" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/netgames/.+")
source_group("netgames/dmfc" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/netgames/dmfc/.+")
source_group("physics" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/physics/.+")
source_group("renderer" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/renderer/.+")
source_group("rtperformance" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/rtperformance/.+")
source_group("sndlib" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/sndlib/.+")
source_group("stream_audio" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/stream_audio/.+")
source_group("ui" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/ui/.+")
source_group("unzip" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/unzip/.+")
source_group("vecmat" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/vecmat/.+")
source_group("win32" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/win32/.+")
source_group("sdl" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/sdl/.+")

