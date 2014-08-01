#!/bin/bash

GCCROOT=""

if [ "$1" == "" ]; then
	GCCROOT="/bin/"
	echo "Using default gcc installation (in /bin/)"
else
	GCCROOT=$1"/"
	echo "Using gcc installation (in $GCCROOT)"
fi

# Get Directory
SCRIPT=$(readlink -f "$0")
SCRIPTPATH=$(dirname "$SCRIPT")

BUILDDIR=$SCRIPTPATH"build/"
rm -fr $BUILDDIR
mkdir $BUILDDIR

MAKEFL=$SCRIPTPATH"/Makefile"
echo $MAKEFL
OBJS=""

echo "" > $MAKEFL
echo "CC=$GCCROOT/gcc -Wall -O3 " >> $MAKEFL
echo "CP=$GCCROOT/g++ -Wall -O3 -w -std=c++0x  " >> $MAKEFL
echo "LD=$GCCROOT/gcc -ldl -lpthread -lpng -D_GLIBCXX_USE_NANOSLEEP " >> $MAKEFL

echo "Building Make File..."

# Build C Files

for file in $(find . -name '*.c'); do
	echo "C FILE: $file"
	echo "$BUILDDIR/$file.o : $SCRIPTPATH$file" >> $MAKEFL
	echo "	$""CC -c -o BUILDDIR/$file.o $SCRIPTPATH$file" >> $MAKEFL
	echo "" >> $MAKEFL
	OBJS=$OBJS" "$BUILDDIR"/"$file".o"
done

for file in $(find . -name '*.cpp'); do
	echo "CXX FILE: $file"
	echo "$BUILDDIR/$file.o : $SCRIPTPATH$file" >> $MAKEFL
	echo "	$""CP -c -o BUILDDIR/$file.o $SCRIPTPATH$file" >> $MAKEFL
	echo "" >> $MAKEFL
	OBJS=$OBJS" "$BUILDDIR"/"$file".o"
done

echo "all : $OBJS" >> $MAKEFL
echo "	$""LD -o fractServ $OBJS" >> $MAKEFL


OUTDIR=""

# first 

