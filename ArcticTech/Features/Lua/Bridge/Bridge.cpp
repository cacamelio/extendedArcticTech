#include "Bridge.h"

#include "../../ShotManager/ShotManager.h"
#include "../../RageBot/LagCompensation.h"
#include "../../../SDK/Requests.h"

std::vector<UILuaCallback_t> g_ui_lua_callbacks;

void LuaSaveConfig(LuaScript_t* script) {
	nlohmann::json result;

	for (auto e : script->ui_elements) {\
		std::string name = e->parent->name + "_" + e->name + "_" + std::to_string(static_cast<int>(e->GetType()));

		switch (e->GetType()) {
		case WidgetType::Checkbox:
			result[name] = ((CCheckBox*)e)->value;
			break;
		case WidgetType::ColorPicker:
			result[name] = ((CColorPicker*)e)->value;
			break;
		case WidgetType::KeyBind:
			result[name] = { ((CKeyBind*)e)->mode, ((CKeyBind*)e)->key };
			break;
		case WidgetType::SliderInt:
			result[name] = ((CSliderInt*)e)->value;
			break;
		case WidgetType::SliderFloat:
			result[name] = ((CSliderFloat*)e)->value;
			break;
		case WidgetType::Combo:
			result[name] = ((CComboBox*)e)->value;
			break;
		case WidgetType::MultiCombo:
			for (int i = 0; i < ((CMultiCombo*)e)->elements.size(); i++)
				result[name] = ((CMultiCombo*)e)->value[i];
			break;
		case WidgetType::Input:
			result[name] = std::string(((CInputBox*)e)->buf);
			break;
		default:
			// handle any unexpected values of WidgetType here
			break;
		}
	}

	const std::string file_path = std::filesystem::current_path().string() + "/at/scripts/cfg/" + script->name + ".cfg";

	std::ofstream file(file_path);

	file << result;
}

void LuaLoadConfig(LuaScript_t* script) {
	const std::string file_path = std::filesystem::current_path().string() + "/at/scripts/cfg/" + script->name + ".cfg";
	if (!std::filesystem::exists(file_path)) {
		// not saved config for lua yet
		return;
	}

	std::ifstream file(file_path);
	nlohmann::json config_json;
	file >> config_json;

	for (auto e : script->ui_elements) {
		try {
			auto& val = config_json[e->parent->name + "_" + e->name + "_" + std::to_string(static_cast<int>(e->GetType()))];

			switch (e->GetType()) {
			case WidgetType::Checkbox:
				((CCheckBox*)e)->value = val;
				break;
			case WidgetType::ColorPicker:
				((CColorPicker*)e)->value[0] = val[0];
				((CColorPicker*)e)->value[1] = val[1];
				((CColorPicker*)e)->value[2] = val[2];
				((CColorPicker*)e)->value[3] = val[3];
				break;
			case WidgetType::KeyBind:
				((CKeyBind*)e)->mode = val[0];
				((CKeyBind*)e)->key = val[1];
				break;
			case WidgetType::SliderInt:
				((CSliderInt*)e)->value = val;
				break;
			case WidgetType::SliderFloat:
				((CSliderFloat*)e)->value = val;
				break;
			case WidgetType::Combo:
				((CComboBox*)e)->value = val;
				break;
			case WidgetType::MultiCombo:
				for (int i = 0; i < ((CMultiCombo*)e)->elements.size(); i++)
					((CMultiCombo*)e)->value[i] = val[i];
				break;
			case WidgetType::Input: {
				ZeroMemory(((CInputBox*)e)->buf, 64);
				std::string inp = val;
				memcpy(((CInputBox*)e)->buf, inp.c_str(), inp.size());
				break;
			}
			default:
				break;
			}

			for (auto cb : e->callbacks)
				cb();

			for (auto lcb : e->lua_callbacks)
				lcb.func();
		}
		catch (nlohmann::json::exception& ex) {
			Console->Error("could not find item: " + e->name);
		}
	}
}

nlohmann::json luaTableToJson(sol::table table) {
	nlohmann::json result;

	for (const auto& pair : table) {
		if (pair.second.is<sol::table>()) {
			if (pair.first.is<int>())
				result[pair.first.as<int>()] = luaTableToJson(pair.second.as<sol::table>());
			else
				result[pair.first.as<std::string>()] = luaTableToJson(pair.second.as<sol::table>());
		} else {
			if (pair.first.is<int>())
				result[pair.first.as<int>()] = pair.second.as<std::string>();
			else
				result[pair.first.as<std::string>()] = pair.second.as<std::string>();
		}
	}

	return result;
}

sol::table jsonToLuaTable(nlohmann::json json);

sol::table jsonToLuaArray(nlohmann::json json) {
	sol::table tmp;
	int index = 1;
	for (const auto& element : json) {
		if (element.is_null()) {
			tmp[index++] = sol::nil;
		}
		else if (element.is_boolean()) {
			tmp[index++] = element.get<bool>();
		}
		else if (element.is_number()) {
			tmp[index++] = element.get<double>();
		}
		else if (element.is_string()) {
			tmp[index++] = element.get<std::string>();
		}
		else if (element.is_object()) {
			tmp[index++] = jsonToLuaTable(element);
		}
		else if (element.is_array()) {
			tmp[index++] = jsonToLuaArray(element);
		}
	}

	return tmp;
}

