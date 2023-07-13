#pragma once

#include "../../SDK/Interfaces/IBaseClientDLL.h"
#include "../../SDK/Globals.h"
#include "../../SDK/Config.h"
#include "../../SDK/Interfaces.h"
#include "../../SDK/Misc/CBaseCombatWeapon.h"

#define SEQUENCE_DEFAULT_DRAW 0
#define SEQUENCE_DEFAULT_IDLE1 1
#define SEQUENCE_DEFAULT_IDLE2 2
#define SEQUENCE_DEFAULT_LIGHT_MISS1 3
#define SEQUENCE_DEFAULT_LIGHT_MISS2 4
#define SEQUENCE_DEFAULT_HEAVY_MISS1 9
#define SEQUENCE_DEFAULT_HEAVY_HIT1 10
#define SEQUENCE_DEFAULT_HEAVY_BACKSTAB 11
#define SEQUENCE_DEFAULT_LOOKAT01 12

#define SEQUENCE_BUTTERFLY_DRAW 0
#define SEQUENCE_BUTTERFLY_DRAW2 1
#define SEQUENCE_BUTTERFLY_LOOKAT01 13
#define SEQUENCE_BUTTERFLY_LOOKAT03 15

#define SEQUENCE_FALCHION_IDLE1 1
#define SEQUENCE_FALCHION_HEAVY_MISS1 8
#define SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP 9
#define SEQUENCE_FALCHION_LOOKAT01 12
#define SEQUENCE_FALCHION_LOOKAT02 13

#define SEQUENCE_DAGGERS_IDLE1 1
#define SEQUENCE_DAGGERS_LIGHT_MISS1 2
#define SEQUENCE_DAGGERS_LIGHT_MISS5 6
#define SEQUENCE_DAGGERS_HEAVY_MISS2 11
#define SEQUENCE_DAGGERS_HEAVY_MISS1 12

#define SEQUENCE_BOWIE_IDLE1 1
#define RandomIntDef(nMin, nMax) (rand() % (nMax - nMin + 1) + nMin);

class CBaseCombatWeapon;

struct KnifeModel_t {
	std::string ui_name;
	std::string model_name;
};

class CSkinChanger {
	std::vector<KnifeModel_t> knife_models;
	RecvVarProxy_t fnSequenceProxyFn = nullptr;
	RecvVarProxy_t oRecvnModelIndex;
public:
	void LoadKnifeModels();
	std::vector<std::string> GetUIKnifeModels();
	bool ApplyKnifeModel( CAttributableItem* weapon, const char* model );
	static void SetViewModelSequence( const CRecvProxyData* pDataConst, void* pStruct, void* pOut );
	static void Hooked_RecvProxy_Viewmodel( CRecvProxyData* pData, void* pStruct, void* pOut );
	void FixViewModelSequence();
	void AnimationUnHook( );
	void AgentChanger( );
	void Run( bool frame_end );
};

extern CSkinChanger* SkinChanger;