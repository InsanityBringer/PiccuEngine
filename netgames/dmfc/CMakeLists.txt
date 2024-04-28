SET (HEADERS dmfcdllinit.h
  dmfcinputcommands.h
  dmfcinternal.h
  encryption.h
)

SET (CPPS 
  dmfcbase.cpp
  dmfccfg.cpp
  dmfcclient.cpp
  dmfcfunctions.cpp
  dmfchudmessages.cpp
  dmfcinputcommand.cpp 
  dmfcmenu.cpp 
  dmfcpackets.cpp 
  dmfcpinfo.cpp 
  dmfcprecord.cpp 
  dmfcremote.cpp 
  dmfcserver.cpp 
  dmfcstats.cpp 
  dmfctimer.cpp 
  dmfcui.cpp 
  dmfcvirtual.cpp 
  idmfc.cpp 
  encryption.cpp 
  dmfcinterface.cpp)
  
if (WIN32)
	set (CPPS
		${CPPS}
		dmfc.def)
endif()

ADD_DEFINITIONS(-DOUTRAGE_VERSION -DDMFC_DLL)

ADD_LIBRARY(dmfc SHARED ${HEADERS} ${CPPS})
