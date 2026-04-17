#!/bin/bash
# Version: 1.0
# Date: 2022-01-12
# This bash script generates IoT Socket documentation
#
# Pre-requisites:
# - bash shell (for Windows: install git for Windows)
# - doxygen 


set -o pipefail

DIRNAME=$(dirname $(readlink -f $0))
DOXYGEN=$(which doxygen 2>/dev/null)
REQ_DXY_VERSION="1.9.2"
REGEN=0

if [[ -z "$*" ]]; then
    REGEN=1
fi

if [[ ! -f "${DOXYGEN}" ]]; then
    echo "Doxygen not found!" >&2
    echo "Did you miss to add it to PATH?"
    exit 1
else
    version=$("${DOXYGEN}" --version | sed -E 's/.*([0-9]+\.[0-9]+\.[0-9]+).*/\1/')
    echo "Doxygen is ${DOXYGEN} at version ${version}"
    if [[ "${version}" != "${REQ_DXY_VERSION}" ]]; then
        echo "Doxygen required to be at version ${REQ_DXY_VERSION}!" >&2
        exit 1
    fi
fi

function doxygen {
    echo "${DOXYGEN} $1"
    "${DOXYGEN}" $(basename "$1")

    if [[ $2 != 0 ]]; then
      mkdir -p "${DIRNAME}/html/search/"
      cp -f "${DIRNAME}/Doxygen_Templates/search.css" "${DIRNAME}/html/search/"
    fi
        
    projectName=$(grep -E "PROJECT_NAME\s+=" $1 | sed -r -e 's/[^"]*"([^"]+)".*/\1/')
    projectNumber=$(grep -E "PROJECT_NUMBER\s+=" $1 | sed -r -e 's/[^"]*"([^"]+)".*/\1/')
    datetime=$(date -u +'%a %b %e %Y %H:%M:%S')
    sed -e "s/{datetime}/${datetime}/" "${DIRNAME}/Doxygen_Templates/footer.js" \
       | sed -e "s/{projectName}/${projectName}/" \
       | sed -e "s/{projectNumber}/${projectNumber}/" \
       > "${DIRNAME}/html/footer.js"
}

if [[ $REGEN != 0 ]]; then
    echo "Cleaning existing documentation ..."
    rm -rf "${DIRNAME}/html/"
fi

echo "Generating documentation ..."
doxygen "${DIRNAME}/IoT_Socket.dxy" 1
exit 0
