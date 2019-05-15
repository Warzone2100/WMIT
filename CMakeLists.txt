cmake_minimum_required(VERSION 3.5)
project(wmit)

OPTION(PACKAGE_SOURCE_ONLY "Disables some requirements - use ONLY for configuring to package source" OFF)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

##################################################

# CXX Standard
if(MSVC)
	set(CMAKE_CXX_STANDARD 14)
else()
	set(CMAKE_CXX_STANDARD 11)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Disable compiler-specific extensions

# Support folders (for nicer IDE organization)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "_CMakePredefinedTargets")

# Use "-fPIC" / "-fPIE" for all targets by default
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

##################################################
# Compiler-specific options

# Treat source files as UTF-8 (unless they have a BOM) (MSVC)
# NOTE: For MSVC, this also sets the executable character set.
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

if(MSVC)
	# Enable /W3
	# NOTE: Do not use /Wall, as it enables *many* warnings that are off by default. MS recommends /W4 (max)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
else() # NOT MSVC
	add_definitions(-Wall)
endif()

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)

# Enable stack protection, if supported by the compiler
# Prefer -fstack-protector-strong if supported, fall-back to -fstack-protector
check_c_compiler_flag(-fstack-protector-strong HAS_CFLAG_FSTACK_PROTECTOR_STRONG)
if (HAS_CFLAG_FSTACK_PROTECTOR_STRONG)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-strong")
else()
	check_c_compiler_flag(-fstack-protector HAS_CFLAG_FSTACK_PROTECTOR)
	if (HAS_CFLAG_FSTACK_PROTECTOR)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector")
	endif()
endif()
check_cxx_compiler_flag(-fstack-protector-strong HAS_CXXFLAG_FSTACK_PROTECTOR_STRONG)
if (HAS_CXXFLAG_FSTACK_PROTECTOR_STRONG)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong")
else()
	check_cxx_compiler_flag(-fstack-protector HAS_CXXFLAG_FSTACK_PROTECTOR)
	if (HAS_CXXFLAG_FSTACK_PROTECTOR)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector")
	endif()
endif()

##################################################
# Find dependencies

set(_required_dependency_flag "REQUIRED")
if(PACKAGE_SOURCE_ONLY)
	message( WARNING "Ignoring dependency failures - this should only be used for configuring to package source" )
	set(_required_dependency_flag "")
endif()

find_package(OpenGL ${_required_dependency_flag})
find_package(QGLViewer ${_required_dependency_flag})
find_package(Qt5 COMPONENTS Core Gui Widgets OpenGL Xml ${_required_dependency_flag})

##################################################

include_directories(
	${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}
)

if(NOT PACKAGE_SOURCE_ONLY)
	include_directories(${QGLVIEWER_INCLUDE_DIR})
endif()

include_directories(
	src
	src/basic
	src/formats
	src/ui
	src/widgets
	3rdparty/GLEW/include
)

set( wmit_HEADERS
	3rdparty/GLEW/include/GL/glew.h
	src/wmit.h
	src/basic/IGLShaderManager.h
	src/basic/IGLShaderRenderable.h
	src/ui/TextureDialog.h
	src/Generic.h
	src/Util.h
	src/widgets/QtGLView.h
	src/ui/ExportDialog.h
	src/ui/ImportDialog.h
	src/ui/MainWindow.h
	src/ui/TexConfigDialog.h
	src/ui/TransformDock.h
	src/ui/UVEditor.h
	src/formats/Mesh.h
	src/formats/OBJ.h
	src/formats/Pie.h
	src/formats/Pie_t.hpp
	src/formats/WZM.h
	src/basic/GLTexture.h
	src/basic/IAnimatable.h
	src/basic/IGLRenderable.h
	src/basic/IGLTexturedRenderable.h
	src/basic/IGLTextureManager.h
	src/basic/Polygon.h
	src/basic/Polygon_t.hpp
	src/basic/Vector.h
	src/basic/VectorTypes.h
	src/widgets/QWZM.h
	src/ui/MaterialDock.h
	src/ui/meshdock.h
)

