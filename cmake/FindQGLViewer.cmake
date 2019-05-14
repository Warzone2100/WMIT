# - Try to find QGLViewer
# Once done this will define
#
#  QGLVIEWER_FOUND - system has QGLViewer
#  QGLVIEWER_INCLUDE_DIR - the QGLViewer include directory
#  QGLVIEWER_LIB - link these to use QGLViewer

find_path(QGLVIEWER_INCLUDE_DIR
	NAMES QGLViewer/qglviewer.h
	PATHS /usr/include
		/usr/local/include
		$ENV{QGLVIEWERROOT}
		$ENV{QGLVIEWERROOT}/include
		$ENV{INCLUDE}
)

find_library(QGLVIEWER_LIB
	NAMES qglviewer-qt5 QGLViewer qglviewer QGLViewer-qt5 QGLViewer2
	PATHS /usr/lib
		/usr/local/lib
		$ENV{QGLVIEWERROOT}
		$ENV{QGLVIEWERROOT}/lib
		$ENV{LD_LIBRARY_PATH}
		$ENV{LIBRARY_PATH}
	PATH_SUFFIXES QGLViewer
)

if(QGLVIEWER_INCLUDE_DIR)
	message(STATUS "Found QGLViewer include dir: ${QGLVIEWER_INCLUDE_DIR}")
else(QGLVIEWER_INCLUDE_DIR)
	message(STATUS "Could NOT find QGLViewer headers.")
endif(QGLVIEWER_INCLUDE_DIR)

if(QGLVIEWER_LIB)
	message(STATUS "Found QGLViewer library: ${QGLVIEWER_LIB}")
else(QGLVIEWER_LIB)
	message(STATUS "Could NOT find QGLViewer library.")
endif(QGLVIEWER_LIB)

if(QGLVIEWER_INCLUDE_DIR AND QGLVIEWER_LIB)
	set(QGLVIEWER_FOUND TRUE)
else(QGLVIEWER_INCLUDE_DIR AND QGLVIEWER_LIB)
	set(QGLVIEWER_FOUND FALSE)
	if(QGLViewer_FIND_REQUIRED)
		message(FATAL_ERROR "Could not find QGLViewer. Please install QGLViewer (http://www.libqglviewer.com)")
	endif(QGLViewer_FIND_REQUIRED)
endif(QGLVIEWER_INCLUDE_DIR AND QGLVIEWER_LIB)
