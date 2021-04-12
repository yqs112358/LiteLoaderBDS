#include "pch.h"

Logger<stdio_commit> LOG(stdio_commit{ "[LL] " });

std::vector<function<void(PostInitEV)>> PostInitCallBacks;
LIAPI void Event::addEventListener(function<void(PostInitEV)> callback) {
	PostInitCallBacks.push_back(callback);
}

void fixUpCWD() {
	string buf;
	buf.assign(8192, '\0');
	GetModuleFileNameA(nullptr, buf.data(), 8192);
	buf = buf.substr(0, buf.find_last_of('\\'));
	SetCurrentDirectoryA(buf.c_str());
}

extern void LoadPlugins();
extern void startWBThread();
extern bool versionCommand(CommandOrigin const& ori, CommandOutput& outp);
extern void updateCheck();

static void entry(bool fixcwd) {
	if (fixcwd)
		fixUpCWD();
	std::filesystem::create_directory("logs");
	Event::addEventListener([](RegCmdEV ev) {
		CMDREG::SetCommandRegistry(ev.CMDRg);
		MakeCommand("version", "Gets the version of this server", 0);
		CmdOverload(version, versionCommand);
		});

	LoadPlugins();
	XIDREG::initAll();
	Event::addEventListener([](ServerStartedEV) {
		startWBThread();

		LOG("LiteLoader is distributed under the GPLv3 License");
#ifdef LiteLoaderVersionGithub
		LOG("Version: " + (std::string)LiteLoaderVersionGithub + " Based on BedrockX Project");
#else
		LOG("Version: " + (std::string)LiteLoaderVersion + " Based on BedrockX Project");
#endif
		LOG("Github: https://git.io/JtwPb");

		updateCheck();
	});

	PostInitEV PostInitEV;
	for (size_t count = 0; count < PostInitCallBacks.size(); count++) {
		PostInitCallBacks[count](PostInitEV);
	}
}

THook(int, "main", int a, void* b) {
	std::ios::sync_with_stdio(false);
	//system("chcp 65001>nul");
	entry(a > 1);
	return original(a, b);
}