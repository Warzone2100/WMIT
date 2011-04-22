cmake_minimum_required (VERSION 2.8)

project (wmit)

set (CMAKE_COLOR_MAKEFILE TRUE)

set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_EXECUTABLE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Use relative paths
# This is mostly to reduce path size for command-line limits on windows
if (WIN32)
  # This seems to break Xcode projects so definitely don't enable on Apple builds
  set (CMAKE_USE_RELATIVE_PATHS true)
  set (CMAKE_SUPPRESS_REGENERATION true)
endif ()

# options
option (BUILD_QGLVIEWER "Should QGLViewer be built from the sources." FALSE)
option (BUILD_LIB3DS "Should lib3ds be built from the sources." FALSE)
option (USE_SYSTEM_GLEE "Use GLee present on the system." FALSE)
option (VERBOSE_COMPILER_WARNINGS "Potentially make the compiler generate many warnings." FALSE)

if (CMAKE_BUILD_TYPE STREQUAL "")
  # CMake defaults to leaving CMAKE_BUILD_TYPE empty. This screws up
  # differentiation between debug and release builds.
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: None (CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif ()

# Look for dependencies
find_package (Qt4 4.4.0 COMPONENTS QtCore QtGui QtXml QtOpenGL REQUIRED)
include(${QT_USE_FILE})

if (BUILD_QGLVIEWER OR BUILD_LIB3DS)
  add_subdirectory(3rdparty)
endif ()

if (NOT BUILD_QGLVIEWER)
  find_library (QGLVIEWER_LIB QGLViewer NAMES qglviewer)
  if (NOT QGLVIEWER_LIB)
    message (SEND_ERROR "Could not find QGLViewer library.")
  endif ()
  find_path (QGLVIEWER_INCLUDE_DIR QGLViewer/qglviewer.h)
  if (NOT QGLVIEWER_INCLUDE_DIR)
    message (SEND_ERROR "Could not find QGLViewer header.")
  endif ()
endif ()

if (NOT BUILD_LIB3DS)
  find_library (LIB3DS_LIB lib3ds)
  if (NOT LIB3DS_LIB)
    message (SEND_ERROR "Could not find lib3ds library.")
  endif ()
  find_path (LIB3DS_INCLUDE_DIR lib3ds/file.h)
  if (NOT LIB3DS_INCLUDE_DIR)
    find_path (LIB3DS_INCLUDE_DIR lib3ds.h)
    if (NOT LIB3DS_INCLUDE_DIR)
      message (SEND_ERROR "Could not find lib3ds header.")
    endif ()
    set (LIB3DS_VERSION 2)
    add_definitions (-DLIB3DS_VERSION_2)
  elseif ()
    set (LIB3DS_VERSION 1)
    add_definitions (-DLIB3DS_VERSION_1)
  endif ()
endif ()

if (CMAKE_COMPILER_IS_GNUCXX)

  exec_program (gcc ARGS --version OUTPUT_VARIABLE CMAKE_GNUCC_COMPILER_VERSION)
  string (REGEX MATCH "[^\\.]4\\.[2-9]+" CMAKE_GNUCC_COMPILER_GEQUAL42 ${CMAKE_GNUCC_COMPILER_VERSION})
  string (STRIP MAKE_GNUCC_COMPILER_GEQUAL42 ${CMAKE_GNUCC_COMPILER_GEQUAL42})
  # Set our own default flags at first run.
  if (NOT CONFIGURED)

    set(WMIT_C_FLAGS "-Wall $ENV{CFLAGS} -pipe ${CMAKE_C_FLAGS}"
      CACHE STRING "C flags for current build type." FORCE)
    set(WARN_FLAGS "-Wall -pedantic -Wextra -Wnon-virtual-dtor -Wno-long-long -Wold-style-cast -Wredundant-decls -Wshadow")
    set(WMIT_CXX_FLAGS "-std=c++0x -fno-nonansi-builtins -ffor-scope -fno-exceptions -fno-signed-zeros -fno-math-errno $ENV{CXXFLAGS} -pipe ${CMAKE_CXX_FLAGS}")

    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
      set(WMIT_C_FLAGS "${CMAKE_C_FLAGS_DEBUG} -DDDEBUG -fstack-protector-all ${WMIT_C_FLAGS}"
        CACHE STRING "C flags for current build type." FORCE)
      set(WMIT_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -fstack-protector-all ${WARN_FLAGS} ${WMIT_CXX_FLAGS}"
        CACHE STRING "CXX flags for current build type." FORCE)
    endif ()
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
      set(WMIT_C_FLAGS "${CMAKE_C_FLAGS_RELEASE} ${WMIT_C_FLAGS}"
        CACHE STRING "C flags for current build type." FORCE)
      set(WMIT_CXX_FLAGS "${CMAKE_CXX_FLAGS_RELEASE} ${WMIT_CXX_FLAGS}"
        CACHE STRING "CXX flags for current build type." FORCE)
    endif ()
    if (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
      set(WMIT_C_FLAGS "${CMAKE_C_FLAGS_RELWITHDEBINFO} -DNDEBUG ${WMIT_C_FLAGS}"
        CACHE STRING "C flags for current build type." FORCE)
      set(WMIT_CXX_FLAGS "${${CMAKE_CXX_FLAGS_RELWITHDEBINFO}} -DNDEBUG ${WMIT_CXX_FLAGS}"
        CACHE STRING "CXX flags for current build type." FORCE)
    endif ()
    if (CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
      set(WMIT_C_FLAGS "${CMAKE_C_FLAGS_MINSIZEREL} ${WMIT_C_FLAGS}"
        CACHE STRING "C flags for current build type." FORCE)
      set(WMIT_CXX_FLAGS "${CMAKE_CXX_FLAGS_MINSIZEREL} ${WMIT_CXX_FLAGS}"
        CACHE STRING "CXX flags for current build type." FORCE)
    endif ()

    if (CMAKE_GNUCC_COMPILER_GEQUAL42 AND CMAKE_BUILD_TYPE STREQUAL "Release")
      message (STATUS "Release build chosen, use RelWithDebInfo if you plan to distribute this binary.\nThis build will be optimized for the current machine.")
      set(CMAKE_C_FLAGS_RELEASE "-march=native -O2 -DNDEBUG"
        CACHE STRING "Flags used by the C compiler during release builds." FORCE)
      set(CMAKE_CXX_FLAGS_RELEASE "-march=native -O2 -DNDEBUG"
        CACHE STRING "Flags used by the CXX compiler during release builds." FORCE)
    endif ()
  endif (NOT CONFIGURED)

  if (CMAKE_GNUCC_COMPILER_GEQUAL42 AND CMAKE_BUILD_TYPE STREQUAL "Release")
    message (STATUS "Release build chosen, use RelWithDebInfo if you plan to distribute this binary.\nThis build will be optimized for the current machine.")
  endif ()
endif ()

if (USE_SYSTEM_GLEE)
  find_library (GLEE_LIB GLee NAMES glee GLEE)
  if (NOT GLEE_LIB)
    message (SEND_ERROR "Could not find GLee library.")
  endif ()

  find_path (GLEE_INCLUDE_DIR GL/glee.h NAMES OpenGL/glee.h)
  if (NOT GLee_INCLUDE_DIR)
    message (SEND_ERROR "Could not find GLee header.")
  endif ()  
endif ()

include_directories (${LIB3DS_INCLUDE_DIR} ${QGLVIEWER_INCLUDE_DIR})

set (HEADERS
  MainWindow.hpp
  ImportDialog.hpp
  UVEditor.hpp
  ConfigDialog.hpp
  TransformDock.hpp
  Vector.hpp
  VectorTypes.hpp
  Polygon.hpp
  Pie.hpp
  WZM.hpp
  TeamColoursDock.hpp
  Generic.hpp
  IAnimatable.hpp
  QtGLView.hpp
  TexturedRenderable.hpp
  IGLRenderable.hpp
  IGLTextureManager.hpp
  GLTexture.hpp
  Mesh.hpp
  OBJ.hpp
  Util.hpp
  QWZM.hpp
  ExportDialog.hpp
  ITCMaskProvider.hpp
  TCMaskRenderable.hpp
)

set (SOURCES 
  main.cpp
  MainWindow.cpp
  ImportDialog.cpp
  UVEditor.cpp
  ConfigDialog.cpp
  TransformDock.cpp
  Polygon_t.cpp
  WZM.cpp
  TeamColoursDock.cpp
  Generic.cpp
  Pie_t.cpp
  Pie.cpp
  QtGLView.cpp
  Mesh.cpp
  GLTexture.cpp
  Util.cpp
  QWZM.cpp
  ExportDialog.cpp
)

set (FORMS
  MainWindow.ui
  ImportDialog.ui
  UVEditor.ui
  ConfigDialog.ui
  TransformDock.ui
  TeamColoursDock.ui
  ExportDialog.ui
)

if (NOT MOC_FILES)
  foreach (moc_file ${HEADERS} ${SOURCES})
    file(STRINGS ${moc_file} QOBJECT_FOUND REGEX Q_OBJECT)
    if (QOBJECT_FOUND)
      list (APPEND MOC_FILES ${moc_file})
    endif ()
  endforeach ()
endif ()
qt_wrap_cpp(wmit MOC_OUTPUTS ${MOC_FILES})

QT4_WRAP_UI (UI_HEADERS ${FORMS})

include_directories (${CMAKE_BINARY_DIR}) # for generated headers
include_directories (${CMAKE_SOURCE_DIR})

if (NOT USE_SYSTEM_GLEE)
  set (HEADERS ${HEADERS} GLee.h)
  set (SOURCES ${SOURCES} GLee.c)
  set_source_files_properties(GLee.c PROPERTIES COMPILE_FLAGS ${WMIT_C_FLAGS})
endif()

add_executable (wmit ${SOURCES} ${MOC_OUTPUTS} ${UI_HEADERS})

set_target_properties (wmit
  PROPERTIES COMPILE_FLAGS ${WMIT_CXX_FLAGS}
)

target_link_libraries (wmit m ${LIB3DS_LIB} ${QGLVIEWER_LIB} ${QT_LIBRARIES})

if (USE_SYSTEM_GLEE)
  target_link_libraries (wmit ${GLEE_LIB})
endif ()

if (BUILD_QGLVIEWER)
  add_dependencies (wmit qglviewer)
endif ()

if (BUILD_LIB3DS)
  add_dependencies (wmit lib3ds)
endif ()

## Other stuff
set (CONFIGURED YES CACHE INTERNAL "")

# vim: ts=2:sw=2
