#pragma once

#include "../../SDK/Interfaces/IBaseClientDLL.h"

class CBaseCombatWeapon;

class CSkinChanger {
public:
	void ApplyKnifeModel(CBaseCombatWeapon* knife, int definitionIndex);
	void Run(EClientFrameStage stage);
};

