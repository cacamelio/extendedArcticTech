#include "Bridge.h"

#include "../../ShotManager/ShotManager.h"
#include "../../RageBot/LagCompensation.h"


std::vector<UILuaCallback_t> g_ui_lua_callbacks;

void LuaSaveConfig(LuaScript_t* script) {
	nlohmann::json result;

	for (auto e : script->ui_elements) {\
		std::string name = e->parent->name + "_" + e->name + "_" + std::to_string(e->GetItemType());

		switch (e->GetItemType()) {
		case CHECKBOX:
			result[name] = ((CCheckbox*)e)->value;
			break;
		case COLORPCIKER:
			result[name] = ((CColorPicker*)e)->get().to_int32();
			break;
		case KEYBIND:
			result[name] = { ((CKeyBind*)e)->bindType, ((CKeyBind*)e)->bindState, ((CKeyBind*)e)->key };
			break;
		case SLIDER:
			result[name] = ((CSlider*)e)->value;
			break;
		case COMBO:
			result[name] = ((CComboBox*)e)->value;
			break;
		case MULTICOMBO:
			result[name] = ((CMultiCombo*)e)->values;
			break;
		case INPUTBOX:
			result[name] = ((CInputBox*)e)->input;
			break;
		case LISTBOX:
			result[name] = ((CListBox*)e)->active;
			break;
		default:
			// handle any unexpected values of ElementType here
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
			auto& val = config_json[e->parent->name + "_" + e->name + "_" + std::to_string(e->GetItemType())];

			switch (e->GetItemType()) {
			case CHECKBOX:
				((CCheckbox*)e)->value = val;
				break;
			case COLORPCIKER:
				((CColorPicker*)e)->Set(val);
				break;
			case KEYBIND:
				((CKeyBind*)e)->bindType = val[0];
				((CKeyBind*)e)->bindState = val[1];
				((CKeyBind*)e)->key = val[2];
				break;
			case SLIDER:
				((CSlider*)e)->value = val;
				break;
			case COMBO:
				((CComboBox*)e)->value = val;
				break;
			case MULTICOMBO:
				((CMultiCombo*)e)->values = val;
				break;
			case INPUTBOX:
				((CInputBox*)e)->input = val;
				break;
			case LISTBOX:
				((CListBox*)e)->active = val;
				break;
			default:
				break;
			}

			if (e->callback)
				e->callback();

			for (auto cb : e->lua_callbacks)
				cb.func();
		}
		catch (nlohmann::json::exception& ex) {
			Console->Error("couldnt find cfg item: " + e->name);
		}
	}
}

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

void ScriptSaveButton() {
	for (auto& script : Lua->scripts) {
		if (script.loaded)
			LuaSaveConfig(&script);
	}
}

