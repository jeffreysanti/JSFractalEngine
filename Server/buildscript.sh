#!/bin/bash

GCCROOT=""
FLAG=""

GCC=$1
GXX=$2
LD=$2

if [ "$4" == "32" ]; then
	FLAG="-m32 "
	echo "Building 32-Bit Version"
else
	FLAG="-m64 "
	echo "Building 64-Bit Version"
fi

# Get Directory
SCRIPT=$(readlink -f "$0")
SCRIPTPATH=$(dirname "$SCRIPT")

BUILDDIR=$SCRIPTPATH"/build/"
rm -fr $BUILDDIR
mkdir $BUILDDIR

cd "$BUILDDIR"
pwd

echo "Calling CMake..."

export CC=$GCC
export CXX=$GXX
cmake -DEXFLAG=$FLAG -DCMAKE_CXX_COMPILER=$GXX -DCMAKE_C_COMPILER=$GCC -DCMAKE_LINKER=$GXX ../


echo "Ready To Build..."

make