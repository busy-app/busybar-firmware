#!/bin/bash

if (( $# != 1 )); then
    echo "Usage: $0 [wiseconnect|simplicity_sdk]"
    echo
    echo "Download and copy latest version of a silabs library"
    exit 1
fi

ROOT=$(dirname $0)/..

if [[ "$1" == wiseconnect ]]; then
    PACKAGE=wiseconnect
    DEST_PATH=${ROOT}/lib/wiseconnect
elif [[ "$1" == simplicity_sdk ]]; then
    PACKAGE=simplicity-sdk
    DEST_PATH=${ROOT}/lib/simplicity_sdk
else
    echo "Usage: $0 [wiseconnect|simplicity_sdk]"
    exit 1
fi


SILABS_REPO_URL="https://conan.silabs.net/"

if ! command -v conan > /dev/null; then
    echo "Please install conan: https://conan.io/"
    exit 1
fi

if ! command -v sqlite3 > /dev/null; then
    echo "Please install sqlite3"
    exit 1
fi

conan remote add silabs "$SILABS_REPO_URL" 2>&1 > /dev/null

PACKAGE=$(conan list -r silabs "$PACKAGE" | tail -n 1 | xargs)

if echo "$PACKAGE" | grep -q ERROR; then
    echo "Cannot find package: $PACKAGE"
    exit 1
fi

echo Downloading $PACKAGE...

conan download -r silabs $PACKAGE || exit 1

DATA_PATH=$(sqlite3 $HOME/.conan2/p/cache.sqlite3 "select path from packages where reference=\"$PACKAGE\";")

if [[ -z "$DATA_PATH" ]]; then
    echo "Cannot get package path"
    exit 1
fi

DATA_PATH=${HOME}/.conan2/p/${DATA_PATH}/p

echo Data path: "$DATA_PATH"
echo Destination path: "$DEST_PATH"

mkdir -p "${DEST_PATH}"
rm -rf "${DEST_PATH}"/*

echo "Copying files..."
cp -R "${DATA_PATH}"/* "${DEST_PATH}"

if [[ "$1" == wiseconnect ]]; then
    echo -e "\033[1;35mDon't forget to disable the temperature sensor (rsi_impu.c)!\033[0m"
fi
