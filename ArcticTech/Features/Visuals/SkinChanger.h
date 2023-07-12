#pragma once

#include "../../SDK/Interfaces/IBaseClientDLL.h"
#include "../../SDK/Globals.h"
#include "../../SDK/Config.h"
#include "../../SDK/Interfaces.h"
#include "../../SDK/Misc/CBaseCombatWeapon.h"

class CBaseCombatWeapon;



class CSkinChanger {
public:
	bool ApplyKnifeModel( attributable_item_t* weapon, const char* model );
	void AgentChanger( EClientFrameStage stage );
	void Run();
};

extern CSkinChanger* SkinChanger;