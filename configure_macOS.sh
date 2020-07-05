#!/bin/sh

# To call this script:
# - set a QT5_DIR environment variable pointing to Qt5's install prefix
# - the PATH environment variable should contain the Qt5's "bin" folder (containing qmake)
# Example:
#	PATH=/Qt/5.9.7/clang_64/bin:$PATH QT5_DIR="/Qt/5.9.7/" ./configure_macOS.sh

if [[ ! "0" == "$(type -aP qmake &> /dev/null; echo ${?})" ]]; then
	echo "error: Cannot find: qmake"
	echo "\tPlease ensure that Qt5 is installed, and its bin/ folder is in the PATH environment variable."
	echo "\tOr execute this script prepending a modified PATH including Qt5's bin/ folder."
	echo "\tExample:"
	echo "\t\tPATH=/Qt/5.9.7/clang_64/bin:\$PATH ./configure_macOS.sh"
	exit 1
fi

# Create the build directory
if [ ! -d "build" ]; then
	mkdir build
fi
cd build

# Compile libQGLViewer (using qmake)
if [ -n "${TRAVIS}" ]; then
	echo "travis_fold:start:qmake.QGLViewer.pro"
fi
echo "Compile libQGLViewer"
if [ ! -d "libQGLViewer" ]; then
	mkdir libQGLViewer
fi
cd libQGLViewer
qmake PREFIX=$(pwd)/installed/ ../../3rdparty/libQGLViewer/QGLViewer/QGLViewer.pro
make && make install
if [ -n "${TRAVIS}" ]; then
	echo "travis_fold:end:qmake.QGLViewer.pro"
fi
# Fixup QGLViewer.framework install_name
Expected_Framework_Library=installed/lib/QGLViewer.framework/QGLViewer
if [ -f "$Expected_Framework_Library" ]; then
	OtoolOutput="$(otool -D $Expected_Framework_Library)"
	# use echo to collapse newlines + whitespace, and sed to remove the prefix
	CurrentInstallName="$(echo $OtoolOutput | sed -e "s|^.*$Expected_Framework_Library: ||")"
	echo "CurrentInstallName=$CurrentInstallName"
	if [ ! [["$CurrentInstallName" = *"is not an object file"]] ]; then
		# found object file
		# check if the CurrentInstallName starts with @rpath/
		if [ ! [["$CurrentInstallName" = "@rpath/"*]] ]; then
			FixedInstallName="@rpath/$CurrentInstallName"
			# The install name requires adjustment to have @rpath/ prepended
			echo "info: Adjusting install name from (\"$CurrentInstallName\") -> (\"$FixedInstallName\")."
			# Use install_name_tool to adjust the install name
			install_name_tool -id "$FixedInstallName" "$Expected_Framework_Library"
		fi
	fi
else
	echo "Failed to find: $(pwd)/$Expected_Framework_Library"
	exit 1
fi
cd .. > /dev/null
export QGLVIEWERROOT=$(pwd)/libQGLViewer/installed/lib/

# Use CMake to configure Xcode project
if [ -n "${TRAVIS}" ]; then
	echo "travis_fold:start:cmake.configure"
fi
echo "Generate Xcode project (using CMake)"
cmake -G"Xcode" -DCMAKE_PREFIX_PATH="$QT5_DIR" -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY= -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGN_INJECT_BASE_ENTITLEMENTS=NO ../
cmakeConfigureResult=${?}
cd .. > /dev/null
if [ -n "${TRAVIS}" ]; then
	echo "travis_fold:end:cmake.configure"
fi

if [ $cmakeConfigureResult -ne 0 ]; then
	echo "error: cmake configure failed"
	exit $cmakeConfigureResult
fi

echo "The Xcode project has been generated at: $(pwd)/build/wmit.xcodeproj"
