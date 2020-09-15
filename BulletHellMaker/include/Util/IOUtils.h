#pragma once
#include <string>
#include <vector>
#include <utility>
#include <sys/stat.h>
#include <Windows.h>
#include <ShlObj_core.h>

/*
Returns whether a file exists.

name - the path to the file
*/
bool fileExists(char* name);
/*
Returns whether a file exists.

name - the path to the file
*/
bool fileExists(const std::string& name);

/*
Returns the absolute path to the folder containing this program.
*/
std::string getPathToFolderContainingExe();

/*
Opens a prompt for the user to select a folder limited to a subdirectory.

hwnd - the window handler
lpszFolder - the subdirectory from which the user can choose a folder
lpszTitle - the prompt to show to the user
*/
BOOL BrowseFolder(HWND hwnd, LPSTR lpszFolder, LPSTR lpszTitle);

/*
Returns whether a file extension (such as ".txt") can be read as an
image by SFML.
*/
bool imageExtensionIsSupportedBySFML(const char* extension);

/*
Returns a list of sprite sheets that have a corresponding metafile.
Each pair in the list is, respectively, the metafile name with extension and the image file name with extension.

spriteSheetsFolderPath - path to the folder containing the sprite sheets and metafiles
*/
std::vector<std::pair<std::string, std::string>> findAllSpriteSheetsWithMetafiles(std::string spriteSheetsFolderPath);

/*
Shows an error dialog box explaining an error code from
https://docs.microsoft.com/en-us/windows/win32/debug/system-error-codes.
*/
void showWindowsErrorDialog(DWORD errorCode, LPCWSTR dialogTitle);