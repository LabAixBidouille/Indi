foreach( subdir
	agent
	telescope
	auxiliary
	ccd
	filter_wheel
	focuser
	video )

    add_subdirectory(${subdir})

endforeach()


