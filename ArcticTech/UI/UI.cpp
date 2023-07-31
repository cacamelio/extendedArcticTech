#include "UI.h"



void CMenu::SetupUI() {
	AddGroupBox("Aimbot", "Aimbot");
	AddGroupBox("Aimbot", "Settings");

	AddGroupBox("Anti aim", "Angles");
	AddGroupBox("Anti aim", "Fake lag", 1.f, 1);
	AddGroupBox("Anti aim", "Other", 1.f, 1);

	AddCheckBox("Aimbot", "Aimbot", "Enable");
	AddKeyBind("Aimbot", "Aimbot", "Enable");
	AddCheckBox("Aimbot", "Aimbot", "Peek assist");
	AddColorPicker("Aimbot", "Aimbot", "Peek assist");
	AddKeyBind("Aimbot", "Aimbot", "Minimum damage override");
	AddMultiCombo("Aimbot", "Settings", "Hitboxes", {"Head", "Chest", "Stomach", "Arms", "Legs"});
	AddSliderInt("Aimbot", "Settings", "Hitchance", 0, 100, 50, "%d%%");
	AddSliderFloat("Aimbot", "Settings", "Min. damage", 0.f, 120.f, 40.f);
	AddComboBox("Aimbot", "Settings", "Auto Scope", { "Disabled", "Always", "Hitchance fail" });
	AddButton("Aimbot", "Settings", "Btn");
	AddInput("Aimbot", "Settings", "input");
}