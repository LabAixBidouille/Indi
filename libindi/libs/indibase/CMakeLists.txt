##################################################
########## INDI Default Driver Library ###########
# To link with main() and indibase classes  ######
##################################################
set (indidriver_SRCS
        basedevice.cpp
        defaultdevice.cpp
        indiproperty.cpp
        indiccd.cpp
        inditelescope.cpp
        indifilterwheel.cpp
        indifocuserinterface.cpp
        indifocuser.cpp
        indiusbdevice.cpp
        indiguiderinterface.cpp
        indifilterinterface.cpp
        indilogger.cpp
        indicontroller.cpp
    )

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
add_library(indidriver SHARED ${libindicom_SRCS} ${liblilxml_SRCS} ${indimain_SRCS} ${indidriver_SRCS} ${libwebcam_SRCS})
target_link_libraries(indidriver ${LIBUSB_1_LIBRARIES} ${NOVA_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${CFITSIO_LIBRARIES} ${M_LIB} ${ZLIB_LIBRARY} ${JPEG_LIBRARY})
add_library(indidriverstatic STATIC ${libindicom_SRCS} ${liblilxml_SRCS} ${indimain_SRCS} ${indidriver_SRCS} ${libwebcam_SRCS})
target_link_libraries(indidriverstatic ${LIBUSB_1_LIBRARIES} ${NOVA_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${CFITSIO_LIBRARIES} ${M_LIB} ${ZLIB_LIBRARY} ${JPEG_LIBRARY})
else()
add_library(indidriver SHARED ${libindicom_SRCS} ${liblilxml_SRCS} ${indimain_SRCS} ${indidriver_SRCS})
target_link_libraries(indidriver ${LIBUSB_1_LIBRARIES} ${NOVA_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${CFITSIO_LIBRARIES} ${M_LIB} ${ZLIB_LIBRARY})
add_library(indidriverstatic STATIC ${libindicom_SRCS} ${liblilxml_SRCS} ${indimain_SRCS} ${indidriver_SRCS})
target_link_libraries(indidriverstatic ${LIBUSB_1_LIBRARIES} ${NOVA_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${CFITSIO_LIBRARIES} ${M_LIB} ${ZLIB_LIBRARY})
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
set_target_properties(indidriver indidriverstatic PROPERTIES VERSION ${CMAKE_INDI_VERSION_STRING} SOVERSION ${INDI_SOVERSION} OUTPUT_NAME indidriver)
install(TARGETS indidriver LIBRARY DESTINATION ${LIB_DESTINATION})
install(TARGETS indidriverstatic ARCHIVE DESTINATION ${LIB_DESTINATION})

##################################################
########### INDI Client Static Library ###########
##################################################
add_library(indiclient STATIC
    basedevice.cpp
    baseclient.cpp
    indiproperty.cpp
)
target_link_libraries(indiclient indi ${CMAKE_THREAD_LIBS_INIT})
install(TARGETS indiclient ARCHIVE DESTINATION ${LIB_DESTINATION})

add_subdirectory(alignment)

