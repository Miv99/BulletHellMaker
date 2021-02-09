#include <Util/IOUtils.h>

#include <stdlib.h>
#include <functional>
#include <filesystem>
#include <set>

#include <Constants.h>

LPITEMIDLIST ConvertPathToLpItemIdList(const char* pszPath) {
	LPITEMIDLIST  pidl = NULL;
	LPSHELLFOLDER pDesktopFolder = NULL;
	OLECHAR       olePath[MAX_PATH];
	ULONG         chEaten;
	HRESULT       hr;

	if (SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder))) {
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszPath, -1,
			olePath, MAX_PATH);
		hr = pDesktopFolder->ParseDisplayName(NULL, NULL,
			olePath, &chEaten, &pidl, NULL);
		pDesktopFolder->Release();
		return pidl;
	}
	return NULL;
}

int CALLBACK BrowseForFolderCallback(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData) {
	char szPath[MAX_PATH];

	switch (uMsg) {
	case BFFM_INITIALIZED:
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, pData);
		break;

	case BFFM_SELCHANGED:
		if (SHGetPathFromIDList((LPITEMIDLIST)lp, szPath)) {
			SendMessage(hwnd, BFFM_SETSTATUSTEXT, 0, (LPARAM)szPath);
		}
		break;
	}

	return 0;
}

bool fileExists(const char* name) {
	return std::filesystem::exists(name);
}

bool fileExists(const std::string& name) {
	return std::filesystem::exists(name) && std::filesystem::is_regular_file(name);
}

int countFiles(const char* directory, const char* extension) {
	if (!std::filesystem::exists(directory)) {
		return -1;
	}

	int count = 0;
	char buffer[MAX_PATH + 1];
	if (strlen(extension) > 0) {
		for (const auto& entry : std::filesystem::directory_iterator(directory)) {
			if (entry.is_regular_file()) {
				_splitpath(entry.path().string().c_str(), NULL, NULL, NULL, buffer);
				if (strcmp(extension, buffer) == 0) {
					count++;
				}
			}
		}
	} else {
		for (const auto& entry : std::filesystem::directory_iterator(directory)) {
			if (entry.is_regular_file()) {
				count++;
			}
		}
	}
	
	return count;
}

std::string getPathToFolderContainingExe() {
	char curDirectory[MAX_PATH + 1];
	GetModuleFileNameA(GetModuleHandle(0), curDirectory, sizeof(curDirectory));
	std::string asCppStr(curDirectory);
	return asCppStr.substr(0, asCppStr.find_last_of('\\'));
}

BOOL BrowseFolder(HWND hwnd, LPSTR lpszFolder, LPSTR lpszTitle) {
	BROWSEINFO bi;
	char szPath[MAX_PATH + 1];
	LPITEMIDLIST pidl;
	BOOL bResult = FALSE;

	LPMALLOC pMalloc;

	if (SUCCEEDED(SHGetMalloc(&pMalloc))) {
		bi.hwndOwner = hwnd;
		bi.pidlRoot = NULL;
		bi.pszDisplayName = NULL;
		bi.lpszTitle = lpszTitle;
		bi.ulFlags = BIF_STATUSTEXT; //BIF_EDITBOX 
		bi.lpfn = BrowseForFolderCallback;
		bi.lParam = (LPARAM)lpszFolder;
		bi.pidlRoot = ConvertPathToLpItemIdList(lpszFolder);

		pidl = SHBrowseForFolder(&bi);
		if (pidl) {
			if (SHGetPathFromIDList(pidl, szPath)) {
				bResult = TRUE;
				strcpy(lpszFolder, szPath);
			}

			pMalloc->Free(pidl);
			pMalloc->Release();
		}
	}

	return bResult;
}

bool imageExtensionIsSupportedBySFML(const char* extension) {
	if (strcmp(extension, ".bmp") == 0 || strcmp(extension, ".png") == 0 || strcmp(extension, ".tga") == 0
		|| strcmp(extension, ".jpg") == 0 || strcmp(extension, ".gif") == 0 || strcmp(extension, ".psd") == 0
		|| strcmp(extension, ".hdr") == 0 || strcmp(extension, ".pic") == 0) {
		return true;
	}
	return false;
}

