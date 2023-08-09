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

struct PaintKit
{
	int id;
	const wchar_t* name;
};

class CCStrike15ItemSchema;
class CCStrike15ItemSystem;
struct CPaintKit;
struct Node_t;
struct Head_t;

struct Head_t
{
	Node_t* pMemory;		//0x0000
	int nAllocationCount;	//0x0004
	int nGrowSize;			//0x0008
	int nStartElement;		//0x000C
	int nNextAvailable;		//0x0010
	int _unknown;			//0x0014
	int nLastElement;		//0x0018
}; //Size=0x001C

struct Node_t
{
	int nPreviousId;		//0x0000
	int nNextId;			//0x0004
	void* _unknown_ptr;		//0x0008
	int _unknown;			//0x000C
	int iPaintKitId;		//0x0010
	CPaintKit* pPaintKit;	//0x0014
}; //Size=0x0018

struct String_t
{
	char* szBuffer;	//0x0000 
	int nCapacity;	//0x0004 
	int _unknown;	//0x0008 
	int nLength;	//0x000C 
}; //Size=0x0010

struct CPaintKit
{
	int iIndex;				//0x0000
	String_t Name;			//0x0004
	String_t NiceName;		//0x0014
	String_t Tag;			//0x0024
	String_t ShortName;		//0x0034
	String_t _unknown;		//0x0044
	char pad_0x0054[0x8C];	//0x0054
}; //Size=0x00E0


class CSkinChanger {
	std::vector<KnifeModel_t> knife_models;
	RecvVarProxy_t fnSequenceProxyFn = nullptr;
	RecvVarProxy_t oRecvnModelIndex;
public:
	void LoadKnifeModels();
	void InitPaintkit();
	std::vector<PaintKit> vecKits;
	std::vector<PaintKit> vecKits_gloves;
	std::vector<PaintKit> GetPaintKits();
	std::vector<PaintKit> GetGlovePaintKits();
	std::vector<std::string> GetUIPaintKits();
	std::vector<std::string> GetUIPaintKitsGloves();
	std::vector<std::string> GetUIKnifeModels();
	bool ApplyKnifeModel( CAttributableItem* weapon, const char* model );
	bool ApplyKnifeSkin(CAttributableItem* pWeapon, const char* szModel, int iItemDefIndex, int iPaintKit, int iEntityQuality, float flFallbackWear);
	static void SetViewModelSequence( const CRecvProxyData* pDataConst, void* pStruct, void* pOut );
	static void Hooked_RecvProxy_Viewmodel( CRecvProxyData* pData, void* pStruct, void* pOut );
	void FixViewModelSequence();
	void AnimationUnHook( );
	const char* default_mask = "models/player/holiday/facemasks/facemask_battlemask.mdl";

	bool LoadModel( const char* thisModelName );
	void InitCustomModels();
	void UpdateSkins();
	void MaskChanger();
	typedef void(__thiscall* UpdateAddonModelsFunc)(void*, bool);

