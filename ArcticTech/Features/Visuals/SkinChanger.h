#pragma once

#include "../../SDK/Interfaces/IBaseClientDLL.h"
#include "../../SDK/Globals.h"
#include "../../SDK/Config.h"
#include "../../SDK/Interfaces.h"
#include "../../SDK/Misc/CBaseCombatWeapon.h"

class CBaseCombatWeapon;

struct KnifeModel_t {
	std::string ui_name;
	std::string model_name;
};

class CSkinChanger {
	std::vector<KnifeModel_t> knife_models;
public:
	void LoadKnifeModels();
	std::vector<std::string> GetUIKnifeModels();
	bool ApplyKnifeModel( CAttributableItem* weapon, const char* model );
	void FixViewModelSequence( CAttributableItem* weapon );
	void AgentChanger( );
	void Run();
};

extern CSkinChanger* SkinChanger;