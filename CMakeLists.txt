project(wmit)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
add_definitions(-Wall)
find_package(QGLViewer)
find_package(Lib3ds)
find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

find_path(GLEE_INCLUDE_DIR NAMES GLee.h PATHS /usr/include)
find_library(GLEE_LIBRARY NAMES glee PATHS /usr/lib)

add_definitions(-DLIB3DS_VERSION_1)

include_directories(${GLEE_INCLUDE_DIR})

include_directories(
	${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
	${LIB3DS_INCLUDE_DIR} ${QGLVIEWER_INCLUDE_DIR}
	${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR} ${QT_QTOPENGL_INCLUDE_DIR} ${QT_QTXML_INCLUDE_DIR}
	src
	src/basic
	src/formats
	src/ui
	src/widgets
	3rdparty/GLee
	$$QGLVIEWER_INCL)
set( wmit_HDRS
	src/formats/WZM.hpp
	src/formats/Pie.hpp
	src/formats/OBJ.hpp
	src/formats/Mesh.hpp
	src/Util.hpp
	src/Generic.hpp
	src/basic/VectorTypes.hpp
	src/basic/Vector.hpp
	src/basic/Polygon.hpp
	src/basic/IGLTextureManager.hpp
	src/basic/IGLRenderable.hpp
	src/basic/IAnimatable.hpp
	src/basic/GLTexture.hpp
	3rdparty/GLee/GLee.h
	src/wmit.h
	src/basic/IGLTexturedRenderable.hpp
	src/basic/IGLShaderManager.h
	src/basic/IGLShaderRenderable.h
	)

set( wmit_SRCS
	src/formats/WZM.cpp
	src/formats/Pie_t.cpp
	src/formats/Pie.cpp
	src/formats/Mesh.cpp
	src/ui/UVEditor.cpp
	src/ui/TransformDock.cpp
	src/ui/TeamColoursDock.cpp
	src/ui/MainWindow.cpp
	src/ui/ImportDialog.cpp
	src/ui/ExportDialog.cpp
	src/Util.cpp
	src/main.cpp
	src/Generic.cpp
	src/basic/Polygon_t.cpp
	src/basic/GLTexture.cpp
	3rdparty/GLee/GLee.c
	src/widgets/QWZM.cpp
	src/widgets/QtGLView.cpp
	src/ui/TextureDialog.cpp
	src/ui/TexConfigDialog.cpp
	)

set( wmit_UIS
	src/ui/UVEditor.ui
	src/ui/TransformDock.ui
	src/ui/TeamColoursDock.ui
	src/ui/MainWindow.ui
	src/ui/ImportDialog.ui
	src/ui/ExportDialog.ui
	src/ui/TextureDialog.ui
	src/ui/TexConfigDialog.ui
	)
QT4_WRAP_UI(UIS ${wmit_UIS})

set( wmit_RSCS
	resources.qrc
	)
QT4_ADD_RESOURCES(RSCS ${wmit_RSCS})

set( wmit_MOCS
	src/ui/UVEditor.hpp
	src/ui/TransformDock.hpp
	src/ui/TeamColoursDock.hpp
	src/ui/MainWindow.hpp
	src/ui/ImportDialog.hpp
	src/ui/ExportDialog.hpp
	src/widgets/QWZM.hpp
	src/widgets/QtGLView.hpp
	src/ui/TextureDialog.h
	src/ui/TexConfigDialog.hpp
	)
QT4_WRAP_CPP(MOCS ${wmit_MOCS})

add_executable(wmit ${wmit_SRCS} ${UIS} ${RSCS} ${TRS} ${MOCS})
target_link_libraries(wmit ${GLEE_LIB} ${LIB3DS_LIB} ${QGLVIEWER_LIB} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTOPENGL_LIBRARY} ${QT_QTXML_LIBRARY})