set( wmit_SRCS
	3rdparty/GLEW/src/glew.c
	src/formats/WZM.cpp
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

set( wmit_INFO
	COPYING.txt
	COPYING.nongpl.txt
	README.md
)

add_executable(wmit ${wmit_SRCS} ${wmit_HEADERS} ${wmit_RSCS} ${wmit_UIS})
set_target_properties(wmit PROPERTIES AUTOMOC TRUE) # handles QT5_WRAP_CPP
set_target_properties(wmit PROPERTIES AUTORCC TRUE) # handles QT5_ADD_RESOURCES
set_target_properties(wmit PROPERTIES AUTOUIC TRUE) # handles QT5_WRAP_UI
if(NOT PACKAGE_SOURCE_ONLY)
	target_link_libraries(wmit OpenGL::GL OpenGL::GLU ${QGLVIEWER_LIB})
	target_link_libraries(wmit Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL Qt5::Xml)
endif()
target_compile_definitions(wmit PRIVATE GLEW_STATIC)
set_target_properties(wmit PROPERTIES OUTPUT_NAME "WMIT")

##################################################
# OS-specific target properties

if(CMAKE_SYSTEM_NAME MATCHES "Windows")

	# Build wmit exe as a Windows app (not a console app)
	set_target_properties(wmit PROPERTIES WIN32_EXECUTABLE TRUE)

	if(CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
		message( WARNING "WindowsStore builds are not currently supported" )
		target_compile_definitions(wmit PRIVATE -D_WIN32_WINNT=0x0A00) # Windows 10+
	else()
		target_compile_definitions(wmit PRIVATE -D_WIN32_WINNT=0x0501) # Windows XP+
	endif()

	if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
		if(${CMAKE_CROSSCOMPILING})
			target_compile_definitions(wmit PRIVATE "QT_STATICPLUGIN")
		endif()
	endif()

	# Copy QGLVIEWER .dll to the build directory
	# Note: This assumes that the .dll exists in the same folder as the .lib
	get_filename_component(_qglviewer_lib_directory "${QGLVIEWER_LIB}" DIRECTORY)
	get_filename_component(_qglviewer_lib_name_no_extension "${QGLVIEWER_LIB}" NAME_WE)
	set(_qglviewer_dll "${_qglviewer_lib_directory}/${_qglviewer_lib_name_no_extension}.dll")
	if(EXISTS "${_qglviewer_dll}")
		message(STATUS "Will copy to build directory: ${_qglviewer_dll}")
		add_custom_command(TARGET wmit POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy ${_qglviewer_dll} ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/
			VERBATIM
		)
	endif()

endif()

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")

	set_target_properties(wmit PROPERTIES MACOSX_BUNDLE TRUE)
	set_target_properties(wmit PROPERTIES OUTPUT_NAME "WMIT")
	set_target_properties(wmit PROPERTIES MACOSX_BUNDLE_INFO_PLIST "WMIT_macOS_Info.plist.in")
	set_target_properties(wmit PROPERTIES XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "net.wz2100.WMIT")
	set_target_properties(wmit PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "net.wz2100.WMIT")
	set_target_properties(wmit PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "WMIT")
	set_target_properties(wmit PROPERTIES MACOSX_BUNDLE_COPYRIGHT "Copyright © 2010-2019 Warzone 2100 Project")

	# Debugging Symbols
	set_target_properties(wmit PROPERTIES XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym")

	# WORKAROUND: Ensure debugging symbols are always generated for Release builds
	#
	# Required because the CMake Xcode generator (at least, as of CMake 3.11.x) automatically
	# sets "Generate Debugging Symbols" to NO for Release builds, and cannot be overridden by
	# setting CMAKE_XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS.
	target_compile_options(wmit PRIVATE $<$<CONFIG:MinSizeRel>:-g>)
	target_compile_options(wmit PRIVATE $<$<CONFIG:Release>:-g>)

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

	# Bundle the INFO files
	set_property(
			SOURCE ${wmit_INFO}
			PROPERTY MACOSX_PACKAGE_LOCATION "Resources")
	target_sources(wmit PRIVATE ${wmit_INFO})

endif()

##################################################
# Installing WMIT

install(TARGETS wmit COMPONENT Core DESTINATION ".")

install(FILES ${wmit_INFO}
	COMPONENT Info
	DESTINATION "."
)

##################################################
# Installing dependencies

get_target_property(_wmit_output_name wmit OUTPUT_NAME)
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	if(NOT CMAKE_CROSSCOMPILING)
		# Install any required runtime dependencies / DLLs
		set(_wz_fixup_bundle_ignored_filenames)
		set(_wz_fixup_bundle_nocopy_libraries)
		if(MSVC)
			# Ignore system (CRT) runtimes in fixup_bundle
			# - Get a list of all of the required system libraries
			set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
			set(CMAKE_INSTALL_DEBUG_LIBRARIES FALSE)
			set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
			set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS TRUE)
			include(InstallRequiredSystemLibraries)
			# - CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS should now contain the runtime files (full paths)
			# - Extract just the filenames
			foreach(lib ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
				get_filename_component(lib_name "${lib}" NAME)
				list(APPEND _wz_fixup_bundle_ignored_filenames "${lib_name}")
				list(APPEND _wz_fixup_bundle_nocopy_libraries "${lib_name}")
			endforeach()
			if(_wz_fixup_bundle_ignored_filenames)
				message( STATUS "fixup_bundle: IGNORE_ITEM ${_wz_fixup_bundle_ignored_filenames}" )
			else()
				message( WARNING "InstallRequiredSystemLibraries returned no libraries? (CMake: ${CMAKE_VERSION}; MSVC: ${MSVC_VERSION})" )
			endif()
		endif()
		get_filename_component(_qglviewer_lib_directory "${QGLVIEWER_LIB}" DIRECTORY)
		set(dll_source_dirs "${_qglviewer_lib_directory}")   # directories to search for prerequisites
		install(CODE "
			set(_ignored_filenames \"${_wz_fixup_bundle_ignored_filenames}\")
			if(_ignored_filenames)
				set(_wz_fixup_bundle_ignore_item \"IGNORE_ITEM \\\"\${_ignored_filenames}\\\"\")
			else()
				set(_wz_fixup_bundle_ignore_item)
			endif()
			set(BU_CHMOD_BUNDLE_ITEMS ON)
			include(BundleUtilities)
			fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/${_wmit_output_name}.exe\" \"\" \"${dll_source_dirs}\" ${_wz_fixup_bundle_ignore_item})
			# Passing IGNORE_ITEM to fixup_bundle does not prevent fixup_bundle from copying the ignored items themselves to the BINDIR
			# Iterate over _wz_fixup_bundle_nocopy_libraries and remove them if they've been copied
			set(_nocopy_libs \"${_wz_fixup_bundle_nocopy_libraries}\")
			foreach(lib \${_nocopy_libs})
				set(_lib_fullpath \"\${CMAKE_INSTALL_PREFIX}/${WZ_APP_INSTALL_DEST}/\${lib}\")
				if(EXISTS \"\${_lib_fullpath}\")
					execute_process(COMMAND \${CMAKE_COMMAND} -E echo \"++Removing lib: \${lib}\")
					file(REMOVE \"\${_lib_fullpath}\")
				endif()
			endforeach()
		" COMPONENT Core)

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

##################################################
# Packaging

add_subdirectory(pkg)

