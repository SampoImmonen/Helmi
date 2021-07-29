#pragma once
#include "windows.h"

#include <string>
#include <commdlg.h>

class FileHandler
{
public:
	static std::string openFilePath(const char* filter = "All Files (*.*)\0*.*\0", HWND owner = NULL);
	static std::string saveFilePath(const char* filter = "All Files (*.*)\0*.*\0", HWND owner = NULL);
};