std::vector<std::pair<std::string, std::string>> findAllSpriteSheetsWithMetafiles(std::string spriteSheetsFolderPath) {
	std::vector<std::pair<std::string, std::string>> results;

	std::set<std::pair<std::string, std::string>> fileNamesAndExtensions;
	char fileName[MAX_PATH + 1];
	char fileExtension[MAX_PATH + 1];
	// Insert all files in the sprite sheets folder into fileNamesWithExtension
	for (const auto& entry : std::filesystem::directory_iterator(spriteSheetsFolderPath)) {
		_splitpath(entry.path().string().c_str(), NULL, NULL, fileName, fileExtension);
		fileNamesAndExtensions.insert(std::make_pair(std::string(fileName), std::string(fileExtension)));
	}

	for (std::pair<std::string, std::string> fileNameAndExtension : fileNamesAndExtensions) {
		std::string fileName = fileNameAndExtension.first;
		std::string extension = fileNameAndExtension.second;

		if (imageExtensionIsSupportedBySFML(extension.c_str())) {
			char imageNameAndExtension[MAX_PATH + 1];
			strcpy(imageNameAndExtension, fileName.c_str());
			strcat(imageNameAndExtension, extension.c_str());

			if (fileNamesAndExtensions.find(std::make_pair(std::string(imageNameAndExtension), LEVEL_PACK_SERIALIZED_DATA_FORMAT)) != fileNamesAndExtensions.end()) {
				// Sprite sheet image has a corresponding metafile

				char metafileNameAndExtension[MAX_PATH + 1];
				strcpy(metafileNameAndExtension, imageNameAndExtension);
				strcat(metafileNameAndExtension, LEVEL_PACK_SERIALIZED_DATA_FORMAT.c_str());

				results.push_back(std::make_pair(std::string(metafileNameAndExtension), std::string(imageNameAndExtension)));
			} else {
				// Sprite sheet image doesn't have a corresponding metafile
			}
		}
	}

	return results;
}

std::vector<std::pair<std::string, std::string>> findAllSpriteSheets(std::string spriteSheetsFolderPath) {
	std::vector<std::pair<std::string, std::string>> results;

	std::set<std::pair<std::string, std::string>> fileNamesAndExtensions;
	char fileName[MAX_PATH + 1];
	char fileExtension[MAX_PATH + 1];
	// Insert all files in the sprite sheets folder into fileNamesWithExtension
	for (const auto& entry : std::filesystem::directory_iterator(spriteSheetsFolderPath)) {
		_splitpath(entry.path().string().c_str(), NULL, NULL, fileName, fileExtension);
		fileNamesAndExtensions.insert(std::make_pair(std::string(fileName), std::string(fileExtension)));
	}

	for (std::pair<std::string, std::string> fileNameAndExtension : fileNamesAndExtensions) {
		std::string fileName = fileNameAndExtension.first;
		std::string extension = fileNameAndExtension.second;

		if (imageExtensionIsSupportedBySFML(extension.c_str())) {
			char imageNameAndExtension[MAX_PATH + 1];
			strcpy(imageNameAndExtension, fileName.c_str());
			strcat(imageNameAndExtension, extension.c_str());

			if (fileNamesAndExtensions.find(std::make_pair(std::string(imageNameAndExtension), LEVEL_PACK_SERIALIZED_DATA_FORMAT)) != fileNamesAndExtensions.end()) {
				// Sprite sheet image has a corresponding metafile

				char metafileNameAndExtension[MAX_PATH + 1];
				strcpy(metafileNameAndExtension, imageNameAndExtension);
				strcat(metafileNameAndExtension, LEVEL_PACK_SERIALIZED_DATA_FORMAT.c_str());

				results.push_back(std::make_pair(std::string(metafileNameAndExtension), std::string(imageNameAndExtension)));
			} else {
				// Sprite sheet image doesn't have a corresponding metafile

				results.push_back(std::make_pair("", std::string(imageNameAndExtension)));
			}
		}
	}

	return results;
}

void showWindowsErrorDialog(DWORD errorCode, LPCWSTR dialogTitle) {
	wchar_t err[256];
	memset(err, 0, 256);
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), err, 255, NULL);
	MessageBoxW(NULL, err, dialogTitle, MB_OK);
}

std::string getCurDateTimeInWindowsFileNameCompliantFormat() {
	time_t t = time(0);
	struct tm* now = localtime(&t);
	char buffer[MAX_PATH + 1];
	strftime(buffer, MAX_PATH + 1, "%Y-%m-%d-%T", now);
	std::string curTime(buffer);
	std::replace(curTime.begin(), curTime.end(), ':', '.');
	return curTime;
}
