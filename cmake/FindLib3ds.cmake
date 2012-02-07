# find lib3ds includes and library
#
# LIB3DS_INCLUDE_DIR - where the lib3ds directory containing the headers can be
#                      found
# LIB3DS_LIB     - full path to the lib3ds library
# LIB3DS_FOUND       - TRUE if lib3ds was found

FIND_PATH(LIB3DS_INCLUDE_DIR lib3ds/file.h
	/usr/include
	/usr/local/include
    $ENV{INCLUDE}
)
FIND_LIBRARY(LIB3DS_LIB NAMES 3ds lib3ds)

IF(LIB3DS_INCLUDE_DIR)
	MESSAGE(STATUS "Found lib3ds include dir: ${LIB3DS_INCLUDE_DIR}")
ELSE(LIB3DS_INCLUDE_DIR)
	MESSAGE(STATUS "Could NOT find lib3ds headers.")
ENDIF(LIB3DS_INCLUDE_DIR)

IF(LIB3DS_LIB)
	MESSAGE(STATUS "Found lib3ds library: ${LIB3DS_LIB}")
ELSE(LIB3DS_LIB)
	MESSAGE(STATUS "Could NOT find lib3ds library.")
ENDIF(LIB3DS_LIB)


IF(LIB3DS_INCLUDE_DIR AND LIB3DS_LIB)
	SET(LIB3DS_FOUND TRUE)
ELSE(LIB3DS_INCLUDE_DIR AND LIB3DS_LIB)
	SET(LIB3DS_FOUND FALSE)
	IF(Lib3ds_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Could not find lib3ds. Please install lib3ds (http://lib3ds.sourceforge.net)")
	ENDIF(Lib3ds_FIND_REQUIRED)
ENDIF(LIB3DS_INCLUDE_DIR AND LIB3DS_LIB)

