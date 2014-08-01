#!/bin/bash

GCCROOT=""
FLAG=""

if [ "$1" == "" ]; then
	GCCROOT="/bin/"
	echo "Using default gcc installation (in /bin/)"
else
	GCCROOT=$1"/"
	echo "Using gcc installation (in $GCCROOT)"
fi

if [ "$2" == "32" ]; then
	FLAG=" -m32 "
	echo "Building 32-Bit Version"
else
	FLAG=" -m64 "
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


cmake -DEXFLAG=$FLAG -DCMAKE_CXX_COMPILER=$GCCROOT/g++ -DCMAKE_CC_COMPILER=$GCCROOT/gcc -DCMAKE_LINKER=$GCCROOT/gcc ../


echo "Ready To Build..."

make
