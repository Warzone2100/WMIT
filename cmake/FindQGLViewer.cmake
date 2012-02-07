# - Try to find QGLViewer
# Once done this will define
#
#  QGLVIEWER_FOUND - system has QGLViewer
#  QGLVIEWER_INCLUDE_DIR - the QGLViewer include directory
#  QGLVIEWER_LIB - Link these to use QGLViewer
#  QGLVIEWER_DEFINITIONS - Compiler switches required for using QGLViewer
#

find_path(QGLVIEWER_INCLUDE_DIR 
          NAMES QGLViewer/qglviewer.h
          PATHS /usr/include
                /usr/local/include
                ENV QGLVIEWERROOT 
         )

find_library(QGLVIEWER_LIBRARY_RELEASE 
             NAMES qglviewer-qt4 qglviewer QGLViewer QGLViewer2
             PATHS /usr/lib
                   /usr/local/lib
                   ENV QGLVIEWERROOT
                   ENV LD_LIBRARY_PATH
                   ENV LIBRARY_PATH
             PATH_SUFFIXES QGLViewer QGLViewer/release
            )

find_library(QGLVIEWER_LIBRARY_DEBUG
             NAMES dqglviewer dQGLViewer dQGLViewer2
             PATHS /usr/lib
                   /usr/local/lib
                   ENV QGLVIEWERROOT
                   ENV LD_LIBRARY_PATH
                   ENV LIBRARY_PATH
             PATH_SUFFIXES QGLViewer QGLViewer/debug      
            )

if(QGLVIEWER_LIBRARY_RELEASE)
  if(QGLVIEWER_LIBRARY_DEBUG)
    set(QGLVIEWER_LIB_ optimized ${QGLVIEWER_LIBRARY_RELEASE} debug ${QGLVIEWER_LIBRARY_DEBUG})
  else()
    set(QGLVIEWER_LIB_ ${QGLVIEWER_LIBRARY_RELEASE})
  endif()

  set(QGLVIEWER_LIB ${QGLVIEWER_LIB_} CACHE FILEPATH "The QGLViewer library")

endif()

IF(QGLVIEWER_INCLUDE_DIR AND QGLVIEWER_LIB)
   SET(QGLVIEWER_FOUND TRUE)
   SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQGLVIEWER_FOUND")
ENDIF(QGLVIEWER_INCLUDE_DIR AND QGLVIEWER_LIB)

IF(QGLVIEWER_FOUND)
  IF(NOT QGLViewer_FIND_QUIETLY)
    MESSAGE(STATUS "Found QGLViewer: ${QGLVIEWER_LIB}")
  ENDIF(NOT QGLViewer_FIND_QUIETLY)
ELSE(QGLVIEWER_FOUND)
  IF(QGLViewer_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could not find QGLViewer")
  ENDIF(QGLViewer_FIND_REQUIRED)
ENDIF(QGLVIEWER_FOUND)

