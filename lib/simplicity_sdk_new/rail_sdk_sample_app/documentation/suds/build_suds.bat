@echo off

REM Navigate to the doxygen directory
cd ..\doxygen

REM Remove the out_doxygen directory if it exists
if exist out_doxygen (
    rmdir /s /q out_doxygen
)

REM Run doxygen with the specified configuration file
doxygen rail_sdk.doxyfile

REM Navigate back to the suds directory
cd ..\suds

REM Remove the _sdm directory if it exists
if exist _sdm (
    rmdir /s /q _sdm
)

REM Run the suds compile command with the specified configuration file
suds compile -c _docleaf-sld508-rail-sdk-services.yml --verbose
