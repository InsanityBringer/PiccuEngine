SET (HEADERS trackclient.h)
SET (CPPS 
trackclient.cpp  
../inetfile/CFtp.cpp
../inetfile/Chttpget.cpp
../inetfile/inetgetfile.cpp
)

SET (NETGAME_MODULE "Tracker")

ADD_LIBRARY(${NETGAME_MODULE} SHARED ${CPPS} ${HEADERS})
set_target_properties(${NETGAME_MODULE} PROPERTIES PREFIX "")
set_target_properties(${NETGAME_MODULE} PROPERTIES SUFFIX ".piccucon")
install(TARGETS ${NETGAME_MODULE} DESTINATION "${D3_GAMEDIR}online/")
target_link_libraries(${NETGAME_MODULE} ${MIN_PLATFORM_LIBS})
