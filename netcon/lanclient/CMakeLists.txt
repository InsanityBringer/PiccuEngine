SET (HEADERS lanclient.h)
SET (CPPS 
lanclient.cpp  
../inetfile/CFtp.cpp
../inetfile/Chttpget.cpp
../inetfile/inetgetfile.cpp
)

SET (NETGAME_MODULE "TCP-IP")

ADD_LIBRARY(${NETGAME_MODULE} SHARED ${CPPS} ${HEADERS})
set_target_properties(${NETGAME_MODULE} PROPERTIES PREFIX "")
set_target_properties(${NETGAME_MODULE} PROPERTIES SUFFIX ".piccucon")
set_target_properties(${NETGAME_MODULE} PROPERTIES OUTPUT_NAME "Direct TCP~IP")
install(TARGETS ${NETGAME_MODULE} DESTINATION "${D3_GAMEDIR}online/")
target_link_libraries(${NETGAME_MODULE} ${MIN_PLATFORM_LIBS})