std::string GetCurrentScript(sol::this_state s) {
	sol::state_view lua_state(s);
	sol::table rs = lua_state["debug"]["getinfo"](2, ("S"));
	std::string source = rs["source"];
	std::string filename = std::filesystem::path(source.substr(1)).stem().string();

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
				verts.emplace_back(Vector2(vec.x, vec.y));
			}
			Render->PolyFilled(verts, col);
		}

		void poly_line(Color col, sol::variadic_args vertecies) {
			std::vector<Vector2> verts;
			for (auto v : vertecies) {
				Vector vec = v;
				verts.emplace_back(Vector2(vec.x, vec.y));
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
	}

	namespace ui {
		IBaseElement* find_item(sol::this_state state, std::string tab, std::string groupbox, std::string name, sol::optional<ElementType> type) {
			ElementType etype = type.value_or(ElementType::ANY);

			IBaseElement* found_item = Menu->FindElement(tab, groupbox, name, etype);

			if (!found_item) {
				Console->Error(std::format("[{}] cont find item: ({}, {}, {})", GetCurrentScript(state), tab, groupbox, name));
				return nullptr;
			}

			return found_item;
		}

		sol::object element_get(sol::this_state state, IBaseElement* element, sol::optional<int> index) {
			switch (element->GetItemType()) {
			case CHECKBOX:
				return sol::make_object(state, static_cast<CCheckbox*>(element)->get());
			case COLORPCIKER:
				return sol::make_object(state, static_cast<CColorPicker*>(element)->get());
			case KEYBIND:
				return sol::make_object(state, static_cast<CKeyBind*>(element)->get());
			case SLIDER:
				return sol::make_object(state, static_cast<CSlider*>(element)->get());
			case COMBO:
				return sol::make_object(state, static_cast<CComboBox*>(element)->get());
			case MULTICOMBO:
				return sol::make_object(state, static_cast<CMultiCombo*>(element)->get(index.value()));
			case INPUTBOX:
				return sol::make_object(state, static_cast<CInputBox*>(element)->get());
			case LISTBOX:
				return sol::make_object(state, static_cast<CListBox*>(element)->get());
			}

			Console->Error("trying to get unknown element");
		}

		void element_update_list(sol::this_state state, IBaseElement* element, std::vector<std::string> list) {
			switch (element->GetItemType()) {
			case COMBO:
				return static_cast<CComboBox*>(element)->UpdateList(list);
			case MULTICOMBO:
				return static_cast<CMultiCombo*>(element)->UpdateList(list);
			case LISTBOX:
				return static_cast<CListBox*>(element)->UpdateList(list);
			}

			Console->Error("trying to update list of non listable element");
		}

		void element_set_callback(sol::this_state state, IBaseElement* element, sol::protected_function func) {
			UILuaCallback_t cb(element, Lua->GetScriptID(GetCurrentScript(state)), func);
			element->lua_callbacks.emplace_back(cb);
			g_ui_lua_callbacks.emplace_back(cb); // track callback to easily remove them
		}

		void element_set_visible(IBaseElement* element, bool visible) {
			element->set_visible(visible);
		}

		IBaseElement* new_checkbox(sol::this_state state, std::string tab, std::string groupbox, std::string name, sol::optional<bool> unsafe) {
			IBaseElement* elem = Menu->AddCheckBox(tab, groupbox, name, unsafe.value_or(false));

			LuaScript_t* script = &Lua->scripts[Lua->GetScriptID(GetCurrentScript(state))];
			script->ui_elements.emplace_back(elem);

			return elem;
		}

		IBaseElement* new_label(sol::this_state state, std::string tab, std::string groupbox, std::string name) {
			IBaseElement* elem = Menu->AddLabel(tab, groupbox, name);

			LuaScript_t* script = &Lua->scripts[Lua->GetScriptID(GetCurrentScript(state))];
			script->ui_elements.emplace_back(elem);

			return elem;
		}

		IBaseElement* new_colorpicker(sol::this_state state, std::string tab, std::string groupbox, std::string name, sol::optional<Color> default_color) {
			IBaseElement* elem = Menu->AddColorPicker(tab, groupbox, name, default_color.value_or(Color()));

			LuaScript_t* script = &Lua->scripts[Lua->GetScriptID(GetCurrentScript(state))];
			script->ui_elements.emplace_back(elem);

			return elem;
		}

		IBaseElement* new_keybind(sol::this_state state, std::string tab, std::string groupbox, std::string name, sol::optional<int> default_key, sol::optional<int> default_type) {
			IBaseElement* elem = Menu->AddKeyBind(tab, groupbox, name, default_key.value_or(-1), default_type.value_or(1));

			LuaScript_t* script = &Lua->scripts[Lua->GetScriptID(GetCurrentScript(state))];
			script->ui_elements.emplace_back(elem);

			return elem;
		}

		IBaseElement* new_slider(sol::this_state state, std::string tab, std::string groupbox, std::string name, float min_, float max_, float default_val, sol::optional<std::string> unit, sol::optional<float> scale, sol::optional<bool> hide_name) {
			IBaseElement* elem = Menu->AddSlider(tab, groupbox, name, min_, max_, default_val, unit.value_or(""), scale.value_or(1.f), hide_name.value_or(false));

			LuaScript_t* script = &Lua->scripts[Lua->GetScriptID(GetCurrentScript(state))];
			script->ui_elements.emplace_back(elem);

			return elem;
		}

		IBaseElement* new_combo(sol::this_state state, std::string tab, std::string groupbox, std::string name, sol::optional<int> default_value, sol::optional<bool> hide_name, sol::variadic_args elements) {
			std::vector<std::string> vals;
			for (auto el : elements) {
				std::string s = el;
				vals.emplace_back(s);
			}
			IBaseElement* elem = Menu->AddComboBox(tab, groupbox, name, vals, default_value.value_or(0), hide_name.value_or(false));

			LuaScript_t* script = &Lua->scripts[Lua->GetScriptID(GetCurrentScript(state))];
			script->ui_elements.emplace_back(elem);

			return elem;
		}

		IBaseElement* new_multicombo(sol::this_state state, std::string tab, std::string groupbox, std::string name, sol::optional<bool> hide_name, sol::variadic_args elements) {
			std::vector<std::string> vals;
			for (auto el : elements) {
				std::string s = el;
				vals.emplace_back(s);
			}

			IBaseElement* elem = Menu->AddMultiCombo(tab, groupbox, name, vals, 0, hide_name.value_or(false));

			LuaScript_t* script = &Lua->scripts[Lua->GetScriptID(GetCurrentScript(state))];
			script->ui_elements.emplace_back(elem);

			return elem;
		}
	
		IBaseElement* new_button(sol::this_state state, std::string tab, std::string groupbox, std::string name) {
			IBaseElement* elem = Menu->AddButton(tab, groupbox, name);

			LuaScript_t* script = &Lua->scripts[Lua->GetScriptID(GetCurrentScript(state))];
			script->ui_elements.emplace_back(elem);

			return elem;
		}

		IBaseElement* new_input(sol::this_state state, std::string tab, std::string groupbox, std::string name, sol::optional<bool> hide_name) {
			IBaseElement* elem = Menu->AddInputBox(tab, groupbox, name, hide_name.value_or(false));

			LuaScript_t* script = &Lua->scripts[Lua->GetScriptID(GetCurrentScript(state))];
			script->ui_elements.emplace_back(elem);

			return elem;
		}

		IBaseElement* new_list(sol::this_state state, std::string tab, std::string groupbox, std::string name, sol::optional<bool> has_filter, sol::variadic_args elements) {
			std::vector<std::string> vals;
			for (auto el : elements) {
				std::string s = el;
				vals.emplace_back(s);
			}

			IBaseElement* elem = Menu->AddListBox(tab, groupbox, name, vals, has_filter.value_or(true));

			LuaScript_t* script = &Lua->scripts[Lua->GetScriptID(GetCurrentScript(state))];
			script->ui_elements.emplace_back(elem);

			return elem;
		}
	}

	namespace entity {
		CBasePlayer* get_local_player() {
			return Cheat.LocalPlayer;
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

	lua.new_enum<ElementType>("e_ui_type", {
		{"checkbox", ElementType::CHECKBOX},
		{"label", ElementType::LABEL},
		{"colorpicker", ElementType::COLORPCIKER},
		{"keybind", ElementType::KEYBIND},
		{"slider", ElementType::SLIDER},
		{"combo", ElementType::COMBO},
		{"multicombo", ElementType::MULTICOMBO},
		{"button", ElementType::BUTTON},
		{"input", ElementType::INPUTBOX},
		{"list", ElementType::LISTBOX},
		{"any", ElementType::ANY}
	});

	// usertypes
	/*
	lua.new_usertype<CCheckbox>("checkbox_t", sol::no_constructor,
		"get", &CCheckbox::get,
		"set_visible", &CCheckbox::set_visible,
		"set_callback", &CCheckbox::set_callback
	);

	lua.new_usertype<CColorPicker>("colorpicker_t", sol::no_constructor,
		"get", &CColorPicker::get,
		"set_visible", &CColorPicker::set_visible,
		"set_callback", &CColorPicker::set_callback
	);

	lua.new_usertype<CKeyBind>("keybind_t", sol::no_constructor,
		"get", &CKeyBind::get,
		"set_visible", &CKeyBind::set_visible,
		"set_callback", &CKeyBind::set_callback
	);

	lua.new_usertype<CSlider>("slider_t", sol::no_constructor,
		"get", &CSlider::get,
		"set_visible", &CSlider::set_visible,
		"set_callback", &CSlider::set_callback
	);	
	
	lua.new_usertype<CComboBox>("combobox_t", sol::no_constructor,
		"get", &CComboBox::get,
		"set_visible", &CComboBox::set_visible,
		"set_callback", &CComboBox::set_callback
	);

	lua.new_usertype<CMultiCombo>("multicombobox_t", sol::no_constructor,
		"get", &CMultiCombo::get,
		"set_visible", &CMultiCombo::set_visible,
		"set_callback", &CMultiCombo::set_callback
	);

	lua.new_usertype<CButton>("button_t", sol::no_constructor,
		"set_visible", &CButton::set_visible,
		"set_callback", &CButton::set_callback
	);

	lua.new_usertype<CInputBox>("input_t", sol::no_constructor,
		"get", &CInputBox::get,
		"set_visible", &CInputBox::set_visible,
		"set_callback", &CInputBox::set_callback
	);

	lua.new_usertype<CListBox>("listbox_t", sol::no_constructor,
		"get", &CListBox::get,
		"set_visible", &CListBox::set_visible,
		"set_callback", &CListBox::set_callback,
		"update_list", &CListBox::UpdateList
	);
	*/
	lua.new_usertype<IBaseElement>("ui_element_t", sol::no_constructor, 
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

	lua.new_usertype<CBaseCombatWeapon>("weapon_t", sol::no_constructor,
		"ent_index", &CBaseCombatWeapon::EntIndex,
		"get_inaccuracy", &CBaseCombatWeapon::GetInaccuracy,
		"get_spread", &CBaseCombatWeapon::GetSpread
	);

	lua.new_usertype<CBasePlayer>("player_t", sol::no_constructor, 
		"ent_index", &CBasePlayer::EntIndex,
		"get_name", &CBasePlayer::GetName,
		"get_active_weapon", &CBasePlayer::GetActiveWeapon,
		"get_abs_orgin", &CBasePlayer::GetAbsOrigin,
		"get_abs_angles", &CBasePlayer::GetAbsAngles
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
		"find", api::ui::find_item,
		"new_checkbox", api::ui::new_checkbox,
		"new_label", api::ui::new_label,
		"new_colorpicker", api::ui::new_colorpicker,
		"new_keybind", api::ui::new_keybind,
		"new_slider", api::ui::new_slider,
		"new_combo", api::ui::new_combo,
		"new_multicombo", api::ui::new_multicombo,
		"new_button", api::ui::new_button,
		"new_input", api::ui::new_input,
		"new_list", api::ui::new_list
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
		"load_font", api::render::load_font,
		"load_image_from_memory", api::render::load_image_from_memory,
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
		"measure_text", api::render::measure_text,
		"push_clip_rect", api::render::push_clip_rect,
		"pop_clip_rect", api::render::pop_clip_rect,
		"world_to_screen", api::vector::to_screen,
		"set_antialias", api::render::set_antialias
	);

	// utils
	lua.create_named_table("utils",
		"console_exec", api::utils::console_exec,
		"create_interface", api::utils::create_interface,
		"pattern_scan", api::utils::pattern_scan
	);

	RefreshScripts();
	Config->lua_button->set_callback(ScriptLoadButton);
	Config->lua_button_unload->set_callback(ScriptUnloadButton);
	Config->lua_save->set_callback(ScriptSaveButton);
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
		Menu->RemoveElement(element);
	}

	script.ui_elements.clear();

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

std::vector<std::string> CLua::GetUIList() {
	std::vector<std::string> result;

	for (auto& script : scripts) {
		result.emplace_back(script.ui_name);
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