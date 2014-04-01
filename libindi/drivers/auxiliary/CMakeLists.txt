########### GPUSB Driver ###############
add_executable(indi_gpusb gpdriver.cpp gpusb.cpp)

target_link_libraries(indi_gpusb indidriver)
install(TARGETS indi_gpusb RUNTIME DESTINATION bin )
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
install(FILES drivers/auxiliary/99-gpusb.rules DESTINATION ${UDEVRULES_INSTALL_DIR})
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

########### Joystick Driver ###############
add_executable(indi_joystick joystickdriver.cpp joystick.cpp)

target_link_libraries(indi_joystick indidriver)
install(TARGETS indi_joystick RUNTIME DESTINATION bin )


endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

