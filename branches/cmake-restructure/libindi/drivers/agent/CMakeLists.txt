#foreach( subdir
#
#    add_subdirectory(${subdir})
#endforeach()

# This should realy be in a subdirectory
########### Imager ##############
add_executable(indi_imager_agent agent_imager.cpp)
target_link_libraries(indi_imager_agent indidriver indiclient)
install(TARGETS indi_imager_agent RUNTIME DESTINATION bin )

