# Hide some variables
mark_as_advanced(HAVE_QGLVIEWER)
mark_as_advanced(QGLVIEWER_CMAKE)

macro (CHECK_EXISTS THISFILE RESULT)
  if (EXISTS ${THISFILE})
    set (${RESULT} YES)
  else ()
    set (${RESULT} NO)
  endif ()
endmacro ()

# QGLViewer Section
set (QGLVIEWER_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libQGLViewer-2.3.9)
if (BUILD_QGLVIEWER)
  # Basic test to see if files are in the right place
  CHECK_EXISTS (${QGLVIEWER_SRC_DIR}/QGLViewer/qglviewer.h HAVE_QGLVIEWER)
  if (NOT HAVE_QGLVIEWER)
    message (FATAL_ERROR "Chose to build QGLViewer from sources, but sources not present!\nCheck 3rdparty/README")
  endif ()
  # Make sure the cmake file wasn't overwritten / idiot proofing
  CHECK_EXISTS (${QGLVIEWER_SRC_DIR}/CMakeLists.txt QGLVIEWER_CMAKE)
  if (NOT QGLVIEWER_CMAKE)
    message (FATAL_ERROR "CMakeLists.txt was deleted from ${QGLVIEWER_SRC_DIR}!")
  endif ()
  add_subdirectory (${QGLVIEWER_SRC_DIR})
endif ()