sol::table jsonToLuaTable(nlohmann::json json) {
	sol::table result;

	if (json.is_array())
		return jsonToLuaArray(json);

	for (auto& pair : json.items()) {
		auto& key = pair.key();
		auto& value = pair.value();

		if (value.is_object()) {
			result[pair.key()] = jsonToLuaTable(value.get<nlohmann::json>());
		}
		else if (value.is_array()) {
			result[pair.key()] = jsonToLuaArray(value);
		}
		else if (value.is_null()) {
			result[key] = sol::nil;
		}
		else if (value.is_boolean()) {
			result[key] = value.get<bool>();
		}
		else if (value.is_number()) {
			result[key] = value.get<double>();
		}
		else if (value.is_string()) {
			result[key] = value.get<std::string>();
		}
	}

	return result;
}

std::string GetCurrentScript(sol::this_state s) {
	sol::state_view lua_state(s);
	sol::table rs = lua_state["debug"]["getinfo"](2, ("S"));
	std::string source = rs["source"];
	std::string filename = std::filesystem::path(source.substr(1)).stem().string();

	return filename;
}

void LuaErrorHandler(sol::optional<std::string> message) {
	if (!message)
		return;

	Console->ArcticTag();
	Console->ColorPrint(message.value_or("unknown"), Color(255, 0, 0));
	Console->Print("\n");
}

void ScriptLoadButton()
{
	Lua->LoadScript(Lua->GetScriptID(Config->lua_list->get_name()));
}

void ScriptUnloadButton()
{
	Lua->UnloadScript(Lua->GetScriptID(Config->lua_list->get_name()));
}

