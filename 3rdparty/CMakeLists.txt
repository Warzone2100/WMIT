# Hide some variables
mark_as_advanced(HAVE_QGLVIEWER)
mark_as_advanced(QGLVIEWER_CMAKE)
mark_as_advanced(LIB3DS_2_CMAKE)
mark_as_advanced(LIB3DS_1_CMAKE)
mark_as_advanced(HAVE_LIB3DS_2)
mark_as_advanced(HAVE_LIB3DS_1)

MACRO (CHECK_EXISTS THISFILE RESULT)
  if (EXISTS ${THISFILE})
    set (${RESULT} YES)
  else ()
    set (${RESULT} NO)
  endif ()
ENDMACRO ()

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


# Lib3ds section

set (LIB3DS_1_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib3ds-1.3.0)
set (LIB3DS_2_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib3ds-2.0.0)

if (BUILD_LIB3DS)
  # Make sure the cmake file wasn't overwritten / idiot proofing
  CHECK_EXISTS (${LIB3DS_1_SRC_DIR}/CMakeLists.txt LIB3DS_1_CMAKE)
  CHECK_EXISTS (${LIB3DS_2_SRC_DIR}/CMakeLists.txt LIB3DS_2_CMAKE)
  if (NOT LIB3DS_1_CMAKE AND NOT LIB3DS_2_CMAKE)
    message (FATAL_ERROR "CMakeLists.txt were deleted from the 3rdparty/lib3ds* subdirectories!")
  endif ()

  # Basic test to see if files are in the right place / if lib3ds 1 or 2
  CHECK_EXISTS (${LIB3DS_2_SRC_DIR}/src/lib3ds.h HAVE_LIB3DS_2)
  if (HAVE_LIB3DS_2)
    add_definitions (-DLIB3DS_VERSION_2)
    add_subdirectory (${LIB3DS_2_SRC_DIR})
  else ()
    CHECK_EXISTS (${LIB3DS_1_SRC_DIR}/lib3ds/mesh.h HAVE_LIB3DS_1)
    if (HAVE_LIB3DS_1)
      add_definitions (-DLIB3DS_VERSION_1)
      add_subdirectory (${LIB3DS_1_SRC_DIR})
    else ()
      message (FATAL_ERROR "Chose to build lib3ds from sources, but sources not present!\nCheck 3rdparty/README")
    endif ()
  endif ()
endif ()