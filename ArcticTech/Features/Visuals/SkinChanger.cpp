#include "SkinChanger.h"

#include "../../SDK/Globals.h"
#include "../../SDK/Config.h"
#include "../../SDK/Interfaces.h"
#include "../../SDK/Misc/CBaseCombatWeapon.h"


void CSkinChanger::ApplyKnifeModel(CBaseCombatWeapon* knife, int definitionIndex) {
	auto weapon_data = WeaponSystem->GetWeaponData(definitionIndex);

	if (!weapon_data)
		return;

	auto model_index = ModelInfoClient->GetModelIndex(weapon_data->szViewModel);

	knife->SetModelIndex(model_index);
}

void CSkinChanger::Run(EClientFrameStage stage) {
	if (!Cheat.InGame || !Cheat.LocalPlayer || !Cheat.LocalPlayer->IsAlive())
		return;

	
}