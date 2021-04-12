#include "pch.h"

LIAPI std::string GetDataPath(const std::string& myname) {
    std::filesystem::create_directory("plugins\\LiteLoader");
    return "plugins\\LiteLoader\\" + myname;
}

void PrintErrorMessage() {
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0) {
		std::wcerr << "Error\n";
		return;
	}
	std::cerr << "[Error] ErrorMessageID: " << errorMessageID << std::endl;
	LPWSTR messageBuffer = nullptr;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, NULL, errorMessageID,
		MAKELANGID(0x09, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);
	std::wcerr << "[Error] " << messageBuffer;
	LocalFree(messageBuffer);
}