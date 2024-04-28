SET (DMFC_SOURCES 
	netgames/dmfc/dmfcbase.cpp
	netgames/dmfc/dmfccfg.cpp
	netgames/dmfc/dmfcclient.cpp
	netgames/dmfc/dmfcdllinit.h
	netgames/dmfc/dmfcfunctions.cpp
	netgames/dmfc/dmfchudmessages.cpp
	netgames/dmfc/dmfcinputcommand.cpp 
	netgames/dmfc/dmfcinputcommands.h
	netgames/dmfc/dmfcinternal.h
	netgames/dmfc/dmfcmenu.cpp 
	netgames/dmfc/dmfcpackets.cpp 
	netgames/dmfc/dmfcpinfo.cpp 
	netgames/dmfc/dmfcprecord.cpp 
	netgames/dmfc/dmfcremote.cpp 
	netgames/dmfc/dmfcserver.cpp 
	netgames/dmfc/dmfcstats.cpp 
	netgames/dmfc/dmfctimer.cpp 
	netgames/dmfc/dmfcui.cpp 
	netgames/dmfc/dmfcvirtual.cpp 
	netgames/dmfc/idmfc.cpp 
	netgames/dmfc/encryption.cpp 
	netgames/dmfc/encryption.h
	netgames/dmfc/dmfcinterface.cpp)
  
if (WIN32)
	set (DMFC_SOURCES
		${DMFC_SOURCES}
		netgames/dmfc/dmfc.def)
endif()

#is there a way to make scoping suck less in CMake?
set (DMFC_SOURCES
	${DMFC_SOURCES}
	PARENT_SCOPE)

