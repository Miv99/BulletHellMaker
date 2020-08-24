# BulletHellMaker
A game focused on user-created bullet hell levels. Supports only Windows.

## Development setup
1. Check setup_config.txt and change EXE_TARGET_FOLDER to your desired game root folder.
2. Run setup.bat. This will automatically unzip sample level packs, BulletHellMaker assets, pre-built x86 Windows binaries for 
[SFML 2.5.1](https://github.com/SFML/SFML/releases/tag/2.5.1) in both debug and release, and a portable distribution of Python 2.7 with 
required packages pre-installed, into the game root folder. This Python will not be installed (only used as a portable version) and so will not 
change any environmental variables. If you want to install Python yourself, download Python 2.7 and install (in order) numpy 1.16.5, kiwisolver 1.1.0, and matplotlib 2.2.3.
3. Build TGUI from [this](https://github.com/Miv99/TGUI) fork of [texus/TGUI](https://github.com/texus/TGUI). 
Copy the TGUI dlls (tgui-d.dll for debug, tgui.dll for release) into the game root folder.
4. Download mpg123 from [here](https://www.mpg123.de/) and copy either the debug or release libmpg123-0.dll into the game root folder.
5. Build BulletHellMaker.
6. Copy the generated BHM.exe from step 5 to the game root folder.
7. Run BHM.exe.
8. Each time you rebuild BulletHellMaker, you must repeat step 6. Alternatively, you can move all the files in the game root folder to where BHM.exe is generated.

If you want to run BulletHellMaker tests:
1. Build Google test from [here](https://github.com/google/googletest). Make sure to build both googletest and googlemock.
2. Set BulletHellMaker cmake option CMAKE_BUILD_TYPE to Debug, BUILD_TESTS to true, GTEST_ROOT to gtest's root folder, and GTEST_BUILD to gtest's cmake build folder.
3. Build BulletHellMaker's tests.
4. Copy gtestd.dll and gmockd.dll from gtest's build into the same folder as the generated BHM_test.exe.
5. Copy SFML, TGUI, and mpg123 debug dlls (libmpg123-0.dll, openal32.dll, sfml-audio-d-2.dll, sfml-graphics-d-2.dll, sfml-system-d-2.dll, 
sfml-window-d-2.dll, tgui-d.dll) into the same folder.
6. Run BHM_test.exe to run tests.
7. Each time you rebuild BulletHellMaker's tests, you only have to re-run BHM_test.exe.

### Third-party libraries
Development has been tested only on x86 and with the following library versions:\
[SFML 2.5.1](https://github.com/SFML/SFML/releases/tag/2.5.1)\
[mpg123 1.25.10](https://www.mpg123.de/)\
[My fork of texus/TGUI](https://github.com/Miv99/TGUI)\
Python 2.7 with numpy 1.16.5, kiwisolver 1.1.0, and matplotlib 2.2.3\
[Google Test 1.10.0](https://github.com/google/googletest/releases/tag/release-1.10.0)\
[matplotlib](https://github.com/lava/matplotlib-cpp) (commit 5adfbe031e9a9a50b9f4f054aceba4cfd805fa52)