void ScriptSaveButton() {
	for (auto& script : Lua->scripts) {
		if (script.loaded)
			LuaSaveConfig(&script);
	}
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
			else if (event_name == "level_init")
				Lua->hooks.registerHook(LUA_LEVELINIT, script_id, func);
			else if (event_name == "aim_shot")
				Lua->hooks.registerHook(LUA_AIM_SHOT, script_id, func);
			else if (event_name == "aim_ack")
				Lua->hooks.registerHook(LUA_AIM_ACK, script_id, func);
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

		int delta_tick() {
			return ClientState->m_nDeltaTick;
		}

		int clock_offset() {
			return ClientState->m_ClockDriftMgr.m_iCurClockOffset;
		}
	}

	namespace common {
		std::string get_map_shortname() {
			return EngineClient->GetLevelNameShort();
		}
	}

	namespace files {
		std::string read(std::string path, sol::optional<bool> relative) {
			std::string file_path = relative.value_or(true) ? std::filesystem::current_path().string() + path : path;

			if (!std::filesystem::exists(file_path)) {
				Console->Error(std::format("files.read: file {} doesn't exists!", file_path));
				return "";
			}

			std::ifstream file(file_path);

			std::string result;
			file >> result;

			return result;
		}

		void write(std::string path, std::string data, sol::optional<bool> relative) {
			std::string file_path = relative.value_or(true) ? std::filesystem::current_path().string() + path : path;

			std::fstream file(file_path);

			file << data;
		}
	}

	namespace json {
		std::string stringify(sol::table table) {
			nlohmann::json json = luaTableToJson(table);

			return json.dump();
		}

		sol::table parse(std::string data) {
			nlohmann::json json = nlohmann::json::parse(data);

			return jsonToLuaTable(json);
		}
	} 

	namespace vector {
		Vector closes_ray_point(Vector self, Vector start, Vector end) {
			return EngineTrace->ClosestPoint(start, end, self);
		}

		float dist_to_ray(Vector self, Vector start, Vector end) {
			return EngineTrace->DistanceToRay(start, end, self);
		}

		Vector lerp(Vector self, Vector other, float weight) {
			return self + (other - self) * weight;
		}

		Vector to_screen(Vector self) {
			Vector2 scr = Render->WorldToScreen(self);
			return Vector(scr.x, scr.y);
		}
		
		std::string tostring(Vector self) {
			return std::format("vector({}, {}, {})", self.x, self.y, self.z);
		}
	}

	namespace render {
		Vector screen_size() {
			return Vector(Cheat.ScreenSize.x, Cheat.ScreenSize.y);
		}

		void add_font_from_memory(std::string mem) {
			Render->AddFontFromMemory(mem.data(), mem.size());
		}

		D3DXFont* load_font(std::string name, int size, std::string flags) {
			return Render->LoadFont(name, size, (flags.find("b") != std::string::npos) ? 600 : 400, (flags.find("a") != std::string::npos) ? CLEARTYPE_NATURAL_QUALITY : 0);
		}

		IDirect3DTexture9* load_image_from_memory(std::string mem, Vector size) {
			return Render->LoadImageFromMemory(mem.data(), mem.size(), size.to_vec2());
		}

		void line(Vector start, Vector end, Color col) {
			Render->Line(start.to_vec2(), end.to_vec2(), col);
		}

		void poly(Color col, sol::variadic_args vertecies) {
			std::vector<Vector2> verts;
			for (auto v : vertecies) {
				Vector vec = v;
				verts.push_back(Vector2(vec.x, vec.y));
			}
			Render->PolyFilled(verts, col);
		}

		void poly_line(Color col, sol::variadic_args vertecies) {
			std::vector<Vector2> verts;
			for (auto v : vertecies) {
				Vector vec = v;
				verts.push_back(Vector2(vec.x, vec.y));
			}
			Render->PolyLine(verts, col);
		}

		void rect(Vector start, Vector end, Color clr, sol::optional<int> rounding) {
			Render->BoxFilled(start.to_vec2(), end.to_vec2(), clr, rounding.value_or(0));
		}

		void rect_outline(Vector start, Vector end, Color clr, sol::optional<int> rounding) {
			Render->Box(start.to_vec2(), end.to_vec2(), clr, rounding.value_or(0));
		}

		void gradient(Vector start, Vector end, Color top_left, Color top_right, Color bottom_left, Color bottom_right) {
			Render->GradientBox(start.to_vec2(), end.to_vec2(), top_left, top_right, bottom_left, bottom_right);
		}

		void circle(Vector center, Color clr, float radius, sol::optional<float> start_deg, sol::optional<float> pct) {
			Render->CircleFilled(center.to_vec2(), radius, clr);
		}

		void circle_outline(Vector center, Color clr, float radius, sol::optional<float> start_deg, sol::optional<float> pct, sol::optional<int> thickness) {
			Render->Circle(center.to_vec2(), radius, clr, -1, start_deg.value_or(0.f), start_deg.value_or(0.f) + pct.value_or(1.f) * 360.f);
		}

		void circle_gradient(Vector center, Color color_outer, Color color_inner, float radius) {
			Render->GlowCircle2(center.to_vec2(), radius, color_inner, color_outer);
		}

		void circle_3d(Vector center, Color color, float radius) {
			Render->Circle3D(center, radius, color, true);
		}

		void circle_3d_outline(Vector center, Color color, float radius) {
			Render->Circle3D(center, radius, color, true);
		}

		void texture(IDirect3DTexture9* texture, Vector pos, std::optional<Color> col) {
			Render->Image(texture, pos.to_vec2(), col.value_or(Color(255, 255, 255)));
		}

		void text(sol::this_state state, sol::object font, Vector position, Color color, std::string flags, std::string text) {
			int flags_ = 0;
			for (char c : flags) {
				if (c == 'c')
					flags_ |= TEXT_CENTERED;
				else if (c == 'o')
					flags_ |= TEXT_OUTLINED;
				else if (c == 'd')
					flags_ |= TEXT_DROPSHADOW;
			}

			D3DXFont* font_ = nullptr;

			if (font.is<int>()) {
				switch (font.as<int>()) {
				case 1:
					font_ = Verdana;
					break;
				case 2:
					font_ = SmallFont;
					break;
				case 3:
					font_ = VerdanaBold;
					break;
				case 4:
					font_ = CalibriBold;
					break;
				default:
					Console->Error(std::format("[{}] unknown font: {}", GetCurrentScript(state), font.as<int>()));
					return;
				}
			}
			else {
				font_ = font.as<D3DXFont*>();

				if (!font_) {
					Console->Error(std::format("[{}] passed nil font", GetCurrentScript(state)));
					return;
				}
			}

			Render->Text(text, position.to_vec2(), color, font_, flags_);
		}

		Vector measure_text(sol::this_state state, sol::object font, std::string text) {
			D3DXFont* font_ = nullptr;

			if (font.is<int>()) {
				switch (font.as<int>()) {
				case 1:
					font_ = Verdana;
					break;
				case 2:
					font_ = SmallFont;
					break;
				case 3:
					font_ = VerdanaBold;
					break;
				case 4:
					font_ = CalibriBold;
					break;
				default:
					Console->Error(std::format("[{}] unknown font: {}", GetCurrentScript(state), font.as<int>()));
					return Vector();
				}
			}
			else {
				font_ = font.as<D3DXFont*>();

				if (!font_) {
					Console->Error(std::format("[{}] passed nil font", GetCurrentScript(state)));
					return Vector();
				}
			}

			Vector2 res = Render->CalcTextSize(text, font_);
			return Vector(res.x, res.y);
		}

		void push_clip_rect(Vector start, Vector end) {
			Render->PushClipRect(start.to_vec2(), end.to_vec2());
		}

		void pop_clip_rect() {
			Render->PopClipRect();
		}

		void set_antialias(bool state) {
			Render->SetAntiAliasing(state);
		}
	}

	namespace utils {
		void console_exec(std::string command) {
			EngineClient->ExecuteClientCmd(command.c_str());
		}

		void* create_interface(std::string module_name, std::string interface_name) {
			return Utils::CreateInterface(module_name.c_str(), interface_name.c_str());
		}

		void* pattern_scan(std::string module_name, std::string pattern, sol::optional<int> offset) {
			return Utils::PatternScan(module_name.c_str(), pattern.c_str(), offset.value_or(0));
		}

		CGameTrace trace_line(Vector start, Vector end, int mask, CBaseEntity* skip_entity) {
			return EngineTrace->TraceRay(start, end, mask, skip_entity);
		}

		CGameTrace trace_hull(Vector start, Vector end, Vector mins, Vector maxs, int mask, CBaseEntity* skip) {
			return EngineTrace->TraceHull(start, end, mins, maxs, mask, skip);
		}
	}

	namespace ui {
		CMenuGroupbox* find_groupbox(sol::this_state state, std::string tab, std::string groupbox) {
			CMenuGroupbox* found_item = Menu->FindGroupbox(tab, groupbox);

			if (!found_item) {
				Console->Error(std::format("[{}] cont find item: ({}, {})", GetCurrentScript(state), tab, groupbox));
				return nullptr;
			}

			return found_item;
		}

		IBaseWidget* find_item(sol::this_state state, std::string tab, std::string groupbox, std::string name, sol::optional<WidgetType> type) {
			WidgetType etype = type.value_or(WidgetType::Any);

			IBaseWidget* found_item = Menu->FindItem(tab, groupbox, name, etype);

			if (!found_item) {
				Console->Error(std::format("[{}] cont find item: ({}, {}, {})", GetCurrentScript(state), tab, groupbox, name));
				return nullptr;
			}

			return found_item;
		}

		sol::object element_get(sol::this_state state, IBaseWidget* element, sol::optional<int> index) {
			switch (element->GetType()) {
			case WidgetType::Checkbox:
				return sol::make_object(state, static_cast<CCheckBox*>(element)->get());
			case WidgetType::ColorPicker:
				return sol::make_object(state, static_cast<CColorPicker*>(element)->get());
			case WidgetType::KeyBind:
				return sol::make_object(state, static_cast<CKeyBind*>(element)->get());
			case WidgetType::SliderInt:
				return sol::make_object(state, static_cast<CSliderInt*>(element)->get());
			case WidgetType::SliderFloat:
				return sol::make_object(state, static_cast<CSliderFloat*>(element)->get());
			case WidgetType::Combo:
				return sol::make_object(state, static_cast<CComboBox*>(element)->get());
			case WidgetType::MultiCombo:
				return sol::make_object(state, static_cast<CMultiCombo*>(element)->get(index.value()));
			case WidgetType::Input:
				return sol::make_object(state, static_cast<CInputBox*>(element)->get());
			}

			Console->Error("trying to get unknown element");
		}

		void element_update_list(sol::this_state state, IBaseWidget* element, std::vector<const char*> list) {
			switch (element->GetType()) {
			case WidgetType::Combo:
				return static_cast<CComboBox*>(element)->UpdateList(list);
			case WidgetType::MultiCombo:
				return static_cast<CMultiCombo*>(element)->UpdateList(list);
			}

			Console->Error("trying to update list of non listable element");
		}

		void element_set_callback(sol::this_state state, IBaseWidget* element, sol::protected_function func) {
			UILuaCallback_t cb(element, Lua->GetScriptID(GetCurrentScript(state)), func);
			element->lua_callbacks.push_back(cb);
			g_ui_lua_callbacks.push_back(cb); // track callback to easily remove them
		}
		 
		void element_set_visible(IBaseWidget* element, bool visible) {
			element->SetVisible(visible);
		}

		CMenuTab* tab(sol::this_state state, std::string name, sol::optional<IDirect3DTexture9*> icon, sol::optional<Vector> size) {
			Vector s = size.value_or(Vector(16, 16));
			ImVec2 im_size(s.x, s.y);
			
			CMenuTab* tab = Menu->AddTab(name, icon.value_or(pic::tab::scripts), im_size);
			
			LuaScript_t* script = &Lua->scripts[Lua->GetScriptID(GetCurrentScript(state))];
			script->tabs.push_back(tab);

			return tab;
		}

		CMenuGroupbox* groupbox(sol::this_state state, std::string tab, std::string groupbox) {
			CMenuGroupbox* gb = Menu->AddGroupBox(tab, groupbox);

			LuaScript_t* script = &Lua->scripts[Lua->GetScriptID(GetCurrentScript(state))];
			script->groupboxes.push_back(gb);

			return gb;
		}

		IBaseWidget* checkbox(sol::this_state state, CMenuGroupbox* self, std::string name, sol::optional<bool> def) {
			IBaseWidget* elem = self->AddCheckBox(name, def.value_or(false));

			LuaScript_t* script = &Lua->scripts[Lua->GetScriptID(GetCurrentScript(state))];
			script->ui_elements.push_back(elem);

			return elem;
		}

		IBaseWidget* label(sol::this_state state, CMenuGroupbox* self, std::string name) {
			IBaseWidget* elem = self->AddLabel(name);

			LuaScript_t* script = &Lua->scripts[Lua->GetScriptID(GetCurrentScript(state))];
			script->ui_elements.push_back(elem);

			return elem;
		}

		IBaseWidget* colorpicker(sol::this_state state, CMenuGroupbox* self, std::string name, sol::optional<Color> default_color) {
			IBaseWidget* elem = self->AddColorPicker(name, default_color.value_or(Color()));

			LuaScript_t* script = &Lua->scripts[Lua->GetScriptID(GetCurrentScript(state))];
			script->ui_elements.push_back(elem);

			return elem;
		}

		IBaseWidget* keybind(sol::this_state state, CMenuGroupbox* self, std::string name) {
			IBaseWidget* elem = self->AddKeyBind(name);

			LuaScript_t* script = &Lua->scripts[Lua->GetScriptID(GetCurrentScript(state))];
			script->ui_elements.push_back(elem);

			return elem;
		}

		IBaseWidget* slider_int(sol::this_state state, CMenuGroupbox* self, std::string name, int min_, int max_, int default_val, sol::optional<std::string> format) {
			IBaseWidget* elem = self->AddSliderInt(name, min_, max_, default_val, format.value_or("%d"));

			LuaScript_t* script = &Lua->scripts[Lua->GetScriptID(GetCurrentScript(state))];
			script->ui_elements.push_back(elem);

			return elem;
		}

		IBaseWidget* slider_float(sol::this_state state, CMenuGroupbox* self, std::string name, float min_, float max_, float default_val, sol::optional<std::string> format) {
			IBaseWidget* elem = self->AddSliderFloat(name, min_, max_, default_val, format.value_or("%.2f"));

			LuaScript_t* script = &Lua->scripts[Lua->GetScriptID(GetCurrentScript(state))];
			script->ui_elements.push_back(elem);

			return elem;
		}

		IBaseWidget* combo(sol::this_state state, CMenuGroupbox* self, std::string name, sol::variadic_args elements) {
			std::vector<const char*> vals;
			for (auto el : elements) {
				std::string s = el;
				vals.push_back(s.c_str());
			}
			IBaseWidget* elem = self->AddComboBox(name, vals);

			LuaScript_t* script = &Lua->scripts[Lua->GetScriptID(GetCurrentScript(state))];
			script->ui_elements.push_back(elem);

			return elem;
		}

		IBaseWidget* multicombo(sol::this_state state, CMenuGroupbox* self, std::string name, sol::variadic_args elements) {
			std::vector<const char*> vals;
			for (auto el : elements) {
				std::string s = el;
				vals.push_back(s.c_str());
			}
			IBaseWidget* elem = self->AddMultiCombo(name, vals);

			LuaScript_t* script = &Lua->scripts[Lua->GetScriptID(GetCurrentScript(state))];
			script->ui_elements.push_back(elem);

			return elem;
		}
	
		IBaseWidget* button(sol::this_state state, CMenuGroupbox* self, std::string name) {
			IBaseWidget* elem = self->AddButton(name);

			LuaScript_t* script = &Lua->scripts[Lua->GetScriptID(GetCurrentScript(state))];
			script->ui_elements.push_back(elem);

			return elem;
		}

		IBaseWidget* input(sol::this_state state, CMenuGroupbox* self, std::string name) {
			IBaseWidget* elem = self->AddInput(name);

			LuaScript_t* script = &Lua->scripts[Lua->GetScriptID(GetCurrentScript(state))];
			script->ui_elements.push_back(elem);

			return elem;
		}
	}

	namespace entity {
		CBasePlayer* get_local_player() {
			return reinterpret_cast<CBasePlayer*>(EntityList->GetClientEntity(EngineClient->GetLocalPlayer()));
		}

		sol::object get_prop(sol::this_state state, CBaseEntity* ent, std::string prop_name) {
			auto prop = NetVars::FindProp(ent->GetClientClass()->m_pRecvTable, prop_name);

			if (prop.offset == 0)
				return sol::nil;

			switch (prop.prop->m_RecvType)
			{
			case DPT_Int:
				return sol::make_object(state, *reinterpret_cast<int*>((uintptr_t)ent + prop.offset));
			case DPT_Float:
				return sol::make_object(state, *reinterpret_cast<float*>((uintptr_t)ent + prop.offset));
			case DPT_Vector:
			case DPT_VectorXY:
				return sol::make_object(state, *reinterpret_cast<Vector*>((uintptr_t)ent + prop.offset));
			case DPT_String:
				return sol::make_object(state, reinterpret_cast<char*>((uintptr_t)ent + prop.offset));
			case DPT_Array: {
				auto array_prop = prop.prop->m_pArrayProp[0];

				switch (array_prop.m_RecvType)
				{
				case DPT_Int:
					return sol::make_object(state, reinterpret_cast<int*>((uintptr_t)ent + prop.offset));
				case DPT_Float:
					return sol::make_object(state, reinterpret_cast<float*>((uintptr_t)ent + prop.offset));
				case DPT_Vector:
				case DPT_VectorXY:
					return sol::make_object(state, reinterpret_cast<Vector*>((uintptr_t)ent + prop.offset));
				case DPT_String:
					return sol::make_object(state, reinterpret_cast<char**>((uintptr_t)ent + prop.offset));
				}
			}
			default:
				break;
			}

			return sol::nil;
		}

		Vector obb_maxs(sol::this_state state, CBaseEntity* ent) {
			auto collidable = ent->GetCollideable();

			if (!collidable)
				return Vector();

			return collidable->OBBMaxs();
		}

		Vector obb_mins(sol::this_state state, CBaseEntity* ent) {
			auto collidable = ent->GetCollideable();

			if (!collidable)
				return Vector();

			return collidable->OBBMins();
		}

		Vector collision_origin(sol::this_state state, CBaseEntity* ent) {
			auto collidable = ent->GetCollideable();

			if (!collidable)
				return Vector();

			return collidable->GetCollisionOrigin();
		}
	}

	namespace network {
		std::string get(sol::this_state state, std::string url, sol::optional<sol::table> headers, sol::optional<sol::protected_function> callback) {
			if (callback.has_value()) {
				// will implement async request later
			}
			else {
				std::vector<HttpHeader> _headers;
				
				if (headers.has_value()) {
					auto val = headers.value();

					for (auto& header : val) {
						_headers.push_back(HttpHeader{ header.first.as<std::string>(), header.second.as<std::string>() });
					}
				}

				return Requests->Create(k_EHTTPMethodGET, url, _headers);
			}
		}
	}
}

