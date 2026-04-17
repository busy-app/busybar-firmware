#!/bin/bash

# Navigate to the doxygen directory
cd ../doxygen

# Remove the out_doxygen directory if it exists
if [ -d "out_doxygen" ]; then
    rm -rf out_doxygen
fi

# Run doxygen with the specified configuration file
doxygen rail_sdk.doxyfile

# Navigate back to the suds directory
cd ../suds

# Remove the _sdm directory if it exists
if [ -d "_sdm" ]; then
    rm -rf _sdm
fi

# Run the suds compile command with the specified configuration file
suds compile -c _docleaf-sld508-rail-sdk-services.yml --verbose
