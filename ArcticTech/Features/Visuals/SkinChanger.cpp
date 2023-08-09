#include "SkinChanger.h"

void CSkinChanger::LoadKnifeModels() {
	for (int i = 0; i < 1000; i++) { // last knife index is ~500 but this func is called once
		CCSWeaponData* wdata = WeaponSystem->GetWeaponData(i);

		if (!wdata || wdata->nWeaponType != WEAPONTYPE_KNIFE || strstr(wdata->szViewModel, "taser"))
			continue;

		knife_models.emplace_back(KnifeModel_t{ wdata->GetName(), wdata->szViewModel });
	}
}

std::vector<int> vecWeapons = {
	WEAPON_AK47,
	WEAPON_AUG,
	WEAPON_AWP,
	WEAPON_CZ75A,
	WEAPON_DEAGLE,
	WEAPON_ELITE,
	WEAPON_FAMAS,
	WEAPON_FIVESEVEN,
	WEAPON_G3SG1,
	WEAPON_GALILAR,
	WEAPON_GLOCK,
	WEAPON_M249,
	WEAPON_M4A1_SILENCER,
	WEAPON_M4A1,
	WEAPON_MAC10,
	WEAPON_MAG7,
	WEAPON_MP5SD,
	WEAPON_MP7,
	WEAPON_MP9,
	WEAPON_NEGEV,
	WEAPON_NOVA,
	WEAPON_HKP2000,
	WEAPON_P250,
	WEAPON_P90,
	WEAPON_BIZON,
	WEAPON_REVOLVER,
	WEAPON_SAWEDOFF,
	WEAPON_SCAR20,
	WEAPON_SSG08,
	WEAPON_SG556,
	WEAPON_TEC9,
	WEAPON_UMP45,
	WEAPON_USP_SILENCER,
	WEAPON_XM1014,
	WEAPON_KNIFE_BAYONET,
	WEAPON_KNIFE_FLIP,
	WEAPON_KNIFE_GUT,
	WEAPON_KNIFE_KARAMBIT,
	WEAPON_KNIFE_M9_BAYONET,
	WEAPON_KNIFE_TACTICAL,
	WEAPON_KNIFE_FALCHION,
	WEAPON_KNIFE_SURVIVAL_BOWIE,
	WEAPON_KNIFE_BUTTERFLY,
	WEAPON_KNIFE_PUSH,
	WEAPON_KNIFE_URSUS,
	WEAPON_KNIFE_GYPSY_JACKKNIFE,
	WEAPON_KNIFE_STILETTO,
	WEAPON_KNIFE_WIDOWMAKER,
	WEAPON_KNIFE_CSS,
	WEAPON_KNIFE_OUTDOOR,
	WEAPON_KNIFE_SKELETON,
	WEAPON_KNIFE_CORD,
	WEAPON_KNIFE_CANIS,
	GLOVE_STUDDED_BLOODHOUND,
	GLOVE_SPORTY,
	GLOVE_SLICK,
	GLOVE_LEATHER_WRAP,
	GLOVE_MOTORCYCLE,
	GLOVE_SPECIALIST,
	GLOVE_STUDDED_HYDRA,
	GLOVE_STUDDED_BROKENFANG
};


