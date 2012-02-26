project(wmit)

cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

add_definitions(-Wall)

find_package(QGLViewer)
find_package(Qt4 REQUIRED)

include(${QT_USE_FILE})

include_directories(
	${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}
	${QGLVIEWER_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR} ${QT_QTOPENGL_INCLUDE_DIR} ${QT_QTXML_INCLUDE_DIR}
	src
	src/basic
	src/formats
	src/ui
	src/widgets
	3rdparty/GLee
)

set( wmit_SRCS
	src/formats/WZM.cpp
	src/formats/Pie_t.cpp
	src/formats/Pie.cpp
	src/formats/Mesh.cpp
	src/ui/UVEditor.cpp
	src/ui/MaterialDock.cpp
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
	src/widgets/QWZM.cpp
	src/widgets/QtGLView.cpp
	src/ui/TextureDialog.cpp
	src/ui/TexConfigDialog.cpp
	3rdparty/GLee/GLee.c
)

set( wmit_UIS
	src/ui/UVEditor.ui
	src/ui/MaterialDock.ui
	src/ui/TransformDock.ui
	src/ui/TeamColoursDock.ui
	src/ui/MainWindow.ui
	src/ui/ImportDialog.ui
	src/ui/ExportDialog.ui
	src/ui/TextureDialog.ui
	src/ui/TexConfigDialog.ui
)

set( wmit_RSCS
	resources.qrc
)

set( wmit_MOCS
	src/ui/UVEditor.h
	src/ui/MaterialDock.h
	src/ui/TransformDock.h
	src/ui/TeamColoursDock.h
	src/ui/MainWindow.h
	src/ui/ImportDialog.h
	src/ui/ExportDialog.h
	src/widgets/QWZM.h
	src/widgets/QtGLView.h
	src/ui/TextureDialog.h
	src/ui/TexConfigDialog.h
)

QT4_WRAP_UI(UIS ${wmit_UIS})
QT4_ADD_RESOURCES(RSCS ${wmit_RSCS})
QT4_WRAP_CPP(MOCS ${wmit_MOCS})

add_executable(wmit ${wmit_SRCS} ${UIS} ${RSCS} ${TRS} ${MOCS})
target_link_libraries(wmit ${QGLVIEWER_LIB} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTOPENGL_LIBRARY} ${QT_QTXML_LIBRARY})
