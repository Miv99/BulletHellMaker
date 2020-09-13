#pragma once
#include <string>
#include <sys/stat.h>
#include <Windows.h>
#include <ShlObj_core.h>

static bool fileExists(char* name) {
	struct stat buffer;
	return (stat(name, &buffer) == 0);
}

static bool fileExists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

static std::string getPathToFolderContainingExe() {
	char curDirectory[MAX_PATH + 1];
	GetModuleFileNameA(GetModuleHandle(0), curDirectory, sizeof(curDirectory));
	std::string asCppStr(curDirectory);
	return asCppStr.substr(0, asCppStr.find_last_of('\\'));
}

BOOL BrowseFolder(HWND hwnd, LPSTR lpszFolder, LPSTR lpszTitle);

static bool imageExtensionIsSupportedBySFML(const char* extension) {
	if (strcmp(extension, ".bmp") == 0 || strcmp(extension, ".png") == 0 || strcmp(extension, ".tga") == 0
		|| strcmp(extension, ".jpg") == 0 || strcmp(extension, ".gif") == 0 || strcmp(extension, ".psd") == 0
		|| strcmp(extension, ".hdr") == 0 || strcmp(extension, ".pic") == 0) {
		return true;
	}
	return false;
}