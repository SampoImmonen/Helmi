#include "FileHandler.h"

std::string FileHandler::openFilePath(const char* filter, HWND owner)
{
	//returns empty string if canceled
	OPENFILENAMEA ofn;
	CHAR fileName[MAX_PATH] = {0};
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = owner;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = "";
	std::string fileNameStr;

	if (GetOpenFileName(&ofn)) {
		fileNameStr = fileName;
		return fileNameStr;
	}
	return std::string();
}

std::string FileHandler::saveFilePath(const char* filter, HWND owner)
{
	return std::string();
}
