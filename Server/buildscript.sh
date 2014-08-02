#!/bin/bash

GCCROOT=""
FLAG=""

TYPE=$1
GCC=$2
GXX=$3
LD=$3

if [ $TYPE == "lin32" ]; then
	FLAG="-m32 "
	echo "Building 32-Bit Linux Version"
elif [ $TYPE == "win32" ]; then
	FLAG="-m32 "
	echo "Building 32-Bit Windows Version"
else
	TYPE="lin64"
	FLAG="-m64 "
	echo "Building 64-Bit Linux Version"
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
export LD=$LD

if [ $TYPE == "win32" ]; then
	cmake -DCMAKE_TOOLCHAIN_FILE=Toolchain-w32clang.cmake -DEXFLAG=$FLAG -DBUILD_TYP=$TYPE -DCMAKE_CXX_COMPILER=$GXX -DCMAKE_C_COMPILER=$GCC -DCMAKE_LINKER=$LD ../
else
	cmake -DEXFLAG=$FLAG -DBUILD_TYP=$TYPE -DCMAKE_CXX_COMPILER=$GXX -DCMAKE_C_COMPILER=$GCC -DCMAKE_LINKER=$GXX ../
fi



echo "Ready To Build..."

make
