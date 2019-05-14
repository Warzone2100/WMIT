cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0054 NEW)

set(CPACK_PACKAGE_NAME			"WMIT")
set(CPACK_PACKAGE_VENDOR		"Warzone 2100 Project")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING.txt")
set(CPACK_PACKAGE_CHECKSUM		SHA256)

#########################
# Default components that are added to package

set(CPACK_COMPONENTS_ALL Core Info)
set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)

#########################
# Default component info

set(CPACK_COMPONENT_CORE_DISPLAY_NAME "Application")
set(CPACK_COMPONENT_CORE_REQUIRED ON)
set(CPACK_COMPONENT_CORE_DESCRIPTION "Application")
set(CPACK_COMPONENT_CORE_GROUP "CoreGroup")

set(CPACK_COMPONENT_INFO_DISPLAY_NAME "Info")
set(CPACK_COMPONENT_INFO_REQUIRED ON)
set(CPACK_COMPONENT_INFO_DESCRIPTION "Info Files")
set(CPACK_COMPONENT_INFO_GROUP "CoreGroup")

set(CPACK_COMPONENT_GROUP_COREGROUP_DISPLAY_NAME "Core files")
set(CPACK_COMPONENT_GROUP_COREGROUP_REQUIRED ON)
set(CPACK_COMPONENT_GROUP_COREGROUP_DESCRIPTION "The core files required to run WMIT.")

#########################

if(NOT DEFINED CPACK_MODULE_PATH)
	set(CPACK_MODULE_PATH)
endif()

if(NOT DEFINED CPACK_PACKAGE_FILE_NAME)
	# Default of "wmit"
	set(CPACK_PACKAGE_FILE_NAME "wmit")
endif()

if(NOT DEFINED CPACK_SOURCE_PACKAGE_FILE_NAME)
	# Default of "wmit"
	set(CPACK_SOURCE_PACKAGE_FILE_NAME "wmit")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	# When targeting Windows, build the NSIS installer

	set(CPACK_GENERATOR "NSIS")

	set(PACKAGE_NAME "Warzone Model Import Tool")
	set(CPACK_NSIS_DISPLAY_NAME "${PACKAGE_NAME}")
	set(CPACK_NSIS_PACKAGE_NAME "${PACKAGE_NAME}")

	# NSIS installer should always have component installation enabled
	set(CPACK_NSIS_COMPONENT_INSTALL ON)

	# Additional information
	set(CPACK_NSIS_HELP_LINK "http://wz2100.net")
	set(CPACK_NSIS_URL_INFO_ABOUT "http://wz2100.net")

elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
	# macOS builds

	# Only install the "Core" component (the application), as the macOS app bundle contains all of the other files
	set(CPACK_COMPONENTS_ALL Core)
	unset(CPACK_COMPONENT_CORE_DEPENDS)
	set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)

	# Default package generators on macOS
	set(CPACK_GENERATOR "ZIP")

	# Support Archive generator
	set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY FALSE)

endif()

#####################
# Package Source

set(CPACK_SOURCE_GENERATOR "TXZ")

if(NOT DEFINED CPACK_SOURCE_IGNORE_FILES)
	set (CPACK_SOURCE_IGNORE_FILES)
endif()

# Ignore version control
list (APPEND CPACK_SOURCE_IGNORE_FILES "\\\\.#")
list (APPEND CPACK_SOURCE_IGNORE_FILES "/#")
list (APPEND CPACK_SOURCE_IGNORE_FILES "/\\\\.git")
list (APPEND CPACK_SOURCE_IGNORE_FILES "/\\\\.svn")
list (APPEND CPACK_SOURCE_IGNORE_FILES "/\\\\.hg/")
list (APPEND CPACK_SOURCE_IGNORE_FILES "/\\\\.bzr/")

# Ignore development / runtime created files
list (APPEND CPACK_SOURCE_IGNORE_FILES "~$")
list (APPEND CPACK_SOURCE_IGNORE_FILES "\\\\.DS_Store$")

# The build directory (if user configured to build in the core repo folder)
list (APPEND CPACK_SOURCE_IGNORE_FILES "/build/")

#####################

INCLUDE(CPack)

