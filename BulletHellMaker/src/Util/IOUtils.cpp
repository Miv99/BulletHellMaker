#include <Util/IOUtils.h>

#include <stdlib.h>
#include <functional>

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