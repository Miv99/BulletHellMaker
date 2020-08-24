@setlocal enableextensions enabledelayedexpansion

:: Read setup config file
set cfg=setup_config.txt
for /F "usebackq eol=[ tokens=* delims=" %%a in ("%cfg%") do (
    set "%%a"
)

:: Create folders if they don't exist
if not exist "%EXE_TARGET_FOLDER%" mkdir "%EXE_TARGET_FOLDER%"
if not exist "%EXE_TARGET_FOLDER%\packages\" mkdir "%EXE_TARGET_FOLDER%\packages"

:: Unzip common-dlls, debug-dlls, and release-dlls into game folder
powershell Expand-Archive -Force "setup-assets\common-dlls.zip" -DestinationPath "%EXE_TARGET_FOLDER%"
powershell Expand-Archive -Force "setup-assets\debug-dlls.zip" -DestinationPath "%EXE_TARGET_FOLDER%"
powershell Expand-Archive -Force "setup-assets\release-dlls.zip" -DestinationPath "%EXE_TARGET_FOLDER%"

:: Unzip BulletHellMaker assets to game folder
if not exist "%EXE_TARGET_FOLDER%\Shaders" powershell Expand-Archive -Force "setup-assets\Game-Assets.zip" -DestinationPath "%EXE_TARGET_FOLDER%"

:: Unzip sample Level Packs to game folder
if not exist "%EXE_TARGET_FOLDER%\Level Packs" powershell Expand-Archive -Force "setup-assets\Level-Packs.zip" -DestinationPath "%EXE_TARGET_FOLDER%"

:: Unzip Python distribution to packages folder
if not exist "%EXE_TARGET_FOLDER%\packages\Python27" powershell Expand-Archive -Force "setup-assets\Python27.zip" -DestinationPath "%EXE_TARGET_FOLDER%\packages"