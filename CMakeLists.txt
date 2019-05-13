project(wmit)

cmake_minimum_required(VERSION 3.5)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

add_definitions(-Wall)

find_package(OpenGL REQUIRED)
find_package(QGLViewer REQUIRED)
find_package(Qt5 COMPONENTS Core Gui Widgets OpenGL Xml REQUIRED)

include_directories(
	${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}
	${QGLVIEWER_INCLUDE_DIR}
	src
	src/basic
	src/formats
	src/ui
	src/widgets
)

set( wmit_SRCS
	src/formats/WZM.cpp
	src/formats/Pie_t.cpp
	src/formats/Pie.cpp
	src/formats/Mesh.cpp
	src/ui/UVEditor.cpp
	src/ui/TransformDock.cpp
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
	src/ui/MaterialDock.cpp
	src/ui/meshdock.cpp
)

set( wmit_UIS
	src/ui/UVEditor.ui
	src/ui/TransformDock.ui
	src/ui/MainWindow.ui
	src/ui/ImportDialog.ui
	src/ui/ExportDialog.ui
	src/ui/TextureDialog.ui
	src/ui/TexConfigDialog.ui
	src/ui/MaterialDock.ui
	src/ui/meshdock.ui
)

set( wmit_RSCS
	resources.qrc
)

set( wmit_MOCS
	src/ui/UVEditor.h
	src/ui/MaterialDock.h
	src/ui/TransformDock.h
	src/ui/MainWindow.h
	src/ui/ImportDialog.h
	src/ui/ExportDialog.h
	src/widgets/QWZM.h
	src/widgets/QtGLView.h
	src/ui/TextureDialog.h
	src/ui/TexConfigDialog.h
	src/ui/meshdock.h
)

QT5_WRAP_UI(UIS ${wmit_UIS})
QT5_ADD_RESOURCES(RSCS ${wmit_RSCS})
QT5_WRAP_CPP(MOCS ${wmit_MOCS})

add_executable(wmit ${wmit_SRCS} ${UIS} ${RSCS} ${TRS} ${MOCS})
target_link_libraries(wmit OpenGL::GL OpenGL::GLU ${QGLVIEWER_LIB})
target_link_libraries(wmit Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL Qt5::Xml)
