:: Copy all .dll files
xcopy /y "setup-assets\*.dll" "BulletHellMaker"

:: Create packages folder if it doesn't exist
if not exist "BulletHellMaker\packages\" mkdir "BulletHellMaker\packages"

:: Unzip Python distribution to packages folder
if not exist "BulletHellMaker\packages\Python27" powershell Expand-Archive "setup-assets\Python27.zip" -DestinationPath "BulletHellMaker\packages"