find_package (Qt4 4.4.0 COMPONENTS QtCore QtGui QtXml QtOpenGL REQUIRED)
include(${QT_USE_FILE})
find_package(OpenGL REQUIRED)
if (NOT OPENGL_GLU_FOUND)
  message (FATAL_ERROR "GLUT not found!")
endif ()


find_package (Threads)

set (QGLV_HEADERS
  qglviewer.h
  camera.h
  manipulatedFrame.h
  manipulatedCameraFrame.h
  frame.h
  constraint.h
  keyFrameInterpolator.h
  mouseGrabber.h
  quaternion.h
  vec.h
  domUtils.h
  config.h
)
set (QGLV_SOURCES
  qglviewer.cpp
  camera.cpp
  manipulatedFrame.cpp
  manipulatedCameraFrame.cpp
  frame.cpp
  saveSnapshot.cpp
  constraint.cpp
  keyFrameInterpolator.cpp
  mouseGrabber.cpp
  quaternion.cpp
  vec.cpp
)

set (QGLV_FORMS ImageInterface.Qt4.ui)
set (VRENDER_FORMS VRenderInterface.Qt4.ui)

set (VRENDER_SOURCES
  VRender/BackFaceCullingOptimizer.cpp
  VRender/BSPSortMethod.cpp
  VRender/EPSExporter.cpp
  VRender/Exporter.cpp
  VRender/FIGExporter.cpp
  VRender/gpc.cpp
  VRender/ParserGL.cpp
  VRender/Primitive.cpp
  VRender/PrimitivePositioning.cpp
  VRender/TopologicalSortMethod.cpp
  VRender/VisibilityOptimizer.cpp
  VRender/Vector2.cpp
  VRender/Vector3.cpp
  VRender/NVector3.cpp
  VRender/VRender.cpp
)

set (VRENDER_HEADERS
  VRender/AxisAlignedBox.h
  VRender/Exporter.h
  VRender/gpc.h
  VRender/NVector3.h
  VRender/Optimizer.h
  VRender/ParserGL.h
  VRender/Primitive.h
  VRender/PrimitivePositioning.h
  VRender/SortMethod.h
  VRender/Types.h
  VRender/Vector2.h
  VRender/Vector3.h
  VRender/VRender.h
)

foreach (source ${QGLV_SOURCES} ${VRENDER_SOURCES})
  list (APPEND QGLVIEWER_SOURCES QGLViewer/${source})
endforeach ()
foreach (header ${QGLV_HEADERS} ${VRENDER_HEADERS})
  list (APPEND QGLVIEWER_HEADERS QGLViewer/${header})
endforeach ()
foreach (form ${QGLV_FORMS} ${VRENDER_FORMS})
  list (APPEND QGLVIEWER_FORMS QGLViewer/${form})
endforeach ()

if (NOT QGLV_MOC_FILES)
  foreach (moc_file ${QGLVIEWER_HEADERS} ${QGLVIEWER_SOURCES})
    file(STRINGS ${moc_file} QOBJECT_FOUND REGEX Q_OBJECT)
    if (QOBJECT_FOUND)
      list (APPEND QGLV_MOC_FILES ${moc_file})
    endif ()
  endforeach ()
endif ()
qt_wrap_cpp (qglviewer QGLVIEWER_MOC_OUTPUTS ${QGLV_MOC_FILES})
QT4_WRAP_UI (QGLVIEWER_UI_HEADERS ${QGLVIEWER_FORMS})

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/QGLViewer)
include_directories (${CMAKE_CURRENT_BINARY_DIR})

add_library (qglviewer STATIC ${QGLVIEWER_SOURCES} ${QGLVIEWER_MOC_OUTPUTS} ${QGLVIEWER_UI_HEADERS})

target_link_libraries(qglviewer ${CMAKE_THREAD_LIBS_INIT} ${OPENGL_glu_LIBRARY}) # ${QT_LIBRARIES})

set_target_properties(qglviewer
  PROPERTIES
  VERSION 2.3
  SOVERSION 2
  COMPILE_FLAGS -frtti
)

set (QGLVIEWER_LIB qglviewer CACHE INTERNAL "")
set (QGLVIEWER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")
