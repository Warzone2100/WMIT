#!/bin/sh

QMAKE_BIN=
if [ -x "$(command -v qmake)" ]; then
	QMAKE_BIN=qmake
elif [ -x "$(command -v qmake-qt5)" ]; then
	QMAKE_BIN=qmake-qt5
fi

if [ -z "$QMAKE_BIN" ]; then
	echo "error: Cannot find: qmake (or qmake-qt5)"
	echo "\tPlease ensure that Qt5 is installed."
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
cd ..
cd 3rdparty/libQGLViewer/QGLViewer
$QMAKE_BIN PREFIX=$(pwd)/../../../build/libQGLViewer/installed/ QGLViewer.pro
make && make install
cd ../../..
if [ -n "${TRAVIS}" ]; then
	echo "travis_fold:end:qmake.QGLViewer.pro"
fi
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

