#include "Bridge.h"

void LuaErrorHandler(sol::optional<std::string> message)
{
	if (!message)
		return;

	Console->ArcticTag();
	Console->ColorPrint(message.value_or("unknown"), Color(255, 0, 0) );
	Console->Print("\n");
}

void ScriptLoadButton()
{
	Lua->LoadScript(Lua->GetScriptID(Config->lua_list->get()));
}

void ScriptUnloadButton()
{
	Lua->UnloadScript(Lua->GetScriptID(Config->lua_list->get()));
}

std::string GetCurrentScript(sol::this_state s) {
	sol::state_view lua_state(s);
	sol::table rs = lua_state["debug"]["getinfo"](2, ("S"));
	std::string source = rs["source"];
	std::string filename = std::filesystem::path(source.substr(1)).filename().string();

	return filename;
}

namespace api {
	void print(std::string msg) {
		Console->Log(msg);
	}

	void error(std::string msg) {
		Console->Error(msg);
	}

	void print_raw(std::string msg, sol::optional<Color> color) {
		Console->ColorPrint(msg, color.value_or(Color(255, 255, 255)));
	}

	namespace client {
		void add_callback(sol::this_state state, std::string event_name, sol::protected_function func) {
			const std::string script_name = GetCurrentScript(state);
			const int script_id = Lua->GetScriptID(script_name);

			if (event_name == "render")
				Lua->hooks.registerHook(LUA_RENDER, script_id, func);
			else if (event_name == "createmove")
				Lua->hooks.registerHook(LUA_CREATEMOVE, script_id, func);
			else if (event_name == "frame_stage")
				Lua->hooks.registerHook(LUA_FRAMESTAGE, script_id, func);
			else if (event_name == "game_events")
				Lua->hooks.registerHook(LUA_GAMEEVENTS, script_id, func);
			else if (event_name == "unload")
				Lua->hooks.registerHook(LUA_UNLOAD, script_id, func);
			else
				Console->Error(std::format("[{}] unknown callback: {}", GetCurrentScript(state), event_name));
		}

		void unload_script(sol::this_state state) {
			Lua->UnloadScript(Lua->GetScriptID(GetCurrentScript(state)));
		}

		void reload_script(sol::this_state state) {
			Lua->UnloadScript(Lua->GetScriptID(GetCurrentScript(state)));
			Lua->LoadScript(Lua->GetScriptID(GetCurrentScript(state)));
		}
	}

	namespace globals {
		bool is_connected() {
			return EngineClient->IsConnected();
		}

		bool is_in_game() {
			return EngineClient->IsInGame();
		}

		int choked_commands() {
			return ClientState->m_nChokedCommands;
		}

		int commandack() {
			return ClientState->m_nCommandAck;
		}

		int commandack_prev() {
			return ClientState->m_nLastCommandAck;
		}

		int last_outgoing_command() {
			return ClientState->m_nLastOutgoingCommand;
		}

		int server_tick() {
			return ClientState->m_ClockDriftMgr.m_nServerTick;
		}

		int client_tick() {
			return ClientState->m_ClockDriftMgr.m_nClientTick;
		}
	}
}

void CLua::Setup() {
	std::filesystem::create_directory(std::filesystem::current_path().string() + "/at/scripts");

	lua = sol::state(sol::c_call<decltype(&LuaErrorHandler), &LuaErrorHandler>);
	lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::math, sol::lib::table, sol::lib::debug, sol::lib::package, sol::lib::bit32, sol::lib::ffi, sol::lib::jit, sol::lib::io, sol::lib::utf8);

	lua.new_usertype<Color>("color", sol::call_constructor, sol::constructors<Color(), Color(int), Color(int, int), Color(int, int, int), Color(int, int, int, int)>(),
		"r", &Color::r,
		"g", &Color::g,
		"b", &Color::b,
		"a", &Color::a,
		"as_int32", &Color::as_int32,
		"as_fraction", &Color::as_fraction,
		"alpha_modulate", &Color::alpha_modulate,
		"alpha_modulatef", &Color::alpha_modulatef,
		"lerp", &Color::lerp,
		"clone", &Color::clone
	);

	// _G
	lua["print"] = api::print;
	lua["error"] = api::error;
	lua["print_raw"] = api::print_raw;

	auto client = lua.create_table();
	client["add_callback"] = api::client::add_callback;
	client["unload_script"] = api::client::unload_script;
	client["reload_script"] = api::client::reload_script;
	lua["client"] = client;

	lua.new_usertype<CGlobalVarsBase>("global_vars_t", sol::no_constructor,
		"curtime", sol::readonly(&CGlobalVarsBase::curtime),
		"realtime", sol::readonly(&CGlobalVarsBase::realtime),
		"frametime", sol::readonly(&CGlobalVarsBase::frametime),
		"framecount", sol::readonly(&CGlobalVarsBase::framecount),
		"absoluteframetime", sol::readonly(&CGlobalVarsBase::absoluteframetime),
		"tickcount", sol::readonly(&CGlobalVarsBase::tickcount),
		"tickinterval", sol::readonly(&CGlobalVarsBase::interval_per_tick),
		"max_players", sol::readonly(&CGlobalVarsBase::max_clients),
		"is_connected", sol::readonly_property(&api::globals::is_connected),
		"is_in_game", sol::readonly_property(&api::globals::is_in_game),
		"choked_commands", sol::readonly_property(&api::globals::choked_commands),
		"commandack", sol::readonly_property(&api::globals::commandack),
		"commandack_prev", sol::readonly_property(&api::globals::commandack_prev),
		"last_outgoing_command", sol::readonly_property(&api::globals::last_outgoing_command),
		"server_tick", sol::readonly_property(&api::globals::server_tick),
		"client_tick", sol::readonly_property(&api::globals::client_tick)
	);

	lua["globals"] = GlobalVars;

	RefreshScripts();
	Config->lua_button->set_callback(ScriptLoadButton);
	Config->lua_button_unload->set_callback(ScriptUnloadButton);
	Config->lua_refresh->set_callback([]() { Lua->RefreshScripts(); });
}

