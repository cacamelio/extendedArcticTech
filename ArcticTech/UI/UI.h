#pragma once

#include <Windows.h>
#include <vector>
#include <string>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../ImGui/imgui.h"
#include "../Features/Lua/Sol.hpp"
#include "../SDK/Misc/Color.h"

class CMenuGroupbox;
class IBaseWidget;

struct UILuaCallback_t {
	IBaseWidget* ref;
	int script_id;
	sol::protected_function func;
};
typedef void(*tUiCallback)();

enum class WidgetType {
	Checkbox,
	SliderInt,
	SliderFloat,
	KeyBind,
	Label,
	ColorPicker,
	Combo,
	MultiCombo,
	Button,
	Input,
	Any // used for menu.find only
};

class IBaseWidget {
public:
	std::string name;
	CMenuGroupbox* parent = nullptr;
	IBaseWidget* additional = nullptr;
	bool visible = true;
	std::vector<UILuaCallback_t> lua_callbacks;
	std::vector<tUiCallback> callbacks;

	void SetCallback(tUiCallback callback) { 
		callbacks.emplace_back(callback);
		if (this->GetType() != WidgetType::Button)
			callback();
	};
	void SetVisible(bool visible_) { visible = visible_; };

	virtual WidgetType GetType() = 0;
	virtual void Render() = 0;
};

class CKeyBind : public IBaseWidget {
	bool pressed_once = false;
public:
	int key = 0;
	int mode = 2;
	bool toggled = false;

	bool get();

	virtual WidgetType GetType() { return WidgetType::KeyBind; };
	virtual void Render();
};

class CCheckBox : public IBaseWidget {
public:
	bool value;

	bool get(bool use_keybind = true) {
		if (!use_keybind || !additional || additional->GetType() != WidgetType::KeyBind)
			return value;

		return value && reinterpret_cast<CKeyBind*>(additional)->get();
	};

	virtual WidgetType GetType() { return WidgetType::Checkbox; };
	virtual void Render();
};

class CSliderInt : public IBaseWidget {
public:
	int value;
	int min;
	int max;
	std::string format;
	ImGuiSliderFlags flags;

	int get() { return value; };

	virtual WidgetType GetType() { return WidgetType::SliderInt; };
	virtual void Render();
};

class CSliderFloat : public IBaseWidget {
public:
	float value;
	float min;
	float max;
	std::string format;
	ImGuiSliderFlags flags;

	float get() { return value; };

	virtual WidgetType GetType() { return WidgetType::SliderFloat; };
	virtual void Render();
};

class CLabel : public IBaseWidget {
public:

	virtual WidgetType GetType() { return WidgetType::Label; };
	virtual void Render();
};

class CColorPicker : public IBaseWidget {
public:
	float value[4];
	bool has_alpha = false;

	Color get() { return Color(value[0] * 255, value[1] * 255, value[2] * 255, value[3] * 255); };

	virtual WidgetType GetType() { return WidgetType::ColorPicker; };
	virtual void Render();
};

class CComboBox : public IBaseWidget {
public:
	std::vector<const char*> elements;
	int value = 0;

	int get() { return value; };
	std::string get_name() { 
		if (elements.size() == 0)
			return "";
		return elements[value];
	};
	void UpdateList(const std::vector<const char*>& new_el) { 
		elements = new_el; 
	};

	virtual WidgetType GetType() { return WidgetType::Combo; };
	virtual void Render();
};

class CMultiCombo : public IBaseWidget {
public:
	std::vector<const char*> elements;
	bool* value;

	bool get(int i) { return value[i]; };
	void UpdateList(const std::vector<const char*>& new_el) { 
		elements = new_el;
		delete[] value;
		value = new bool[new_el.size()];
	};

	virtual WidgetType GetType() { return WidgetType::Combo; };
	virtual void Render();

	~CMultiCombo() {
		if (value)
			delete[] value;
	}
};

class CButton : public IBaseWidget {
public:
	virtual WidgetType GetType() { return WidgetType::Button; };
	virtual void Render();
};

class CInputBox : public IBaseWidget {
public:
	char buf[64];
	ImGuiInputTextFlags flags;

	const std::string get() { return std::string(buf); };

	virtual WidgetType GetType() { return WidgetType::Input; };
	virtual void Render();
};

class CMenuGroupbox {
public:
	int column = 0;
	float relative_size = 1.f;
	ImVec2 position;
	ImVec2 size;

	std::string name;
	std::vector<IBaseWidget*> widgets;

	void Render();
};

class CMenu {
private:
	bool m_bMenuOpened = true;
	bool m_bIsInitialized = false;
	ImVec2 m_WindowSize;
	ImVec2 m_ItemSpacing;

	std::vector<CMenuGroupbox*> m_Groupboxes[8];

	void RecalculateGroupboxes();
public:
	bool			IsOpened() { return m_bMenuOpened; };
	bool			IsInitialized() { return m_bIsInitialized; };

	void			Setup();
	void			SetupUI();
	void			Release();
	void			Render();
	void			WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void			AddGroupBox(const std::string& tab, const std::string& groupbox, float relative_size = 1.f, int column = -1);
	CCheckBox*		AddCheckBox(const std::string& tab, const std::string& groupbox, const std::string& name, bool init = false);
	CSliderInt*		AddSliderInt(const std::string& tab, const std::string& groupbox, const std::string& name, int min, int max, int init, const std::string& format = "%d", ImGuiSliderFlags flags = 0);
	CSliderFloat*	AddSliderFloat(const std::string& tab, const std::string& groupbox, const std::string& name, float min, float max, float init, const std::string& format = "%.3f", ImGuiSliderFlags flags = 0);
	CKeyBind*		AddKeyBind(const std::string& tab, const std::string& groupbox, const std::string& name);
	CLabel*			AddLabel(const std::string& tab, const std::string& groupbox, const std::string& name);
	CColorPicker*	AddColorPicker(const std::string& tab, const std::string& groupbox, const std::string& name, Color color = Color(), bool has_alpha = true);
	CComboBox*		AddComboBox(const std::string& tab, const std::string& groupbox, const std::string& name, std::vector<const char*> items);
	CMultiCombo*	AddMultiCombo(const std::string& tab, const std::string& groupbox, const std::string& name, std::vector<const char*> items);
	CButton*		AddButton(const std::string& tab, const std::string& groupbox, const std::string& name);
	CInputBox*		AddInput(const std::string& tab, const std::string& groupbox, const std::string& name, const std::string& init = "", ImGuiInputTextFlags flags = 0);

	CMenuGroupbox*	FindGroupbox(const std::string& tab, const std::string& groupbox);
	IBaseWidget*	FindItem(const std::string& tab, const std::string& groupbox, const std::string& name, WidgetType type = WidgetType::Any);
	void			RemoveItem(IBaseWidget* item);
};

extern CMenu* Menu;