	void AgentChanger( );
	static constexpr std::array models_ct {
			"models/player/custom_player/legacy/ctm_diver_varianta.mdl", // Cmdr. Davida "Goggles" Fernandez | SEAL Frogman
			"models/player/custom_player/legacy/ctm_diver_variantb.mdl", // Cmdr. Frank "Wet Sox" Baroud | SEAL Frogman
			"models/player/custom_player/legacy/ctm_diver_variantc.mdl", // Lieutenant Rex Krikey | SEAL Frogman
			"models/player/custom_player/legacy/ctm_fbi_varianth.mdl", // Michael Syfers | FBI Sniper
			"models/player/custom_player/legacy/ctm_fbi_variantf.mdl", // Operator | FBI SWAT
			"models/player/custom_player/legacy/ctm_fbi_variantb.mdl", // Special Agent Ava | FBI
			"models/player/custom_player/legacy/ctm_fbi_variantg.mdl", // Markus Delrow | FBI HRT
			"models/player/custom_player/legacy/ctm_gendarmerie_varianta.mdl", // Sous-Lieutenant Medic | Gendarmerie Nationale
			"models/player/custom_player/legacy/ctm_gendarmerie_variantb.mdl", // Chem-Haz Capitaine | Gendarmerie Nationale
			"models/player/custom_player/legacy/ctm_gendarmerie_variantc.mdl", // Chef d"Escadron Rouchard | Gendarmerie Nationale
			"models/player/custom_player/legacy/ctm_gendarmerie_variantd.mdl", // Aspirant | Gendarmerie Nationale
			"models/player/custom_player/legacy/ctm_gendarmerie_variante.mdl", // Officer Jacques Beltram | Gendarmerie Nationale
			"models/player/custom_player/legacy/ctm_sas_variantg.mdl", // D Squadron Officer | NZSAS
			"models/player/custom_player/legacy/ctm_sas_variantf.mdl", // B Squadron Officer | SAS
			"models/player/custom_player/legacy/ctm_st6_variante.mdl", // Seal Team 6 Soldier | NSWC SEAL
			"models/player/custom_player/legacy/ctm_st6_variantg.mdl", // Buckshot | NSWC SEAL
			"models/player/custom_player/legacy/ctm_st6_varianti.mdl", // Lt. Commander Ricksaw | NSWC SEAL
			"models/player/custom_player/legacy/ctm_st6_variantj.mdl", // "Blueberries" Buckshot | NSWC SEAL
			"models/player/custom_player/legacy/ctm_st6_variantk.mdl", // 3rd Commando Company | KSK
			"models/player/custom_player/legacy/ctm_st6_variantl.mdl", // "Two Times" McCoy | TACP Cavalry
			"models/player/custom_player/legacy/ctm_st6_variantm.mdl", // "Two Times" McCoy | USAF TACP
			"models/player/custom_player/legacy/ctm_st6_variantn.mdl", // Primeiro Tenente | Brazilian 1st Battalion
			"models/player/custom_player/legacy/ctm_swat_variante.mdl", // Cmdr. Mae "Dead Cold" Jamison | SWAT
			"models/player/custom_player/legacy/ctm_swat_variantf.mdl", // 1st Lieutenant Farlow | SWAT
			"models/player/custom_player/legacy/ctm_swat_variantg.mdl", // John "Van Healen" Kask | SWAT
			"models/player/custom_player/legacy/ctm_swat_varianth.mdl", // Bio-Haz Specialist | SWAT
			"models/player/custom_player/legacy/ctm_swat_varianti.mdl", // Sergeant Bombson | SWAT
			"models/player/custom_player/legacy/ctm_swat_variantj.mdl", // Chem-Haz Specialist | SWAT
			"models/player/custom_player/legacy/ctm_swat_variantk.mdl", // Lieutenant "Tree Hugger" Farlow | SWAT
			"models/player/custom_player/legacy/ctm_gign_varianta.mdl" // GIGN (a)
	};


