# - Try to find GLee
# Once done this will define
#
# GLEE_FOUND - system has GLee
# GLEE_INCLUDE_DIR - the GLee include directory
# GLEE_LIB - link these to use GLee

find_path(GLEE_INCLUDE_DIR
	NAMES GLee.h
	PATHS /usr/include
		/usr/local/include
		$ENV{INCLUDE}
)

find_library(GLEE_LIB
	NAMES glee
	PATHS /usr/lib
)

if(GLEE_INCLUDE_DIR)
	message(STATUS "Found GLee include dir: ${GLEE_INCLUDE_DIR}")
else(GLEE_INCLUDE_DIR)
	message(STATUS "Could NOT find GLee headers.")
endif(GLEE_INCLUDE_DIR)

if(GLEE_LIB)
	message(STATUS "Found GLee library: ${GLEE_LIB}")
else(GLEE_LIB)
	message(STATUS "Could NOT find GLee library.")
endif(GLEE_LIB)

if(GLEE_INCLUDE_DIR AND GLEE_LIB)
	set(GLEE_FOUND TRUE)
else(GLEE_INCLUDE_DIR AND GLEE_LIB)
	set(GLEE_FOUND FALSE)
	if(GLee_FIND_REQUIRED)
		message(FATAL_ERROR "Could not find GLee. Please install GLee (http://elf-stone.com/glee.php)")
	endif(GLee_FIND_REQUIRED)
endif(GLEE_INCLUDE_DIR AND GLEE_LIB)
