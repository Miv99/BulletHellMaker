#pragma once
#include <static_block.hpp>

#include <Windows.h>
#include <WinUser.h>

static HCURSOR DEFAULT_CURSOR = LoadCursorA(NULL, IDC_ARROW);
static HCURSOR LOADING_CURSOR = LoadCursorA(NULL, IDC_APPSTARTING);
static HCURSOR RESIZE_NS_CURSOR = LoadCursorA(NULL, IDC_SIZENS);
static HCURSOR RESIZE_WE_CURSOR = LoadCursorA(NULL, IDC_SIZEWE);
static HCURSOR RESIZE_NWSE_CURSOR = LoadCursorA(NULL, IDC_SIZENWSE);
static HCURSOR RESIZE_NESW_CURSOR = LoadCursorA(NULL, IDC_SIZENESW);