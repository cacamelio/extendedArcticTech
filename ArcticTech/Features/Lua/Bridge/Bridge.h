#pragma once

#pragma comment(lib, "lua51.lib")
#pragma comment(lib, "luajit.lib")

#include "../Hook/Hook.h"
#include <filesystem>
#include "../../../Utils/Console.h"
#include "../../../SDK/Globals.h"

class CLua
{
public:
	int get_script_id( std::string name );
	std::string get_script_path( std::string name );
	std::string get_script_path( int id );
	void load_script( int id );
	void unload_script( int id );
	void refresh_scripts( );
	void Setup();
	c_lua_hookManager hooks;
	sol::state lua;
	std::vector <bool> loaded;
	std::vector <std::string> scripts;
	std::vector<std::string> scripts_names;
	std::vector <std::filesystem::path> pathes;
};

extern CLua* Lua;