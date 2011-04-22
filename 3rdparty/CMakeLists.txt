# Hide some variables
mark_as_advanced(HAVE_QGLVIEWER)
mark_as_advanced(QGLVIEWER_CMAKE)
mark_as_advanced(LIB3DS_2_CMAKE)
mark_as_advanced(LIB3DS_1_CMAKE)
mark_as_advanced(HAVE_LIB3DS_2)
mark_as_advanced(HAVE_LIB3DS_1)

# QGLViewer Section
set (QGLVIEWER_SRC_DIR libQGLViewer-2.3.9)
if (BUILD_QGLVIEWER)
  # Basic test to see if files are in the right place
  if (NOT HAVE_QGLVIEWER)
    find_file(HAVE_QGLVIEWER
	      qglviewer.h
	      PATHS ${QGLVIEWER_SRC_DIR}/QGLViewer
	      NO_DEFAULT_PATH
    )
  endif ()
  if (NOT HAVE_QGLVIEWER)
    message (FATAL_ERROR "Chose to build QGLViewer from sources, but sources not present!\nCheck 3rdparty/README")
  endif ()
  # Make sure the cmake file wasn't overwritten / idiot proofing
  if (NOT QGLVIEWER_CMAKE)
    find_file(QGLVIEWER_CMAKE
	      CMakeLists.txt
	      PATHS ${QGLVIEWER_SRC_DIR}
	      NO_DEFAULT_PATH
    )
  endif ()
  if (NOT QGLVIEWER_CMAKE)
    message (FATAL_ERROR "CMakeLists.txt was deleted from ${QGLVIEWER_SRC_DIR}!")
  endif ()
  add_subdirectory (${QGLVIEWER_SRC_DIR})
endif ()


# Lib3ds section

set (LIB3DS_1_SRC_DIR lib3ds-1.3.0)
set (LIB3DS_2_SRC_DIR lib3ds-2.0.0)

if (BUILD_LIB3DS)
  # Make sure the cmake file wasn't overwritten / idiot proofing
  if (NOT LIB3DS_1_CMAKE AND NOT LIB3DS_2_CMAKE)
    find_file(LIB3DS_1_CMAKE
	      CMakeLists.txt
	      PATHS ${LIB3DS_1_SRC_DIR}
	      NO_DEFAULT_PATH
    )
    find_file(LIB3DS_2_CMAKE
	      CMakeLists.txt
	      PATHS ${LIB3DS_2_SRC_DIR}
	      NO_DEFAULT_PATH
    )
  endif ()
  if (NOT LIB3DS_1_CMAKE AND NOT LIB3DS_2_CMAKE)
    message (FATAL_ERROR "CMakeLists.txt were deleted from the 3rdparty/lib3ds* subdirectories!")
  endif ()

  # Basic test to see if files are in the right place / if lib3ds 1 or 2
  if (NOT HAVE_LIB3DS_1 AND NOT HAVE_LIB3DS_2)
    find_file(HAVE_LIB3DS_1
	      mesh.h
	      PATHS ${LIB3DS_1_SRC_DIR}/lib3ds
	      NO_DEFAULT_PATH
  )
  endif ()
  if (HAVE_LIB3DS_1)
    add_subdirectory (${LIB3DS_1_SRC_DIR})
  else ()
    if (NOT HAVE_LIB3DS_2)
      find_file(HAVE_LIB3DS_2
		lib3ds.h
		PATHS ${LIB3DS_2_SRC_DIR}/src
		NO_DEFAULT_PATH
      )
    endif ()
    if (HAVE_LIB3DS_2)
      add_subdirectory (${LIB3DS_2_SRC_DIR})
    else ()
      message (FATAL_ERROR "Chose to build lib3ds from sources, but sources not present!\nCheck 3rdparty/README")
    endif ()
  endif ()
endif ()