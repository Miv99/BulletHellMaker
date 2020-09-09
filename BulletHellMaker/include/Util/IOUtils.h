#pragma once
#include <string>
#include <sys/stat.h>
#include <Windows.h>
#include <ShlObj_core.h>

static bool fileExists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

BOOL BrowseFolder(HWND hwnd, LPSTR lpszFolder, LPSTR lpszTitle);