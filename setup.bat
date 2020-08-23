@setlocal enableextensions enabledelayedexpansion

:: Read setup config file
set cfg=setup_config.txt
for /F "usebackq eol=[ tokens=* delims=" %%a in ("%cfg%") do (
    set "%%a"
)

:: Create folders if they don't exist
if not exist "%EXE_TARGET_FOLDER%" mkdir "%EXE_TARGET_FOLDER%"
if not exist "%EXE_TARGET_FOLDER%\packages\" mkdir "%EXE_TARGET_FOLDER%\packages"

:: Copy all .dll files
xcopy /y "setup-assets\*.dll" "%EXE_TARGET_FOLDER%"

:: Unzip Python distribution to packages folder
if not exist "%EXE_TARGET_FOLDER%\packages\Python27" powershell Expand-Archive "setup-assets\Python27.zip" -DestinationPath "%EXE_TARGET_FOLDER%\packages"