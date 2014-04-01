########### LX200 Basic #############
add_executable(indi_lx200basic
    ${indimain_SRCS}
    lx200driver.c
    lx200basic.cpp
    ${lx200basic_SRCS}
    ${liblilxml_SRCS}
    ${libindicom_SRCS})

target_link_libraries(indi_lx200basic ${NOVA_LIBRARIES} ${NOVA_LIBRARIES})

install(TARGETS indi_lx200basic RUNTIME DESTINATION bin )

#################################################################################

########### LX200 Generic ###########

add_executable(indi_lx200generic
   lx200driver.c
   lx200autostar.cpp
   lx200_16.cpp
   lx200gps.cpp
   lx200generic.cpp
   lx200classic.cpp
   lx200apdriver.c
   lx200ap.cpp
)

target_link_libraries(indi_lx200generic indidriver)

install(TARGETS indi_lx200generic RUNTIME DESTINATION bin )

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/make_lx200generic_symlink.cmake
"exec_program(\"${CMAKE_COMMAND}\" ARGS -E create_symlink ${BIN_INSTALL_DIR}/indi_lx200generic \$ENV{DESTDIR}${BIN_INSTALL_DIR}/indi_lx200classic)\n
exec_program(\"${CMAKE_COMMAND}\" ARGS -E create_symlink ${BIN_INSTALL_DIR}/indi_lx200generic \$ENV{DESTDIR}${BIN_INSTALL_DIR}/indi_lx200autostar)\n
exec_program(\"${CMAKE_COMMAND}\" ARGS -E create_symlink ${BIN_INSTALL_DIR}/indi_lx200generic \$ENV{DESTDIR}${BIN_INSTALL_DIR}/indi_lx200_16)\n
exec_program(\"${CMAKE_COMMAND}\" ARGS -E create_symlink ${BIN_INSTALL_DIR}/indi_lx200generic \$ENV{DESTDIR}${BIN_INSTALL_DIR}/indi_lx200gps)\n
exec_program(\"${CMAKE_COMMAND}\" ARGS -E create_symlink ${BIN_INSTALL_DIR}/indi_lx200generic \$ENV{DESTDIR}${BIN_INSTALL_DIR}/indi_lx200ap)\n
")
set_target_properties(indi_lx200generic PROPERTIES POST_INSTALL_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/make_lx200generic_symlink.cmake)
#################################################################################

########### LX200 Generic Legacy ###########
add_executable(indi_lx200genericlegacy
    ${indimain_SRCS}
    lx200driver.c
    lx200genericlegacy.cpp
    lx200fs2.cpp
    ${liblilxml_SRCS}
    ${libindicom_SRCS})

target_link_libraries(indi_lx200genericlegacy ${NOVA_LIBRARIES} ${M_LIB} )

install(TARGETS indi_lx200genericlegacy RUNTIME DESTINATION bin )

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/make_lx200genericlegacy_symlink.cmake
"exec_program(\"${CMAKE_COMMAND}\" ARGS -E create_symlink ${BIN_INSTALL_DIR}/indi_lx200genericlegacy \$ENV{DESTDIR}${BIN_INSTALL_DIR}/indi_lx200fs2)\n")
set_target_properties(indi_lx200genericlegacy PROPERTIES POST_INSTALL_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/make_lx200genericlegacy_symlink.cmake)
#################################################################################

########### Celestron GPS ############
add_executable(indi_celestron_gps
    celestronprotocol.c
    celestrongps.cpp
)

target_link_libraries(indi_celestron_gps indidriver)

install(TARGETS indi_celestron_gps RUNTIME DESTINATION bin )

#################################################################################

########### Takahashi Temma ##########
add_executable(indi_temma
    ${indimain_SRCS}
    temmadriver.c
    ${liblilxml_SRCS}
    ${libindicom_SRCS})

target_link_libraries(indi_temma  ${NOVA_LIBRARIES} ${M_LIB} )

install(TARGETS indi_temma RUNTIME DESTINATION bin )
#################################################################################

########### Sky Commander #############
add_executable(indi_skycommander
    ${indimain_SRCS}
    lx200driver.c
    skycommander.c
    ${liblilxml_SRCS}
    ${libindicom_SRCS})

target_link_libraries(indi_skycommander  ${NOVA_LIBRARIES} ${M_LIB} )

install(TARGETS indi_skycommander  RUNTIME DESTINATION bin )

#################################################################################

########### Intelliscope ###############
add_executable(indi_intelliscope
    ${indimain_SRCS}
    lx200driver.c
    intelliscope.c
    ${liblilxml_SRCS}
    ${libindicom_SRCS})

target_link_libraries(indi_intelliscope ${NOVA_LIBRARIES} ${M_LIB} )

install(TARGETS indi_intelliscope RUNTIME DESTINATION bin )

########### Syncscan ###############
add_executable(indi_synscan synscanmount.cpp)
target_link_libraries(indi_synscan indidriver)

install(TARGETS indi_synscan RUNTIME DESTINATION bin )

########### Magellan I #############
add_executable(indi_magellan1
    ${indimain_SRCS}
    magellandriver.c
    magellan1.cpp
    ${liblilxml_SRCS}
    ${libindicom_SRCS})

target_link_libraries(indi_magellan1 ${NOVA_LIBRARIES})

install(TARGETS indi_magellan1 RUNTIME DESTINATION bin )

########### IEQ45 #############
###Handheld 8406 version
add_executable(indi_ieq45_8406
    ${indimain_SRCS}
    ieq45driver8406.c
    ieq45.cpp
    ${liblilxml_SRCS}
    ${libindicom_SRCS})

target_link_libraries(indi_ieq45_8406 ${NOVA_LIBRARIES})

install(TARGETS indi_ieq45_8406 RUNTIME DESTINATION bin )

###Handheld 8407 version
add_executable(indi_ieq45_8407
    ${indimain_SRCS}
    ieq45driver8407.c
    ieq45.cpp
    ${liblilxml_SRCS}
    ${libindicom_SRCS})

target_link_libraries(indi_ieq45_8407 ${NOVA_LIBRARIES})

install(TARGETS indi_ieq45_8407 RUNTIME DESTINATION bin )

########### Telescope Simulator ##############
add_executable(indi_simulator_telescope telescope_simulator.cpp)
target_link_libraries(indi_simulator_telescope indidriver)
install(TARGETS indi_simulator_telescope RUNTIME DESTINATION bin )

########### CCD Simulator ##############
if (CFITSIO_FOUND)

add_executable(indi_simulator_ccd ../ccd/ccd_simulator.cpp)
target_link_libraries(indi_simulator_ccd indidriver)
install(TARGETS indi_simulator_ccd RUNTIME DESTINATION bin )

endif (CFITSIO_FOUND)