std::string wchar_to_UTF8(const wchar_t* in)
{
	std::string out;
	unsigned int codepoint = 0;
	for (in; *in != 0; ++in)
	{
		if (*in >= 0xd800 && *in <= 0xdbff)
			codepoint = ((*in - 0xd800) << 10) + 0x10000;
		else
		{
			if (*in >= 0xdc00 && *in <= 0xdfff)
				codepoint |= *in - 0xdc00;
			else
				codepoint = *in;

			if (codepoint <= 0x7f)
				out.append(1, static_cast<char>(codepoint));
			else if (codepoint <= 0x7ff)
			{
				out.append(1, static_cast<char>(0xc0 | ((codepoint >> 6) & 0x1f)));
				out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
			}
			else if (codepoint <= 0xffff)
			{
				out.append(1, static_cast<char>(0xe0 | ((codepoint >> 12) & 0x0f)));
				out.append(1, static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
				out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
			}
			else
			{
				out.append(1, static_cast<char>(0xf0 | ((codepoint >> 18) & 0x07)));
				out.append(1, static_cast<char>(0x80 | ((codepoint >> 12) & 0x3f)));
				out.append(1, static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
				out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
			}
			codepoint = 0;
		}
	}
	return out;
}

std::vector<PaintKit> CSkinChanger::GetPaintKits()
{
	auto dwSigAddress = (DWORD)Utils::PatternScan("client.dll", "E8 ? ? ? ? FF 76 0C 8D 48 04 E8");

	// Skip the opcode, read rel32 address
	auto iItemSystemOffset = *reinterpret_cast<int32_t*>(dwSigAddress + 1);
	// Add the offset to the end of the instruction
	auto pItemSystem = reinterpret_cast<CCStrike15ItemSystem * (*)()>(dwSigAddress + 5 + iItemSystemOffset);

	// Skip the instructions between, skip the opcode, read rel32 address
	auto iGetPaintKitDefinitionOffset = *reinterpret_cast<int32_t*>(dwSigAddress + 11 + 1);
	// Add the offset to the end of the instruction
	auto pGetPaintKitDefinition = reinterpret_cast<CPaintKit * (__thiscall*)(int id)>(dwSigAddress + 11 + 5 + iGetPaintKitDefinitionOffset);

	// The last offset is nHeadElement, we need that

	// push    ebp
	// mov     ebp, esp
	// sub     esp, 0Ch
	// mov     eax, [ecx+298h]

	// Skip instructions, skip opcode, read offset
	auto dwStartElementOffset = *reinterpret_cast<uint32_t*>(uintptr_t(pGetPaintKitDefinition) + 8 + 2);

	// Calculate head base from nStartElement's offset
	auto dwHeadOffset = dwStartElementOffset - 12;

	// Skip VTable, first member variable of ItemSystem is ItemSchema
	auto pItemSchema = reinterpret_cast<CCStrike15ItemSchema*>(uintptr_t(pItemSystem()) + sizeof(void*));

	auto pHead = reinterpret_cast<Head_t*>(uintptr_t(pItemSchema) + dwHeadOffset);

	for (int i = 0; i <= pHead->nLastElement; ++i)
	{
		auto pKit = pHead->pMemory[i].pPaintKit;

		if (pKit->iIndex == 9001)
			continue;

		const wchar_t* wstrName = Localize->Find(pKit->Tag.szBuffer + 1);
		if (pKit->iIndex < 10000)
			vecKits.push_back(PaintKit{ pKit->iIndex, wstrName });
	}

	return vecKits;
}

std::vector<PaintKit> CSkinChanger::GetGlovePaintKits()
{
	auto dwSigAddress = (DWORD)Utils::PatternScan("client.dll", "E8 ? ? ? ? FF 76 0C 8D 48 04 E8");

	// Skip the opcode, read rel32 address
	auto iItemSystemOffset = *reinterpret_cast<int32_t*>(dwSigAddress + 1);
	// Add the offset to the end of the instruction
	auto pItemSystem = reinterpret_cast<CCStrike15ItemSystem * (*)()>(dwSigAddress + 5 + iItemSystemOffset);

	// Skip the instructions between, skip the opcode, read rel32 address
	auto iGetPaintKitDefinitionOffset = *reinterpret_cast<int32_t*>(dwSigAddress + 11 + 1);
	// Add the offset to the end of the instruction
	auto pGetPaintKitDefinition = reinterpret_cast<CPaintKit * (__thiscall*)(int id)>(dwSigAddress + 11 + 5 + iGetPaintKitDefinitionOffset);

	// The last offset is nHeadElement, we need that

	// push    ebp
	// mov     ebp, esp
	// sub     esp, 0Ch
	// mov     eax, [ecx+298h]

	// Skip instructions, skip opcode, read offset
	auto dwStartElementOffset = *reinterpret_cast<uint32_t*>(uintptr_t(pGetPaintKitDefinition) + 8 + 2);

	// Calculate head base from nStartElement's offset
	auto dwHeadOffset = dwStartElementOffset - 12;

	// Skip VTable, first member variable of ItemSystem is ItemSchema
	auto pItemSchema = reinterpret_cast<CCStrike15ItemSchema*>(uintptr_t(pItemSystem()) + sizeof(void*));

	auto pHead = reinterpret_cast<Head_t*>(uintptr_t(pItemSchema) + dwHeadOffset);

	for (int i = 0; i <= pHead->nLastElement; ++i)
	{
		auto pKit = pHead->pMemory[i].pPaintKit;

		if (pKit->iIndex == 9001)
			continue;

		const wchar_t* wstrName = Localize->Find(pKit->Tag.szBuffer + 1);
		if (pKit->iIndex > 10000)
			vecKits_gloves.push_back(PaintKit{ pKit->iIndex, wstrName });
	}

	return vecKits_gloves;
}

std::vector<std::string> CSkinChanger::GetUIPaintKits() {
	std::vector<std::string> result{};

	auto kits = GetPaintKits();
	for (auto& model : kits)
		result.emplace_back(wchar_to_UTF8(model.name));

	return result;
}

std::vector<std::string> CSkinChanger::GetUIPaintKitsGloves() {
	std::vector<std::string> result{};

	auto kits = GetGlovePaintKits();
	for (auto& model : kits)
		result.emplace_back(wchar_to_UTF8(model.name));

	return result;
}

std::vector<std::string> CSkinChanger::GetUIKnifeModels() {
	std::vector<std::string> result;

	for (auto& model : knife_models)
		result.emplace_back(model.ui_name);

	return result;
}

bool CSkinChanger::ApplyKnifeModel( CAttributableItem* weapon, const char* model) {
	auto viewmodel = reinterpret_cast< CBaseViewModel* >( EntityList->GetClientEntityFromHandle( Cheat.LocalPlayer->m_hViewModel( )[0] ));
	if ( !viewmodel )
		return false;

	auto h_view_model_weapon = viewmodel->m_hWeapon( );
	if ( !h_view_model_weapon )
		return false;

	auto view_model_weapon = reinterpret_cast< CAttributableItem* >( EntityList->GetClientEntityFromHandle( h_view_model_weapon ) );
	if ( view_model_weapon != weapon )
		return false;

	auto world_model_weapon = EntityList->GetClientEntityFromHandle(view_model_weapon->m_hWeaponWorldModel());

	int model_index = ModelInfoClient->GetModelIndex(model);

	viewmodel->SetModelIndex(model_index);

	if (world_model_weapon)
		world_model_weapon->SetModelIndex(model_index + 1);

	return true;
}

bool CSkinChanger::ApplyKnifeSkin(CAttributableItem* pWeapon, const char* szModel, int iItemDefIndex, int iPaintKit, int iEntityQuality, float flFallbackWear)
{
	pWeapon->m_iItemDefinitionIndex() = iItemDefIndex;
	pWeapon->m_iEntityQuality() = iEntityQuality;
	pWeapon->m_nModelIndex() = ModelInfoClient->GetModelIndex(szModel);

	CBaseHandle pWorldModelHandle = pWeapon->m_hWeaponWorldModel();
	if (!pWorldModelHandle)
		return false;

	CBaseCombatWeapon* pWorldModel = (CBaseCombatWeapon*)(EntityList->GetClientEntityFromHandle(pWorldModelHandle));
	if (!pWorldModel)
		return false;

	pWorldModel->m_hWeaponWorldModel() = ModelInfoClient->GetModelIndex(szModel) + 1;

	return true;
}

void CSkinChanger::SetViewModelSequence( const CRecvProxyData* pDataConst, void* pStruct, void* pOut ) {
	// Make the incoming data editable.
	CRecvProxyData* pData = const_cast< CRecvProxyData* >( pDataConst );

	// Confirm that we are replacing our view model and not someone elses.
	CBaseViewModel* pViewModel = ( CBaseViewModel* )pStruct;

	if ( pViewModel ) {
		auto pOwner = reinterpret_cast< CBaseEntity* >( EntityList->GetClientEntityFromHandle( uintptr_t( pViewModel->m_hOwner( ) ) ) );

		// Compare the owner entity of this view model to the local player entity.
		if ( pOwner && pOwner->EntIndex( ) == EngineClient->GetLocalPlayer( ) ) {
			// Get the filename of the current view model.
			const void* pModel = ModelInfoClient->GetModel( pViewModel->m_nModelIndex( ) );

			const char* szModel = ModelInfoClient->GetModelName( ( model_t* )pModel );

			// Store the current sequence.
			int m_nSequence = pData->m_Value.m_Int;


			if ( !strcmp( szModel, "models/weapons/v_knife_butterfly.mdl" ) ) {
				// Fix animations for the Butterfly Knife.
				switch ( m_nSequence ) {
					case SEQUENCE_DEFAULT_DRAW:
						m_nSequence = RandomIntDef( SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2 ); break;
					case SEQUENCE_DEFAULT_LOOKAT01:
						m_nSequence = RandomIntDef( SEQUENCE_BUTTERFLY_LOOKAT01, SEQUENCE_BUTTERFLY_LOOKAT03 ); break;
					default:
						m_nSequence++;
				}
			}
			else if ( !strcmp( szModel, "models/weapons/v_knife_falchion_advanced.mdl" ) ) {
				// Fix animations for the Falchion Knife.
				switch ( m_nSequence ) {
					case SEQUENCE_DEFAULT_IDLE2:
						m_nSequence = SEQUENCE_FALCHION_IDLE1; break;
					case SEQUENCE_DEFAULT_HEAVY_MISS1:
						m_nSequence = RandomIntDef( SEQUENCE_FALCHION_HEAVY_MISS1, SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP ); break;
					case SEQUENCE_DEFAULT_LOOKAT01:
						m_nSequence = RandomIntDef( SEQUENCE_FALCHION_LOOKAT01, SEQUENCE_FALCHION_LOOKAT02 ); break;
					case SEQUENCE_DEFAULT_DRAW:
					case SEQUENCE_DEFAULT_IDLE1:
						break;
					default:
						m_nSequence--;
				}
			}
			else if ( !strcmp( szModel, "models/weapons/v_knife_push.mdl" ) ) {
				// Fix animations for the Shadow Daggers.
				switch ( m_nSequence ) {
					case SEQUENCE_DEFAULT_IDLE2:
						m_nSequence = SEQUENCE_DAGGERS_IDLE1; break;
					case SEQUENCE_DEFAULT_LIGHT_MISS1:
					case SEQUENCE_DEFAULT_LIGHT_MISS2:
						m_nSequence = RandomIntDef( SEQUENCE_DAGGERS_LIGHT_MISS1, SEQUENCE_DAGGERS_LIGHT_MISS5 ); break;
					case SEQUENCE_DEFAULT_HEAVY_MISS1:
						m_nSequence = RandomIntDef( SEQUENCE_DAGGERS_HEAVY_MISS2, SEQUENCE_DAGGERS_HEAVY_MISS1 ); break;
					case SEQUENCE_DEFAULT_HEAVY_HIT1:
					case SEQUENCE_DEFAULT_HEAVY_BACKSTAB:
					case SEQUENCE_DEFAULT_LOOKAT01:
						m_nSequence += 3; break;
					case SEQUENCE_DEFAULT_DRAW:
					case SEQUENCE_DEFAULT_IDLE1:
						break;
					default:
						m_nSequence += 2;
				}
			}
			else if ( !strcmp( szModel, "models/weapons/v_knife_survival_bowie.mdl" ) ) {
				// Fix animations for the Bowie Knife.
				switch ( m_nSequence ) {
					case SEQUENCE_DEFAULT_DRAW:
					case SEQUENCE_DEFAULT_IDLE1:
						break;
					case SEQUENCE_DEFAULT_IDLE2:
						m_nSequence = SEQUENCE_BOWIE_IDLE1; break;
					default:
						m_nSequence--;
				}
			}
			else if ( !strcmp( szModel, "models/weapons/v_knife_ursus.mdl" ) )
			{
				switch ( m_nSequence )
				{
					case SEQUENCE_DEFAULT_DRAW:
						m_nSequence = RandomIntDef( SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2 );
						break;
					case SEQUENCE_DEFAULT_LOOKAT01:
						m_nSequence = RandomIntDef( SEQUENCE_BUTTERFLY_LOOKAT01, 14 );
						break;
					default:
						m_nSequence++;
						break;
				}
			}
			else if ( !strcmp( szModel, "models/weapons/v_knife_stiletto.mdl" ) )
			{
				switch ( m_nSequence )
				{
					case SEQUENCE_DEFAULT_LOOKAT01:
						m_nSequence = RandomIntDef( 12, 13 );
						break;
				}
			}
			else if ( !strcmp( szModel, "models/weapons/v_knife_widowmaker.mdl" ) )
			{
				switch ( m_nSequence )
				{
					case SEQUENCE_DEFAULT_LOOKAT01:
						m_nSequence = RandomIntDef( 14, 15 );
						break;
				}
			}
			else if ( !strcmp( szModel, "models/weapons/v_knife_css.mdl" ) )
			{
				switch ( m_nSequence )
				{
					case SEQUENCE_DEFAULT_LOOKAT01:
						m_nSequence = 15;
						break;
				}
			}
			else if ( !strcmp( szModel, "models/weapons/v_knife_cord.mdl" ) ||
				!strcmp( szModel, "models/weapons/v_knife_canis.mdl" ) ||
				!strcmp( szModel, "models/weapons/v_knife_outdoor.mdl" ) ||
				!strcmp( szModel, "models/weapons/v_knife_skeleton.mdl" ) )
			{
				switch ( m_nSequence )
				{
					case SEQUENCE_DEFAULT_DRAW:
						m_nSequence = RandomIntDef( SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2 );
						break;
					case SEQUENCE_DEFAULT_LOOKAT01:
						m_nSequence = RandomIntDef( SEQUENCE_BUTTERFLY_LOOKAT01, 14 );
						break;
					default:
						m_nSequence++;
				}
			}
			// Set the fixed sequence.
			pData->m_Value.m_Int = m_nSequence;
		}
	}

	// Call original function with the modified data.
	SkinChanger->fnSequenceProxyFn( pData, pStruct, pOut );
}

void CSkinChanger::Hooked_RecvProxy_Viewmodel( CRecvProxyData* pData, void* pStruct, void* pOut )
{
	if (SkinChanger->knife_models.empty())
		return;

	if ( Cheat.LocalPlayer && Cheat.LocalPlayer->IsAlive() && config.skins.override_knife->get() )
	{
		CBaseCombatWeapon* active_weapon = Cheat.LocalPlayer->GetActiveWeapon();

		if (active_weapon) {
			CCSWeaponData* weapon_info = active_weapon->GetWeaponInfo();

			if (weapon_info && weapon_info->nWeaponType == WEAPONTYPE_KNIFE && active_weapon->m_iItemDefinitionIndex() != Taser)
			{
				pData->m_Value.m_Int = ModelInfoClient->GetModelIndex(SkinChanger->knife_models[config.skins.knife_model->get()].model_name.c_str());
			}
		}
	}

	SkinChanger->oRecvnModelIndex( pData, pStruct, pOut );
}

void CSkinChanger::FixViewModelSequence()
{
	ClientClass* pClass = Client->GetAllClasses( );
	while ( pClass )
	{
		const char* pszName = pClass->m_pRecvTable->m_pNetTableName;

		if ( !strcmp( pszName, "DT_BaseViewModel" ) ) {
			// Search for the 'm_nModelIndex' property.
			RecvTable* pClassTable = pClass->m_pRecvTable;

			for ( int nIndex = 0; nIndex < pClass->m_pRecvTable->m_nProps; nIndex++ ) {
				RecvProp* pProp = &( pClass->m_pRecvTable->m_pProps[ nIndex ] );

				if ( !pProp || strcmp( pProp->m_pVarName, "m_nSequence") )
					continue;

				// Store the original proxy function.
				fnSequenceProxyFn = ( RecvVarProxy_t )pProp->m_ProxyFn;

				// Replace the proxy function with our sequence changer.
				pProp->m_ProxyFn = ( RecvVarProxy_t )SetViewModelSequence;
			}
		}

		if ( !strcmp( pszName, "DT_BaseViewModel") )
		{
			for ( int i = 0; i < pClass->m_pRecvTable->m_nProps; i++ )
			{
				RecvProp* pProp = &( pClass->m_pRecvTable->m_pProps[ i ] );
				const char* name = pProp->m_pVarName;

				// Knives
				if ( !strcmp( name, "m_nModelIndex") )
				{
					oRecvnModelIndex = ( RecvVarProxy_t )pProp->m_ProxyFn;
					pProp->m_ProxyFn = ( RecvVarProxy_t )Hooked_RecvProxy_Viewmodel;
				}
			}
		}
		pClass = pClass->m_pNext;
	}
}

void CSkinChanger::AnimationUnHook( )
{
	for ( ClientClass* pClass = Client->GetAllClasses( ); pClass; pClass = pClass->m_pNext ) {
		if ( !strcmp( pClass->m_pNetworkName, "CBaseViewModel" ) ) {
			// Search for the 'm_nModelIndex' property.
			RecvTable* pClassTable = pClass->m_pRecvTable;

			for ( int nIndex = 0; nIndex < pClassTable->m_nProps; nIndex++ ) {
				RecvProp* pProp = &pClassTable->m_pProps[ nIndex ];

				if ( !pProp || strcmp( pProp->m_pVarName, "m_nSequence" ) )
					continue;

				// Replace the proxy function with our sequence changer.
				pProp->m_ProxyFn = fnSequenceProxyFn;

				break;
			}

			break;
		}
	}

	for ( ClientClass* pClass = Client->GetAllClasses( ); pClass; pClass = pClass->m_pNext ) {
		if ( !strcmp( pClass->m_pNetworkName, "CBaseViewModel" ) ) {
			// Search for the 'm_nModelIndex' property.
			RecvTable* pClassTable = pClass->m_pRecvTable;

			for ( int nIndex = 0; nIndex < pClassTable->m_nProps; nIndex++ ) {
				RecvProp* pProp = &pClassTable->m_pProps[ nIndex ];

				if ( !pProp || strcmp( pProp->m_pVarName, "m_nModelIndex" ) )
					continue;

				// Replace the proxy function with our sequence changer.
				pProp->m_ProxyFn = oRecvnModelIndex;

				break;
			}

			break;
		}
	}
}

bool CSkinChanger::LoadModel( const char* thisModelName )
{
	const auto CustomModel = NetworkStringTableContainer->FindTable("modelprecache");

	if ( CustomModel )
	{
		ModelInfoClient->GetModelIndex( thisModelName );
		int MdlNum = CustomModel->AddString(false, thisModelName);

		if ( MdlNum == NULL )
			return false;
	}
	return true;
}


void CSkinChanger::InitCustomModels()
{
	LoadModel( "models/player/custom_player/legacy/tm_jumpsuit_varianta.mdl" );
	LoadModel( "models/player/custom_player/legacy/tm_jumpsuit_variantb.mdl" );
	LoadModel( "models/player/custom_player/legacy/ctm_gign_varianta.mdl" );
	LoadModel("models/player/holiday/facemasks/facemask_dallas.mdl");
	LoadModel("models/player/holiday/facemasks/facemask_battlemask.mdl");
	LoadModel("models/player/holiday/facemasks/evil_clown.mdl");
	LoadModel("models/player/holiday/facemasks/facemask_anaglyph.mdl");
	LoadModel("models/player/holiday/facemasks/facemask_boar.mdl");
	LoadModel("models/player/holiday/facemasks/facemask_bunny.mdl");
	LoadModel("models/player/holiday/facemasks/facemask_bunny_gold.mdl");
	LoadModel("models/player/holiday/facemasks/facemask_chains.mdl");
	LoadModel("models/player/holiday/facemasks/facemask_chicken.mdl");
	LoadModel("models/player/holiday/facemasks/facemask_devil_plastic.mdl");
	LoadModel("models/player/holiday/facemasks/facemask_hoxton.mdl");
	LoadModel("models/player/holiday/facemasks/facemask_pumpkin.mdl");
	LoadModel("models/player/holiday/facemasks/facemask_samurai.mdl");
	LoadModel("models/player/holiday/facemasks/facemask_sheep_bloody.mdl");
	LoadModel("models/player/holiday/facemasks/facemask_sheep_gold.mdl");
	LoadModel("models/player/holiday/facemasks/facemask_sheep_model.mdl");
	LoadModel("models/player/holiday/facemasks/facemask_skull.mdl");
	LoadModel("models/player/holiday/facemasks/facemask_template.mdl");
	LoadModel("models/player/holiday/facemasks/facemask_wolf.mdl");
	LoadModel("models/player/holiday/facemasks/porcelain_doll.mdl");
}	
constexpr int mask_flags = 0x10000;

void CSkinChanger::UpdateSkins()
{
	EngineClient->ClientCmd_Unrestricted("record x;stop");
}

void CSkinChanger::MaskChanger()
{
	static UpdateAddonModelsFunc pUpdateAddonModels = reinterpret_cast<UpdateAddonModelsFunc>(Utils::PatternScan("client.dll", "55 8B EC 83 EC ? 53 8B D9 8D 45 ? 8B 08"));
	static auto currentMask = *reinterpret_cast<char***>(Utils::PatternScan("client.dll", "FF 35 ? ? ? ? FF 90 ? ? ? ? 8B 8F", 0x2));

	static int oldMask = -1;

	if (!Cheat.LocalPlayer)
		return;

	auto mask = mask_models[config.skins.mask_changer_models->get()];

	if (!LoadModel(default_mask) || !LoadModel(mask))
		return;

	if (config.skins.mask_changer->get())
	{
		Cheat.LocalPlayer->m_iAddonBits() |= mask_flags;

		if (oldMask != config.skins.mask_changer_models->get())
		{
			*currentMask = (char*)mask;
			pUpdateAddonModels(Cheat.LocalPlayer, true);
			oldMask = config.skins.mask_changer_models->get();
		}
	}
	else
	{
		if (Cheat.LocalPlayer->m_iAddonBits() & mask_flags)
			Cheat.LocalPlayer->m_iAddonBits() &= ~mask_flags;
	}
}

void CSkinChanger::AgentChanger( ) {
	static int originalIdx = 0;
	InitCustomModels( );

	if (!config.skins.override_agent->get()) {
		if (Cheat.LocalPlayer && originalIdx) {
			Cheat.LocalPlayer->SetModelIndex(originalIdx);
			originalIdx = 0;
		}
		return;
	}

	auto pLocal = Cheat.LocalPlayer;

	if ( !pLocal ) {
		originalIdx = 0;
		return;
	}

	if ( !originalIdx )
		originalIdx = pLocal->m_nModelIndex( );

	switch ( Cheat.LocalPlayer->m_iTeamNum() )
	{
		case 2:
			pLocal->SetModelIndex( ModelInfoClient->GetModelIndex( models_t[ config.skins.agent_model_t->get( ) ] ) );
			break;
		case 3:
			pLocal->SetModelIndex( ModelInfoClient->GetModelIndex( models_ct[ config.skins.agent_model_ct->get( ) ] ) );
			break;
	}
}

static void* GetWearableCreateFn()
{
	auto clazz = Client->GetAllClasses();

	while (strcmp(clazz->m_pNetworkName, "CEconWearable"))
		clazz = clazz->m_pNext;

	return clazz->m_pCreateFn;
}

void CSkinChanger::ApplyGlove(CAttributableItem* pGlove)
{
	pGlove->m_iItemDefinitionIndex() = GloveCT;

	pGlove->m_nFallbackPaintKit() = 0;
	pGlove->m_nFallbackSeed() = 0.0000001f;

	pGlove->m_OriginalOwnerXuidHigh() = -1;

	pGlove->m_nModelIndex() = ModelInfoClient->GetModelIndex("models/weapons/v_models/arms/glove_motorcycle/v_glove_motorcycle.mdl");
	pGlove->net_pre_data_update(0);
}

void CSkinChanger::Run( bool frame_end ) {
	if ( !Cheat.InGame || !Cheat.LocalPlayer || !config.skins.override_knife->get() )
		return;

	auto my_weapons = Cheat.LocalPlayer->m_hMyWeapons( );

	for ( size_t i = 0; my_weapons[ i ] != 0xFFFFFFFF; i++ ) {
		auto weapon = reinterpret_cast< CAttributableItem* >( EntityList->GetClientEntityFromHandle( my_weapons[ i ] ) );

		if ( !weapon )
			return;


		if ( weapon->GetClientClass( )->m_ClassID == C_KNIFE ) {
			ApplyKnifeModel( weapon, knife_models[ config.skins.knife_model->get( ) ].model_name.c_str( ) );
		}

		auto cs_weapon = reinterpret_cast<CAttributableItem*>(Cheat.LocalPlayer->GetActiveWeapon());

		cs_weapon->m_nFallbackPaintKit() = vecKits[config.skins.paint_kits->get()].id;

		player_info_t info;

		EngineClient->GetPlayerInfo(Cheat.LocalPlayer->EntIndex(), &info);

		// fix stattrak ownership.
		cs_weapon->m_iAccountID() = info.xuid_low;

		// fix stattrak in hud.
		if (cs_weapon->m_nFallbackStatTrak() >= 0)
			cs_weapon->m_iEntityQuality() = 9;

		// force use fallback values.
		cs_weapon->m_OriginalOwnerXuidHigh() = -1;
	}
}

CSkinChanger* SkinChanger = new CSkinChanger;