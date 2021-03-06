cmake_minimum_required(VERSION 2.4.7)
PROJECT(sbig C CXX)

set(SBIG_VERSION "2.0.2")
set(SBIG_SOVERSION "2")

set(FIRMWARE_INSTALL_DIR "/lib/firmware")
set(UDEVRULES_INSTALL_DIR "/lib/udev/rules.d" CACHE STRING "Base directory for udev rules")
set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib")

IF(UNIX AND NOT WIN32 AND NOT APPLE)
 IF (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "arm.*")
    SET(LIB_ARMHF TRUE)
    MARK_AS_ADVANCED(LIB_ARMHF)
 ELSEIF(CMAKE_SIZEOF_VOID_P MATCHES "8")
    SET(LIB_64 TRUE)
    MARK_AS_ADVANCED(LIB_64)
 ENDIF (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "arm.*")
ENDIF(UNIX AND NOT WIN32 AND NOT APPLE)

IF(NOT DEFINED LIB_64)
   SET(LIB_64 "")
ENDIF(NOT DEFINED LIB_64)
IF (NOT DEFINED LIB_ARMHF)
    SET(LIB_ARMHF "")
ENDIF (NOT DEFINED LIB_ARMHF)

if (LIB_ARMHF)
exec_program(cp ARGS ${CMAKE_CURRENT_SOURCE_DIR}/libsbigudrv_armhf.bin ${CMAKE_BINARY_DIR}/libsbigudrv.so.${SBIG_VERSION})
elseif (LIB_64)
exec_program(cp ARGS ${CMAKE_CURRENT_SOURCE_DIR}/libsbigudrv64.bin ${CMAKE_BINARY_DIR}/libsbigudrv.so.${SBIG_VERSION})
else ()
exec_program(cp ARGS ${CMAKE_CURRENT_SOURCE_DIR}/libsbigudrv32.bin ${CMAKE_BINARY_DIR}/libsbigudrv.so.${SBIG_VERSION})
endif(LIB_ARMHF)

# Make sure symbolic links are installed
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/make_sbig_symlink.cmake "
exec_program(${CMAKE_COMMAND} ARGS -E create_symlink ${LIB_INSTALL_DIR}${LIB_POSTFIX}/libsbigudrv.so.${SBIG_VERSION} ${LIB_INSTALL_DIR}${LIB_POSTFIX}/libsbigudrv.so.${SBIG_SOVERSION})\n
exec_program(${CMAKE_COMMAND} ARGS -E create_symlink ${LIB_INSTALL_DIR}${LIB_POSTFIX}/libsbigudrv.so.${SBIG_SOVERSION} ${LIB_INSTALL_DIR}${LIB_POSTFIX}/libsbigudrv.so)\n
")

install( FILES ${CMAKE_BINARY_DIR}/libsbigudrv.so.${SBIG_VERSION} DESTINATION ${LIB_INSTALL_DIR}${LIB_POSTFIX})
install( FILES sbigucam.hex sbiglcam.hex sbigfcam.hex sbigpcam.hex stfga.bin DESTINATION ${FIRMWARE_INSTALL_DIR})
install( FILES 51-sbig-debian.rules DESTINATION ${UDEVRULES_INSTALL_DIR})
install( SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/make_sbig_symlink.cmake)
