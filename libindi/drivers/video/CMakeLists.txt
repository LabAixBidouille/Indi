########### STV #######################
if (CFITSIO_FOUND)

add_executable(indi_sbig_stv
    ${indimain_SRCS}
    stvdriver.c
    stv.c
    ${liblilxml_SRCS}
    ${libindicom_SRCS})

target_link_libraries(indi_sbig_stv ${NOVA_LIBRARIES} ${CFITSIO_LIBRARIES} ${M_LIB} ${ZLIB_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS indi_sbig_stv RUNTIME DESTINATION bin )

endif(CFITSIO_FOUND)

#################################################################################

### Meade Lunar Planetary Imager ########
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
if (CFITSIO_FOUND)

ADD_DEFINITIONS(-DHAVE_LINUX_VIDEODEV2_H)

add_executable(indi_meade_lpi
	${indimain_SRCS}
	v4ldriver.cpp
	indi_lpi.cpp
    ${libwebcam_SRCS}
    ${liblilxml_SRCS}
    ${libindicom_SRCS})

target_link_libraries(indi_meade_lpi z ${JPEG_LIBRARY} ${NOVA_LIBRARIES} ${CFITSIO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS indi_meade_lpi RUNTIME DESTINATION bin )

endif (CFITSIO_FOUND)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

#################################################################################

########### V4L Philips ##############
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
if (CFITSIO_FOUND)

add_executable(indi_v4l_philips
	${indimain_SRCS}
	v4ldriver.cpp
	v4lphilips.cpp
	indi_philips.cpp
    ${libwebcam_SRCS}
    ${liblilxml_SRCS}
    ${libindicom_SRCS})

target_link_libraries(indi_v4l_philips ${JPEG_LIBRARY} ${M_LIB} ${ZLIB_LIBRARY} ${NOVA_LIBRARIES} ${CFITSIO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS indi_v4l_philips RUNTIME DESTINATION bin )

endif (CFITSIO_FOUND)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

#################################################################################

########### Old Generic V4L Driver ###############
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
if (CFITSIO_FOUND)

add_executable(indi_v4l_legacy
	${indimain_SRCS}
	v4ldriver.cpp
	indi_v4l.cpp
    ${libwebcam_SRCS}
    ${liblilxml_SRCS}
    ${libindicom_SRCS})

target_link_libraries(indi_v4l_legacy ${JPEG_LIBRARY} ${M_LIB} ${ZLIB_LIBRARY} ${NOVA_LIBRARIES} ${CFITSIO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS indi_v4l_legacy RUNTIME DESTINATION bin )

########### New INDI::CCD V4L Driver ###############

add_executable(indi_v4l2_ccd
    v4l2driver.cpp
    indi_v4l2driver.cpp
    ${libwebcam_SRCS}
    ../../libs/lx/Lx.cpp)

target_link_libraries(indi_v4l2_ccd ${JPEG_LIBRARY} indidriver)

install(TARGETS indi_v4l2_ccd RUNTIME DESTINATION bin )


endif (CFITSIO_FOUND)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

