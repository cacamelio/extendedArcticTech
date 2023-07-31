#pragma once
#include "../SDK/Render.h"
#include "menu_elements.h"

class CGroupBox {
public:
	Vector2 pos;
	Vector2 size;
	Vector2 clipRect[2];
	std::string name;
	int tabId;
	int column;
	int scrollOffset = 0;
	float textSize;
	float scale = 0;
	int minsize = 30;

	bool dragging = false;
	bool resizing = false;
	bool scrolling = false;

	int scrollMouseOffset = 0;

	std::vector<IBaseElement*> elements;

	void Draw();
	void HandleClick();
	void HandleScroll(bool up);
	void Update();
};

class CMenuOld
{
private:
	bool opened = true;
	bool dragging = false;

	int activeTab = 0;

	IDirect3DTexture9* aimImg;
	IDirect3DTexture9* visualsImg;
	IDirect3DTexture9* backgroundTexture;
	IDirect3DTexture9* miscImg;
	IDirect3DTexture9* antiaimImg;
	IDirect3DTexture9* skinchangerImg;
	IDirect3DTexture9* configImg;
	IDirect3DTexture9* playerListImg;
	IDirect3DTexture9* luaImg;

	inline int get_tab_id(const std::string& name) {
		if (name == "RAGE")
			return 0;
		else if (name == "ANTIAIM")
			return 1;
		else if (name == "VISUALS")
			return 2;
		else if (name == "MISC")
			return 3;
		else if (name == "SKINS")
			return 4;
		else if (name == "PLAYERS")
			return 5;
		else if (name == "CONFIG")
			return 6;
		else if (name == "LUA")
			return 7;
		return 0;
	};
public:
	bool initialized = false;
	CColorPickerOld* menu_color_picker;
	CKeyBindOld* menu_key_bind;
	std::vector<CGroupBox*> group_boxes;
	Vector2 menuPos{ 200, 200 };
	Vector2 size{ 760, 710 };
	Color menuColor{ 150, 210, 25 };
	bool autoscaling{ true };
	CGroupBox* transformingGroup{ nullptr };
	Vector2 deadZoneStart;
	Vector2 deadZoneEnd;
	bool isInDeadZone{ false };
	bool resizing{ false };

	Color copyColor;

	inline bool is_opened() { return opened; };

	void Init();
	void WndProc(UINT msg, WPARAM wParam);

	void Draw();
	CGroupBox*		AddGroupBox(const std::string& tab, const std::string& name, float scale, int column);
	CCheckboxOld*		AddCheckBox(const std::string& tab, const std::string& groupbox, const std::string& name, bool unsafe = false);
	CLabelOld*			AddLabel(const std::string& tab, const std::string& groupbox, const std::string& name);
	CColorPickerOld*	AddColorPicker(const std::string& tab, const std::string& groupbox, const std::string& name, Color defaultColor = Color(255, 255, 255));
	CKeyBindOld*		AddKeyBind(const std::string& tab, const std::string& groupbox, const std::string& name, int defaultKey = -1, int defaultType = 1);
	CSliderOld*		AddSlider(const std::string& tab, const std::string& groupbox, const std::string& name, float min, float max, float def, const std::string& unit = "", float scale = 1.f, bool hideName = false);
	CComboBoxOld*		AddComboBox(const std::string& tab, const std::string& groupbox, const std::string& name, std::vector<std::string> items, int def = 0, bool hideName = false);
	CMultiComboOld*	AddMultiCombo(const std::string& tab, const std::string& groupbox, const std::string& name, std::vector<std::string> items, int def = 0, bool hideName = false);
	CButtonOld*		AddButton(const std::string& tab, const std::string& groupbox, const std::string& name);
	CInputBoxOld*		AddInputBox(const std::string& tab, const std::string& groupbox, const std::string& name, bool hidden = false);
	CListBox*		AddListBox(const std::string& tab, const std::string& groupbox, const std::string& name, std::vector<std::string> items, bool filter = true);

	IBaseElement*	FindElement(const std::string& tab, const std::string& groupbox, const std::string& name, ElementType type = ElementType::ANY);
	void			RemoveElement(IBaseElement* element);
};

extern CMenuOld* MenuOld;