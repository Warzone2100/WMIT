#!/bin/bash

if [ ! -d build  ]
then
    mkdir build
fi

cd build
cmake ../
make

if [ $? != 0 ]
then
    echo
    echo "An error occured during compilation!"
    echo "Check if you have installed all needed libraries and header files."

    exit 1
fi

echo "Done"
echo "Run ./build/wmit"
