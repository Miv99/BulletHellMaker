@setlocal enableextensions enabledelayedexpansion

:: Read setup config file
set cfg=setup_config.txt
for /F "usebackq eol=[ tokens=* delims=" %%a in ("%cfg%") do (
    set "%%a"
)

:: Create folders if they don't exist
if not exist "%CMAKE_BUILD_FOLDER%" mkdir "%CMAKE_BUILD_FOLDER%"
if not exist "%CMAKE_BUILD_FOLDER%\packages\" mkdir "%CMAKE_BUILD_FOLDER%\packages"

:: Copy all .dll files
xcopy /y "setup-assets\*.dll" "%CMAKE_BUILD_FOLDER%"

:: Unzip Python distribution to packages folder
if not exist "%CMAKE_BUILD_FOLDER%\packages\Python27" powershell Expand-Archive "setup-assets\Python27.zip" -DestinationPath "%CMAKE_BUILD_FOLDER%\packages"