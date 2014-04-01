################ Focuser Simulator ################
add_executable(indi_simulator_focus focus_simulator.cpp)
target_link_libraries(indi_simulator_focus indidriver)
install(TARGETS indi_simulator_focus RUNTIME DESTINATION bin )

################ Robo Focuser ################
add_executable(indi_robo_focus robofocus.cpp)
target_link_libraries(indi_robo_focus indidriver)
install(TARGETS indi_robo_focus RUNTIME DESTINATION bin )


################ Moonlite Focuser ################
add_executable(indi_moonlite_focus moonlite.cpp)
target_link_libraries(indi_moonlite_focus indidriver)
install(TARGETS indi_moonlite_focus RUNTIME DESTINATION bin )

################ Optec TCF-S ################
add_executable(indi_tcfs_focus tcfs.cpp)

target_link_libraries(indi_tcfs_focus indidriver)
install(TARGETS indi_tcfs_focus RUNTIME DESTINATION bin )

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/make_tcfs_symlink.cmake
"exec_program(\"${CMAKE_COMMAND}\" ARGS -E create_symlink ${BIN_INSTALL_DIR}/indi_tcfs_focus \$ENV{DESTDIR}${BIN_INSTALL_DIR}/indi_tcfs3_focus)\n")
set_target_properties(indi_tcfs_focus PROPERTIES POST_INSTALL_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/make_tcfs_symlink.cmake)

