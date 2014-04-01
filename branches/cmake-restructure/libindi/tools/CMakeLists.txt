########### getINDI ##############
add_executable(indi_getprop ../eventloop.c getINDIproperty.c  ${liblilxml_SRCS} ${libindicom_SRCS})

target_link_libraries(indi_getprop ${NOVA_LIBRARIES} ${M_LIB} ${ZLIB_LIBRARY})

install(TARGETS indi_getprop RUNTIME DESTINATION bin )

#################################################################################

########### setINDI ##############
add_executable(indi_setprop ../eventloop.c setINDIproperty.c ${liblilxml_SRCS} ${libindicom_SRCS})

target_link_libraries(indi_setprop ${NOVA_LIBRARIES} ${M_LIB} ${ZLIB_LIBRARY})

install(TARGETS indi_setprop RUNTIME DESTINATION bin )

#################################################################################

########### evelINDI ##############
add_executable(indi_eval ../eventloop.c compiler.c evalINDI.c ${liblilxml_SRCS} ${libindicom_SRCS})

target_link_libraries(indi_eval ${NOVA_LIBRARIES} ${M_LIB} ${ZLIB_LIBRARY})

install(TARGETS indi_eval RUNTIME DESTINATION bin )
