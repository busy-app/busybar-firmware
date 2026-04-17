#!/bin/bash

# Version: 1.0 
# Date: 2018-05-25
# This bash script generates a CMSIS Software Pack:
#
# Requirements:
# bash shell
# curl
# zip (zip archiving utility) 
# 

# Pack Vendor
PACK_VENDOR=MDK-Packs
# Pack Name
PACK_NAME=IoT_Socket

# Pack Destination Folder
PACK_DESTINATION=./

# Pack Build Folder
PACK_BUILD=./build

# Pack build utilities Repository
UTILITIES_URL=https://github.com/ARM-software/CMSIS_5/blob/master/
UTILITIES_TAG=1.0.0
UTILITIES_DIR=./Utilities
UTILITIES_OS=Win32
if [ $UTILITIES_OS = "Win32" ]; then
  ZIP="/c/Program\ Files/7-Zip/7z.exe"
else
  ZIP=zip
fi

# if not present, fetch utilities
if [ ! -d $UTILITIES_DIR ]; then
  mkdir $UTILITIES_DIR
  pushd $UTILITIES_DIR
  mkdir $UTILITIES_OS
  # PackChk
  curl -L $UTILITIES_URL/CMSIS/Utilities/$UTILITIES_OS/PackChk.exe?raw=true -o $UTILITIES_OS/PackChk.exe
  popd
fi

#if $PACK_BUILD folder does not exist create it
if [ ! -d $PACK_BUILD ]; then
  mkdir $PACK_BUILD
fi

# Generate documentation
pushd ./documentation
./gen_doc.sh
errorlevel=$?
popd
if [ $errorlevel -ne 0 ]; then
  echo "build aborted: documentation build failed"
  exit
fi

#if $PACK_BUILD/documentation folder does not exist create it
if [ ! -d $PACK_BUILD/documentation ]; then
  mkdir $PACK_BUILD/documentation
fi
# Move built documentation into $PACK_BUILD
mv -v ./documentation/html/* $PACK_BUILD/documentation/
rm -rf ./documentation/html/

# Copy files into $PACK_BUILD
cp -f  ./$PACK_VENDOR.$PACK_NAME.pdsc $PACK_BUILD/ 
cp -f  ./LICENSE.txt $PACK_BUILD/ 
cp -vr ./include $PACK_BUILD/
cp -vr ./source $PACK_BUILD/
cp -vr ./template $PACK_BUILD/

# Run Pack Check and generate PackName file
$UTILITIES_DIR/$UTILITIES_OS/PackChk.exe $PACK_BUILD/$PACK_VENDOR.$PACK_NAME.pdsc -n PackName.txt -x M362
errorlevel=$?

if [ $errorlevel -ne 0 ]; then
  echo "build aborted: pack check failed"
  exit
fi

PACKNAME=`cat PackName.txt`
rm -rf PackName.txt

# Archiving
# $ZIP a $PACKNAME
pushd $PACK_BUILD
/c/Program\ Files/7-Zip/7z.exe a ../$PACKNAME -tzip
popd
errorlevel=$?
if [ $errorlevel -ne 0 ]; then
  echo "build aborted: archiving failed"
  exit
fi

echo "build of pack succeeded"
# Clean up
echo "cleaning up"
rm -rf $PACK_BUILD
rm -rf $UTILITIES_DIR
