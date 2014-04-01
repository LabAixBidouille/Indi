########## True Technology Wheel ############
add_executable(indi_trutech_wheel
    ${indimain_SRCS}
    trutech_wheel.c
    ${liblilxml_SRCS}
    ${libindicom_SRCS})

target_link_libraries(indi_trutech_wheel ${NOVA_LIBRARIES} ${M_LIB} ${ZLIB_LIBRARY})

install(TARGETS indi_trutech_wheel RUNTIME DESTINATION bin )

########### Filter Simulator ##############
add_executable(indi_simulator_wheel filter_simulator.cpp)

target_link_libraries(indi_simulator_wheel indidriver)

install(TARGETS indi_simulator_wheel RUNTIME DESTINATION bin )