void CLua::Setup() {
	std::filesystem::create_directory(std::filesystem::current_path().string() + "/at/scripts");
	std::filesystem::create_directory(std::filesystem::current_path().string() + "/at/scripts/cfg");

	lua = sol::state(sol::c_call<decltype(&LuaErrorHandler), &LuaErrorHandler>);
	lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::math, sol::lib::table, sol::lib::debug, sol::lib::package, sol::lib::bit32, sol::lib::ffi, sol::lib::jit, sol::lib::io, sol::lib::utf8);

	// enums
	lua.new_enum<EDamageGroup>("e_dmg_group", {
		{"head", EDamageGroup::DAMAGEGROUP_HEAD},
		{"chest", EDamageGroup::DAMAGEGROUP_CHEST},
		{"stomach", EDamageGroup::DAMAGEGROUP_STOMACH},
		{"arms", EDamageGroup::DAMAGEGROUP_ARM},
		{"legs", EDamageGroup::DAMAGEGROUP_LEG},
	});

	lua.new_enum<WidgetType>("e_ui_type", {
		{"checkbox", WidgetType::Checkbox},
		{"label", WidgetType::Label},
		{"color_picker", WidgetType::ColorPicker},
		{"keybind", WidgetType::KeyBind},
		{"sliderint", WidgetType::SliderInt},
		{"sliderfloat", WidgetType::SliderFloat},
		{"combo", WidgetType::Combo},
		{"multicombo", WidgetType::MultiCombo},
		{"button", WidgetType::Button},
		{"input", WidgetType::Input},
		{"any", WidgetType::Any}
	});

	// usertypes
	lua.new_usertype<IBaseWidget>("ui_element_t", sol::no_constructor, 
		"set_callback", api::ui::element_set_callback,
		"set_visible", api::ui::element_set_visible,
		"get", api::ui::element_get,
		"update_list", api::ui::element_update_list
	);

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

	lua.new_usertype<Vector>("vector", sol::call_constructor, sol::constructors<Vector(), Vector(float, float), Vector(float, float, float)>(),
		"x", &Vector::x,
		"y", &Vector::y,
		"z", &Vector::z,
		"__add", &Vector::operator+,
		"__sub", &Vector::operator-,
		"__mul", &Vector::operator*,
		"__div", &Vector::operator/,
		"__eq", &Vector::operator==,
		"__len", &Vector::Q_Length,
		"__tostring", &api::vector::tostring,
		"length", &Vector::Q_Length,
		"length_sqr", &Vector::LengthSqr,
		"length2d", &Vector::Q_Length2D,
		"length2d_sqr", &Vector::Length2DSqr,
		"closest_ray_point", &api::vector::closes_ray_point,
		"dist_to_ray", &api::vector::dist_to_ray,
		"dot", &Vector::Dot,
		"cross", &Vector::Cross,
		"dist", &Vector::DistTo,
		"lerp", &api::vector::lerp,
		"to_screen", &api::vector::to_screen
	);

	lua.new_usertype<CBaseEntity>("entity_t", sol::no_constructor,
		"ent_index", &CBaseEntity::EntIndex,
		"obb_maxs", api::entity::obb_maxs,
		"obb_mins", api::entity::obb_mins,
		"collision_origin", api::entity::collision_origin,
		"__index", api::entity::get_prop
	);

	lua.new_usertype<CBaseCombatWeapon>("weapon_t", sol::no_constructor,
		"ent_index", &CBaseCombatWeapon::EntIndex,
		"weapon_index", &CBaseCombatWeapon::m_iItemDefinitionIndex,
		"get_inaccuracy", &CBaseCombatWeapon::GetInaccuracy,
		"get_spread", &CBaseCombatWeapon::GetSpread,
		"obb_maxs", api::entity::obb_maxs,
		"obb_mins", api::entity::obb_mins,
		"collision_origin", api::entity::collision_origin,
		"__index", api::entity::get_prop
	);

	lua.new_usertype<CBasePlayer>("player_t", sol::no_constructor, 
		"ent_index", &CBasePlayer::EntIndex,
		"get_name", &CBasePlayer::GetName,
		"get_active_weapon", &CBasePlayer::GetActiveWeapon,
		"get_abs_origin", &CBasePlayer::GetAbsOrigin,
		"get_abs_angles", &CBasePlayer::GetAbsAngles,
		"obb_maxs", api::entity::obb_maxs,
		"obb_mins", api::entity::obb_mins,
		"collision_origin", api::entity::collision_origin,
		"__index", api::entity::get_prop
	);

	lua.new_usertype<LagRecord>("lag_record_t", sol::no_constructor,
		"player", &LagRecord::player,
		"origin", &LagRecord::m_vecOrigin,
		"velocity", &LagRecord::m_vecVelocity,
		"mins", &LagRecord::m_vecMins,
		"maxs", &LagRecord::m_vecMaxs,
		"abs_angles", &LagRecord::m_vecAbsAngles,
		"view_angles", &LagRecord::m_viewAngle,
		"simulation_time", &LagRecord::m_flSimulationTime,
		"duck_amount", &LagRecord::m_flDuckAmout,
		"duck_speed", &LagRecord::m_flDuckSpeed,
		"choked_ticks", &LagRecord::m_nChokedTicks,
		"shifting_tickbase", &LagRecord::shifting_tickbase,
		"breaking_lag_compensation", &LagRecord::breaking_lag_comp
	);

	lua.new_usertype<QAngle>("qangle", sol::call_constructor, sol::constructors<QAngle(), QAngle(float, float), QAngle(float, float, float)>(), 
		"pitch", &QAngle::pitch,
		"yaw", &QAngle::yaw,
		"roll", &QAngle::roll
	);

	lua.new_usertype<RegisteredShot_t>("shot_t", sol::no_constructor, 
		"client_shoot_pos", &RegisteredShot_t::client_shoot_pos,
		"vector_pos", &RegisteredShot_t::target_pos,
		"client_angle", &RegisteredShot_t::client_angle,
		"shot_tick", &RegisteredShot_t::shot_tick,
		"wanted_damage", &RegisteredShot_t::wanted_damage,
		"wanted_damagegroup", &RegisteredShot_t::wanted_damagegroup,
		"hitchance", &RegisteredShot_t::hitchance,
		"backtrack", &RegisteredShot_t::backtrack,
		"record", &RegisteredShot_t::record,
		"shoot_pos", &RegisteredShot_t::shoot_pos,
		"end_pos", &RegisteredShot_t::end_pos,
		"angle", &RegisteredShot_t::angle,
		"ack_tick", &RegisteredShot_t::ack_tick,
		"impacts", &RegisteredShot_t::impacts,
		"damage", &RegisteredShot_t::damage,
		"damagegroup", &RegisteredShot_t::damagegroup,
		"hit_point", &RegisteredShot_t::hit_point,
		"acked", &RegisteredShot_t::acked,
		"miss_reason", &RegisteredShot_t::miss_reason
	);

	lua.new_usertype<CMenuGroupbox>("groupbox_t", sol::no_constructor,
		"checkbox", api::ui::checkbox,
		"label", api::ui::label,
		"color_picker", api::ui::colorpicker,
		"keybind", api::ui::keybind,
		"slider_int", api::ui::slider_int,
		"slider_float", api::ui::slider_float,
		"combo", api::ui::combo,
		"multicombo", api::ui::multicombo,
		"input", api::ui::input,
		"button", api::ui::button
	);

	lua.new_usertype<CUserCmd_lua>("user_cmd_t", sol::no_constructor,
		"command_number", &CUserCmd_lua::command_number,
		"tickcount", &CUserCmd_lua::tickcount,
		"move", &CUserCmd_lua::move,
		"viewangles", &CUserCmd_lua::viewangles,
		"buttons", &CUserCmd_lua::buttons,
		"random_seed", &CUserCmd_lua::random_seed,
		"hasbeenpredicted", &CUserCmd_lua::hasbeenpredicted,
		"override_defensive", &CUserCmd_lua::override_defensive,
		"allow_defensive", &CUserCmd_lua::allow_defensive
	);

	lua.new_usertype<CGameTrace>("trace_t", sol::no_constructor,
		"startpos", &CGameTrace::startpos,
		"endpos", &CGameTrace::endpos,
		"fraction", &CGameTrace::fraction,
		"allsolid", &CGameTrace::allsolid,
		"startsolid", &CGameTrace::startsolid,
		"hit_entity", &CGameTrace::hit_entity,
		"hitgroup", &CGameTrace::hitgroup
	);

	// _G
	lua["print"] = api::print;
	lua["error"] = api::error;
	lua["print_raw"] = api::print_raw;

	// client
	lua.create_named_table("client",
		"add_callback", api::client::add_callback,
		"unload_script", api::client::unload_script,
		"reload_script", api::client::reload_script
	);

	// entity
	lua.create_named_table("entity",
		"get_local_player", api::entity::get_local_player
	);

	// ui
	lua.create_named_table("ui",
		"tab", api::ui::tab,
		"groupbox", api::ui::groupbox,
		"find_groupbox", api::ui::find_groupbox,
		"find_item", api::ui::find_item
	);

	// global vars
	lua.new_usertype<CGlobalVarsBase>("global_vars_t", sol::no_constructor,
		"curtime", sol::readonly(&CGlobalVarsBase::curtime),
		"realtime", sol::readonly(&CGlobalVarsBase::realtime),
		"frametime", sol::readonly(&CGlobalVarsBase::frametime),
		"framecount", sol::readonly(&CGlobalVarsBase::framecount),
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
		"client_tick", sol::readonly_property(&api::globals::client_tick),
		"delta_tick", sol::readonly_property(&api::globals::delta_tick),
		"clock_offset", sol::readonly_property(&api::globals::clock_offset)
	);
	lua["globals"] = GlobalVars;

	// render
	lua.create_named_table("render",
		"screen_size", api::render::screen_size,
		"add_font_from_memory", api::render::add_font_from_memory,
		"load_font", api::render::load_font,
		"load_image_from_memory", api::render::load_image_from_memory,
		"measure_text", api::render::measure_text,
		"line", api::render::line,
		"poly", api::render::poly,
		"poly_line", api::render::poly_line,
		"rect", api::render::rect,
		"rect_outline", api::render::rect_outline,
		"gradient", api::render::gradient,
		"circle", api::render::circle,
		"circle_outline", api::render::circle_outline,
		"circle_3d", api::render::circle_3d,
		"circle_3d_outline", api::render::circle_3d_outline,
		"circle_gradient", api::render::circle_gradient,
		"texture", api::render::texture,
		"text", api::render::text,
		"push_clip_rect", api::render::push_clip_rect,
		"pop_clip_rect", api::render::pop_clip_rect,
		"world_to_screen", api::vector::to_screen,
		"set_antialias", api::render::set_antialias
	);

	lua.create_named_table("common",
		"get_map_shortname", api::common::get_map_shortname
	);

	lua.create_named_table("files",
		"read", api::files::read,
		"write", api::files::write
	);

	lua.create_named_table("json", 
		"stringify", api::json::stringify,
		"parse", api::json::parse
	);

	// utils
	lua.create_named_table("utils",
		"console_exec", api::utils::console_exec,
		"create_interface", api::utils::create_interface,
		"pattern_scan", api::utils::pattern_scan,
		"trace_line", api::utils::trace_line,
		"trace_hull", api::utils::trace_hull
	);

	// network
	lua.create_named_table("network",
		"get", api::network::get
	);

	RefreshScripts();
	Config->lua_button->SetCallback(ScriptLoadButton);
	Config->lua_button_unload->SetCallback(ScriptUnloadButton);
	Config->lua_save->SetCallback(ScriptSaveButton);
	Config->lua_refresh->SetCallback([]() { Lua->RefreshScripts(); });
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

	LuaLoadConfig(&script);

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

	for (auto element : script.ui_elements) {
		Menu->RemoveItem(element);
	}

	for (auto gb : script.groupboxes) {
		Menu->RemoveGroupBox(gb);
	}

	for (auto tab : script.tabs) {
		Menu->RemoveTab(tab);
	}

	script.ui_elements.clear();
	script.groupboxes.clear();
	script.tabs.clear();

	for (auto cb = g_ui_lua_callbacks.begin(); cb != g_ui_lua_callbacks.end();) {
		if (cb->script_id == id) {
			for (auto it = cb->ref->lua_callbacks.begin(); it != cb->ref->lua_callbacks.end();) {
				if (it->script_id == id) {
					it = cb->ref->lua_callbacks.erase(it);
					continue;
				}

				it++;
			}

			cb = g_ui_lua_callbacks.erase(cb);
			continue;
		}

		cb++;
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

std::vector<const char*> CLua::GetUIList() {
	std::vector<const char*> result;

	for (auto& script : scripts) {
		result.push_back(script.ui_name.c_str());
	}

	return result;
}

void CLua::RefreshScripts() {
	auto old_scripts = scripts;

	ScriptSaveButton();
	UnloadAll();
	scripts.clear();

	for (auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path().string() + "/at/scripts"))
	{
		if (entry.path().extension() == ".lua")
		{
			LuaScript_t script;

			script.path = entry.path();
			script.name = script.path.stem().string();
			script.loaded = false;

			bool was_loaded = false;

			for (auto& o_script : old_scripts) {
				if (o_script.name == script.name && o_script.loaded) {
					was_loaded = true;
					break;
				}
			}
			script.ui_name = was_loaded ? "* " + script.name : script.name;

			scripts.push_back(script);
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