int CLua::GetScriptID(std::string name) {
	for (int i = 0; i < scripts.size(); i++) {
		if (scripts[i].name == name || scripts[i].ui_name == name)
			return i;
	}

	return -1;
}

std::string CLua::GetScriptPath(std::string name) {
	return GetScriptPath(GetScriptID(name));
}

std::string CLua::GetScriptPath( int id ) {
	if (id == -1)
		return  "";

	return scripts[id].path.string();
}

void CLua::LoadScript( int id ) {
	if (id == -1)
		return;

	if (scripts[id].loaded)
		return;

	const std::string path = GetScriptPath( id );

	if (path == "")
		return;

	LuaScript_t& script = scripts[id];

	script.loaded = true;
	script.env = new sol::environment(lua, sol::create, lua.globals());

	sol::environment& env = *script.env;

	bool error_load = false;

	auto load_result_func = [&error_load, script](lua_State* state, sol::protected_function_result result) {
		if (!result.valid()) {
			sol::error error = result;
			Console->Error(error.what());
			error_load = true;
		}

		return result;
	};
	
	lua.script_file(path, env, load_result_func);

	if (error_load)
	{
		script.loaded = false;
		delete script.env;
		script.env = nullptr;

		RefreshUI();
		return;
	}

	RefreshUI();
}

void CLua::UnloadScript(int id) {
	if (id == -1 )
		return;

	LuaScript_t& script = scripts[id];

	if (!script.loaded)
		return;

	for (auto& current : hooks.getHooks(LUA_UNLOAD)) {
		if (current.scriptId == id)
			current.func();
	}

	hooks.unregisterHooks(id);
	script.loaded = false;
	delete script.env;
	script.env = nullptr;

	RefreshUI();
}

void CLua::ReloadAll() {
	hooks.removeAll();

	for (int i = 0; i < scripts.size(); i++) {
		LuaScript_t* script = &scripts[i];

		if (script->loaded) {
			UnloadScript(i);
			LoadScript(i);
		}
	}
}

void CLua::UnloadAll() {
	for (int i = 0; i < scripts.size(); i++) {
		LuaScript_t* script = &scripts[i];

		if (script->loaded) {
			UnloadScript(i);
		}
	}
}

std::vector<std::string> CLua::GetUIList() {
	std::vector<std::string> result;

	for (auto& script : scripts) {
		result.emplace_back(script.ui_name);
	}

	return result;
}

void CLua::RefreshScripts() {
	auto old_scripts = scripts;

	UnloadAll();
	scripts.clear();

	for (auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path().string() + "/at/scripts"))
	{
		if (entry.path().extension() == ".lua")
		{
			LuaScript_t script;

			script.path = entry.path();
			script.name = script.path.filename().string();
			script.loaded = false;

			bool was_loaded = false;

			for (auto& o_script : old_scripts) {
				if (o_script.name == script.name && o_script.loaded) {
					was_loaded = true;
					break;
				}
			}

			script.ui_name = was_loaded ? "* " + script.name : script.name;

			scripts.emplace_back(script);
		}
	}

	Config->lua_list->UpdateList(GetUIList());

	for (auto script : old_scripts) {
		if (script.loaded)
			LoadScript(GetScriptID(script.name));
	}
}

void CLua::RefreshUI() {
	for (auto& script : scripts) {
		script.ui_name = script.loaded ? "* " + script.name : script.name;
	}

	Config->lua_list->UpdateList(GetUIList());
}

CLua* Lua = new CLua;