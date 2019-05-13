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
#	src/formats/Pie_t.cpp
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
#	src/basic/Polygon_t.cpp
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

if(CMAKE_SYSTEM_NAME MATCHES "Windows")

	# Build wmit exe as a Windows app (not a console app)
	set_target_properties(wmit PROPERTIES WIN32_EXECUTABLE TRUE)

endif()

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")

	set_target_properties(wmit PROPERTIES MACOSX_BUNDLE TRUE)
	set_target_properties(wmit PROPERTIES OUTPUT_NAME "WMIT")
	set_target_properties(wmit PROPERTIES MACOSX_BUNDLE_INFO_PLIST "WMIT_macOS_Info.plist.in")
	set_target_properties(wmit PROPERTIES XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "net.wz2100.WMIT")
	set_target_properties(wmit PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "net.wz2100.WMIT")
	set_target_properties(wmit PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "WMIT")
	set_target_properties(wmit PROPERTIES MACOSX_BUNDLE_COPYRIGHT "Copyright © 2010-2019 Warzone 2100 Project")

	# Workaround: Empty "Archive" build of target
	# See: https://cmake.org/pipermail/cmake/2012-December/053017.html; https://gitlab.kitware.com/cmake/cmake/issues/15183
	set_target_properties(wmit PROPERTIES XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)")
	set_target_properties(wmit PROPERTIES XCODE_ATTRIBUTE_SKIP_INSTALL "No")

	# Embed the dSYM file in the app bundle
	set_target_properties(wmit PROPERTIES
		XCODE_ATTRIBUTE_DWARF_DSYM_FOLDER_PATH "\$(CONFIGURATION_BUILD_DIR)/\$(EXECUTABLE_FOLDER_PATH)"
		XCODE_ATTRIBUTE_DWARF_DSYM_FILE_NAME "\$(EXECUTABLE_NAME).dSYM"
	)

	# Copy QGLVIEWER_LIB to the Frameworks folder
	set_source_files_properties(
		"${QGLVIEWER_LIB}" PROPERTIES
		MACOSX_PACKAGE_LOCATION "Frameworks"
		XCODE_LAST_KNOWN_FILE_TYPE "wrapper.framework"
		XCODE_FILE_ATTRIBUTES "CodeSignOnCopy;"
	)
	if(NOT CMAKE_GENERATOR STREQUAL "Xcode")
		# Other generators will need a custom-command equivalent of setting the "Code Sign on Copy" Xcode attribute to ON
		message( WARNING "The generator \"${CMAKE_GENERATOR}\" does not currently handle code-signing: \"${QGLVIEWER_LIB}\"" )
	endif()
	target_sources(wmit PRIVATE "${QGLVIEWER_LIB}")

	# Set install RPATH for app bundle
	set_target_properties(wmit PROPERTIES INSTALL_RPATH "@executable_path/../Frameworks")

endif()

install(TARGETS wmit COMPONENT Core DESTINATION ".")


get_target_property(_wmit_output_name wmit OUTPUT_NAME)
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	if(NOT CMAKE_CROSSCOMPILING)
		# Run windeployqt to pick up all Qt dependencies
		find_program(TOOL_WINDEPLOYQT NAMES windeployqt)
		if(TOOL_WINDEPLOYQT)
			install(CODE "
				execute_process(
					COMMAND ${TOOL_WINDEPLOYQT} \"\${CMAKE_INSTALL_PREFIX}/${_wmit_output_name}.exe\" --no-compiler-runtime --no-angle --no-svg
				)
			" COMPONENT Core)
		else()
			message( WARNING "Unable to find windeployqt; installation may not include all required Qt libraries" )
		endif()
	else()
		message( STATUS "CMAKE_CROSSCOMPILING is defined - skipping windeployqt" )
	endif()
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
	# Run macdeployqt to pick up all Qt dependencies
	find_program(TOOL_MACDEPLOYQT NAMES macdeployqt)
	if(TOOL_MACDEPLOYQT)
		install(CODE "
			execute_process(
				COMMAND ${TOOL_MACDEPLOYQT} \"\${CMAKE_INSTALL_PREFIX}/${_wmit_output_name}.app\"
			)
		" COMPONENT Core)
	else()
		message( WARNING "Unable to find macdeployqt; installation may not include all required Qt libraries" )
	endif()
endif()

add_subdirectory(pkg)