	static constexpr std::array models_t {
		"models/player/custom_player/legacy/tm_professional_varj.mdl", // Getaway Sally | The Professionals
			"models/player/custom_player/legacy/tm_professional_vari.mdl", // Number K | The Professionals
			"models/player/custom_player/legacy/tm_professional_varh.mdl", // Little Kev | The Professionals
			"models/player/custom_player/legacy/tm_professional_varg.mdl", // Safecracker Voltzmann | The Professionals
			"models/player/custom_player/legacy/tm_professional_varf5.mdl", // Bloody Darryl The Strapped | The Professionals
			"models/player/custom_player/legacy/tm_professional_varf4.mdl", // Sir Bloody Loudmouth Darryl | The Professionals
			"models/player/custom_player/legacy/tm_professional_varf3.mdl", // Sir Bloody Darryl Royale | The Professionals
			"models/player/custom_player/legacy/tm_professional_varf2.mdl", // Sir Bloody Skullhead Darryl | The Professionals
			"models/player/custom_player/legacy/tm_professional_varf1.mdl", // Sir Bloody Silent Darryl | The Professionals
			"models/player/custom_player/legacy/tm_professional_varf.mdl", // Sir Bloody Miami Darryl | The Professionals
			"models/player/custom_player/legacy/tm_phoenix_varianti.mdl", // Street Soldier | Phoenix
			"models/player/custom_player/legacy/tm_phoenix_varianth.mdl", // Soldier | Phoenix
			"models/player/custom_player/legacy/tm_phoenix_variantg.mdl", // Slingshot | Phoenix
			"models/player/custom_player/legacy/tm_phoenix_variantf.mdl", // Enforcer | Phoenix
			"models/player/custom_player/legacy/tm_leet_variantj.mdl", // Mr. Muhlik | Elite Crew
			"models/player/custom_player/legacy/tm_leet_varianti.mdl", // Prof. Shahmat | Elite Crew
			"models/player/custom_player/legacy/tm_leet_varianth.mdl", // Osiris | Elite Crew
			"models/player/custom_player/legacy/tm_leet_variantg.mdl", // Ground Rebel | Elite Crew
			"models/player/custom_player/legacy/tm_leet_variantf.mdl", // The Elite Mr. Muhlik | Elite Crew
			"models/player/custom_player/legacy/tm_jungle_raider_variantf2.mdl", // Trapper | Guerrilla Warfare
			"models/player/custom_player/legacy/tm_jungle_raider_variantf.mdl", // Trapper Aggressor | Guerrilla Warfare
			"models/player/custom_player/legacy/tm_jungle_raider_variante.mdl", // Vypa Sista of the Revolution | Guerrilla Warfare
			"models/player/custom_player/legacy/tm_jungle_raider_variantd.mdl", // Col. Mangos Dabisi | Guerrilla Warfare
			"models/player/custom_player/legacy/tm_jungle_raider_variantñ.mdl", // Arno The Overgrown | Guerrilla Warfare
			"models/player/custom_player/legacy/tm_jungle_raider_variantb2.mdl", // "Medium Rare" Crasswater | Guerrilla Warfare
			"models/player/custom_player/legacy/tm_jungle_raider_variantb.mdl", // Crasswater The Forgotten | Guerrilla Warfare
			"models/player/custom_player/legacy/tm_jungle_raider_varianta.mdl", // Elite Trapper Solman | Guerrilla Warfare
			"models/player/custom_player/legacy/tm_balkan_varianth.mdl", // "The Doctor" Romanov | Sabre
			"models/player/custom_player/legacy/tm_balkan_variantj.mdl", // Blackwolf | Sabre
			"models/player/custom_player/legacy/tm_balkan_varianti.mdl", // Maximus | Sabre
			"models/player/custom_player/legacy/tm_balkan_variantf.mdl", // Dragomir | Sabre
			"models/player/custom_player/legacy/tm_balkan_variantg.mdl", // Rezan The Ready | Sabre
			"models/player/custom_player/legacy/tm_balkan_variantk.mdl", // Rezan the Redshirt | Sabre
			"models/player/custom_player/legacy/tm_balkan_variantl.mdl", // Dragomir | Sabre Footsoldier
			"models/player/custom_player/legacy/tm_jumpsuit_varianta.mdl", // Danger zone (a)
			"models/player/custom_player/legacy/tm_jumpsuit_variantb.mdl" // Danger zone (b)
	};

	static constexpr std::array mask_models
	{	"",
		"models/player/holiday/facemasks/facemask_dallas.mdl",
		"models/player/holiday/facemasks/facemask_battlemask.mdl",
		"models/player/holiday/facemasks/evil_clown.mdl",
		"models/player/holiday/facemasks/facemask_anaglyph.mdl",
		"models/player/holiday/facemasks/facemask_boar.mdl",
		"models/player/holiday/facemasks/facemask_bunny.mdl",
		"models/player/holiday/facemasks/facemask_bunny_gold.mdl",
		"models/player/holiday/facemasks/facemask_chains.mdl",
		"models/player/holiday/facemasks/facemask_chicken.mdl",
		"models/player/holiday/facemasks/facemask_devil_plastic.mdl",
		"models/player/holiday/facemasks/facemask_hoxton.mdl",
		"models/player/holiday/facemasks/facemask_pumpkin.mdl",
		"models/player/holiday/facemasks/facemask_samurai.mdl",
		"models/player/holiday/facemasks/facemask_sheep_bloody.mdl",
		"models/player/holiday/facemasks/facemask_sheep_gold.mdl",
		"models/player/holiday/facemasks/facemask_sheep_model.mdl",
		"models/player/holiday/facemasks/facemask_skull.mdl",
		"models/player/holiday/facemasks/facemask_template.mdl",
		"models/player/holiday/facemasks/facemask_wolf.mdl",
		"models/player/holiday/facemasks/porcelain_doll.mdl",
	};

	void ApplyGlove(CAttributableItem* pGlove);

	void Run( bool frame_end );
};

extern CSkinChanger* SkinChanger;