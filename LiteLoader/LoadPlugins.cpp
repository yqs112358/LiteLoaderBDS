#include "pch.h"

extern Logger<stdio_commit> LOG;
extern void PrintErrorMessage();

static void AddPluginsLibDir() {
	std::filesystem::create_directory("plugins");

	WCHAR* buffer = new WCHAR[8192];
	auto sz = GetEnvironmentVariableW(TEXT("PATH"), buffer, 8192);
	std::wstring PATH{ buffer, sz };
	sz = GetCurrentDirectoryW(8192, buffer);
	std::wstring CWD{ buffer, sz };
	SetEnvironmentVariableW(TEXT("PATH"), (CWD + L"\\plugins;" + PATH).c_str());
	delete[] buffer;
}

std::vector<std::wstring> GetPreloadList()
{
	std::vector<std::wstring> preloadList{};

	if (std::filesystem::exists(std::filesystem::path(TEXT(".\\plugins\\preload.conf"))))
	{
		std::wifstream dllList(TEXT(".\\plugins\\preload.conf"));
		if (dllList)
		{
			std::wstring dllName;
			while (getline(dllList, dllName))
			{
				if (dllName.back() == TEXT('\n'))
					dllName.pop_back();
				if (dllName.back() == TEXT('\r'))
					dllName.pop_back();

				if (dllName.empty() || dllName.front() == TEXT('#'))
					continue;
				preloadList.push_back(dllName);
			}
			dllList.close();
		}
	}
	return preloadList;
}

void LoadPlugins()
{
	AddPluginsLibDir();
	
	LOG("Loading plugins");
	static std::vector<std::pair<std::wstring, HMODULE>> libs;
	std::filesystem::directory_iterator ent("plugins");
	std::vector<std::wstring> preloadList = GetPreloadList();
	
	int plugins = 0;
	for (auto& i : ent) {
		if (i.is_regular_file() && i.path().extension().u8string() == ".dll") {
			bool loaded = false;
			for (auto& p : preloadList)
				if (p.find(std::wstring(i.path())) != std::wstring::npos)
				{
					loaded = true;
					break;
				}
			if (loaded)
				continue;
			//若在preload.conf中，则不加载

			auto lib = LoadLibrary(i.path().c_str());
			if (lib) {
				plugins++;
				LOG("Plugin " + canonical(i.path()).filename().u8string() + " loaded");
				libs.push_back({ std::wstring{ i.path().c_str() }, lib });
			}
			else {
				LOG("Error when loading " + i.path().filename().u8string() + "");
				PrintErrorMessage();
			}
		}
	}
	for (auto& [name, h] : libs) {
		auto FN = GetProcAddress(h, "onPostInit");
		if (!FN) {
#ifdef _DEBUG
			std::wcerr << "Warning!!! mod" << name << " doesnt have a onPostInit\n";
#endif
		}
		else {
			try {
				((void (*)()) FN)();
			}
			catch (...) {
				std::wcerr << "[Error] plugin " << name << " throws an exception when onPostInit\n";
				exit(1);
			}
		}
	}
	libs.clear();
	LOG(std::to_string(plugins) + " plugin(s) loaded");
}