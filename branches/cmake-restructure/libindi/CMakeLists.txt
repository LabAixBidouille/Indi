cmake_minimum_required(VERSION 2.4.7)
PROJECT(libindi C CXX)

cmake_policy(SET CMP0003 NEW)

##################  INDI version  ################################
set(INDI_SOVERSION "0")
set(CMAKE_INDI_VERSION_MAJOR 0)
set(CMAKE_INDI_VERSION_MINOR 9)
set(CMAKE_INDI_VERSION_RELEASE 8)
set(CMAKE_INDI_VERSION_STRING "${CMAKE_INDI_VERSION_MAJOR}.${CMAKE_INDI_VERSION_MINOR}.${CMAKE_INDI_VERSION_RELEASE}")

##################  Paths  ################################
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")
set(DATA_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/indi/")
set(BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin")
set(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include")

IF(APPLE)
set(CMAKE_SHARED_LINKER_FLAGS "-undefined dynamic_lookup")
ENDIF(APPLE)

##################  setup install directories  ################################
set (LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)" )
## the following are directories where stuff will be installed to
set(UDEVRULES_INSTALL_DIR "/lib/udev/rules.d" CACHE STRING "Base directory for udev rules")
# Non cache properties
set (LIB_DESTINATION "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}")
set(INCLUDE_INSTALL_DIR      "${CMAKE_INSTALL_PREFIX}/include/")
set(PKGCONFIG_INSTALL_PREFIX "${LIB_DESTINATION}/pkgconfig/")

##################  Includes  ################################
Include (CheckCXXSourceCompiles)
include (MacroOptionalFindPackage)
include (MacroLogFeature)
include (MacroBoolTo01)
include (CheckIncludeFiles)

FIND_LIBRARY(M_LIB m)
FIND_PACKAGE(ZLIB REQUIRED)
FIND_PACKAGE(USB-1 REQUIRED)
FIND_PACKAGE(CFITSIO REQUIRED)
FIND_PACKAGE(Nova REQUIRED)
FIND_PACKAGE(Threads REQUIRED)
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  FIND_PACKAGE(JPEG REQUIRED)
endif ()

if (NOT CFITSIO_FOUND OR CFITSIO_VERSION_MAJOR LESS 3)
  message(FATAL_ERROR "CFITSIO version too old, Please install cfitsio 3.x and try again. http://heasarc.gsfc.nasa.gov/fitsio/fitsio.html")
endif (NOT CFITSIO_FOUND OR CFITSIO_VERSION_MAJOR LESS 3)

macro_bool_to_01(CFITSIO_FOUND HAVE_CFITSIO_H)
macro_log_feature(CFITSIO_FOUND "libcfitsio" "A library for reading and writing data files in FITS (Flexible Image Transport System) data format" "http://heasarc.gsfc.nasa.gov/fitsio/fitsio.html" FALSE "3.03" "Provides INDI with FITS I/O support.")

macro_bool_to_01(NOVA_FOUND HAVE_NOVA_H)
macro_log_feature(NOVA_FOUND "libnova" "A general purpose, double precision, Celestial Mechanics, Astrometry and Astrodynamics library" "http://libnova.sourceforge.net" FALSE "0.12.1" "Provides INDI with astrodynamics library.")

check_include_files(linux/videodev2.h HAVE_LINUX_VIDEODEV2_H)
check_include_files(termios.h TERMIOS_FOUND)
macro_bool_to_01(TERMIOS_FOUND HAVE_TERMIOS_H)

configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(.)
include_directories(libs)
include_directories(libs/indibase)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
include_directories( libs/webcam)
  include_directories( ${JPEG_INCLUDE_DIR} )
endif()

if (CFITSIO_FOUND)
  include_directories(${CFITSIO_INCLUDE_DIR})
endif (CFITSIO_FOUND)

include_directories(${NOVA_INCLUDE_DIR})

set(liblilxml_SRCS  ${CMAKE_CURRENT_SOURCE_DIR}/libs/lilxml.c )

set(libindicom_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/libs/indicom.c
	${CMAKE_CURRENT_SOURCE_DIR}/base64.c
	)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
set(libwebcam_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/libs/webcam/PPort.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/libs/webcam/port.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/libs/webcam/v4l2_base.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/libs/webcam/ccvt_c2.c
	${CMAKE_CURRENT_SOURCE_DIR}/libs/webcam/ccvt_misc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/webcam/jpegutils.c
	)
endif()

set (indimain_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/indidriver.c
	${CMAKE_CURRENT_SOURCE_DIR}/indidrivermain.c
	${CMAKE_CURRENT_SOURCE_DIR}/eventloop.c
    )

######################################
########### INDI SERVER ##############
######################################
add_executable(indiserver indiserver.c fq.c libs/lilxml.c)

target_link_libraries(indiserver ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS indiserver RUNTIME DESTINATION bin)

#################################################
############# INDI Shared Library ###############
# To offer lilxml and communination routines    #
# Mostly used by generic clients                #
#################################################
add_library(indi SHARED ${libindicom_SRCS} ${liblilxml_SRCS})
target_link_libraries(indi ${NOVA_LIBRARIES} ${M_LIB} ${ZLIB_LIBRARY} ${CFITSIO_LIBRARIES})

install(TARGETS indi LIBRARY DESTINATION ${LIB_DESTINATION})
set_target_properties(indi PROPERTIES VERSION ${CMAKE_INDI_VERSION_STRING} SOVERSION ${INDI_SOVERSION})

###################################################
############ INDI Main Shared Library #############
# To link with main() for 3rd party legacy drivers#
###################################################
add_library(indimain SHARED ${indimain_SRCS} ${libindicom_SRCS} ${liblilxml_SRCS})

target_link_libraries(indimain ${NOVA_LIBRARIES} ${CFITSIO_LIBRARIES})

install(TARGETS indimain LIBRARY DESTINATION ${LIB_DESTINATION})
set_target_properties(indimain PROPERTIES VERSION ${CMAKE_INDI_VERSION_STRING} SOVERSION ${INDI_SOVERSION})

add_subdirectory(libs)

add_subdirectory(drivers)

add_subdirectory(tools)

add_subdirectory(examples)

#################################################################################
# Miscellaneous extra install info
#################################################################################

install( FILES drivers.xml drivers/focuser/indi_tcfs_sk.xml DESTINATION ${DATA_INSTALL_DIR})

install( FILES indiapi.h indidevapi.h base64.h eventloop.h indidriver.h libs/lilxml.h libs/indibase/indibase.h
libs/indibase/basedevice.h  libs/indibase/defaultdevice.h
libs/indibase/indiccd.h  libs/indibase/indifilterwheel.h
libs/indibase/indifocuserinterface.h  libs/indibase/indifocuser.h
libs/indibase/inditelescope.h libs/indibase/baseclient.h libs/indibase/indiguiderinterface.h
libs/indibase/indifilterinterface.h libs/indibase/indiproperty.h
libs/indicom.h libs/indibase/indiusbdevice.h
libs/indibase/indilogger.h libs/indibase/indicontroller.h
libs/webcam/ccvt.h libs/webcam/ccvt_types.h
 DESTINATION ${INCLUDE_INSTALL_DIR}/libindi COMPONENT Devel)

configure_file(libindi.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/libindi.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libindi.pc DESTINATION ${PKGCONFIG_INSTALL_PREFIX})
