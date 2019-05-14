#!/bin/sh

if ! [ -x "$(command -v qmake)" ]; then
	echo "error: Cannot find: qmake"
	echo "\tPlease ensure that Qt5 is installed."
	exit 1
fi

# Create the build directory
if [ ! -d "build" ]; then
	mkdir build
fi
cd build

# Compile libQGLViewer (using qmake)
echo "Compile libQGLViewer"
if [ ! -d "libQGLViewer" ]; then
	mkdir libQGLViewer
fi
cd ..
cd 3rdparty/libQGLViewer/QGLViewer
qmake PREFIX=$(pwd)/../../../build/libQGLViewer/installed/ QGLViewer.pro
make && make install
cd ../../..
cd build
export QGLVIEWERROOT=$(pwd)/libQGLViewer/installed/

# CMake configure
echo "CMake configure"
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ../
cmakeConfigureResult=${?}
cd .. > /dev/null

if [ $cmakeConfigureResult -ne 0 ]; then
	echo "error: cmake configure failed"
	exit $cmakeConfigureResult
fi

