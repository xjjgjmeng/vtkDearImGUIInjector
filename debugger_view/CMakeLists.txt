file(GLOB modules "*")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
foreach(m ${modules})
	if(IS_DIRECTORY ${m})
		add_subdirectory(${m})
	endif()
endforeach()