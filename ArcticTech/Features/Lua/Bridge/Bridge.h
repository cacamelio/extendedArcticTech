#pragma once

#include <filesystem>
#include <string>
#include <vector>

#pragma comment(lib, "lua51.lib")
#pragma comment(lib, "luajit.lib")

#include "../Hook/Hook.h"
#include "../../../Utils/Console.h"
#include "../../../SDK/Globals.h"

struct LuaScript_t {
	bool loaded = false;
	std::string name;
	std::string ui_name; // added star if loaded
	std::filesystem::path path;
	sol::environment* env = nullptr;
	std::vector<CMenuTab*> tabs;
	std::vector<CMenuGroupbox*> groupboxes;
	std::vector<IBaseWidget*> ui_elements;
};

struct CUserCmd_lua {
	int command_number = 0;
	int tickcount = 0;
	Vector move;
	QAngle viewangles;

	int buttons = 0;

	int random_seed = false;
	bool hasbeenpredicted = false;

	sol::object override_defensive = sol::nil;
	bool allow_defensive = true;
};

class CLua
{
public:
	int GetScriptID( std::string name );
	std::string GetScriptPath( std::string name );
	std::string GetScriptPath( int id );
	std::vector<std::string> GetUIList();

	void LoadScript( int id );
	void UnloadScript( int id );
	void UnloadAll();
	void ReloadAll();

	void RefreshScripts();
	void RefreshUI();

	void Setup();

	CLuaHookManager hooks;
	sol::state lua;
	std::vector<LuaScript_t> scripts;
};

extern CLua* Lua;