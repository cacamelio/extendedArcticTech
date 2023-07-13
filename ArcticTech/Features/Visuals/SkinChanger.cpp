#include "SkinChanger.h"


void CSkinChanger::LoadKnifeModels() {
	for (int i = 0; i < 1000; i++) { // last knife index is ~500 but this func is called once
		CCSWeaponData* wdata = WeaponSystem->GetWeaponData(i);

		if (!wdata || wdata->nWeaponType != WEAPONTYPE_KNIFE || strstr(wdata->szViewModel, "taser"))
			continue;

		knife_models.emplace_back(KnifeModel_t{ wdata->GetName(), wdata->szViewModel });
	}
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
	world_model_weapon->SetModelIndex(model_index + 1);

	return true;
}


void CSkinChanger::AgentChanger( ) {
	static constexpr std::array models {
		"models/player/custom_player/legacy/ctm_fbi_variantb.mdl",
		"models/player/custom_player/legacy/ctm_fbi_variantf.mdl",
		"models/player/custom_player/legacy/ctm_fbi_variantg.mdl",
		"models/player/custom_player/legacy/ctm_fbi_varianth.mdl",
		"models/player/custom_player/legacy/ctm_sas_variantf.mdl",
		"models/player/custom_player/legacy/ctm_st6_variante.mdl",
		"models/player/custom_player/legacy/ctm_st6_variantg.mdl",
		"models/player/custom_player/legacy/ctm_st6_varianti.mdl",
		"models/player/custom_player/legacy/ctm_st6_variantk.mdl",
		"models/player/custom_player/legacy/ctm_st6_variantm.mdl",
		"models/player/custom_player/legacy/tm_balkan_variantf.mdl",
		"models/player/custom_player/legacy/tm_balkan_variantg.mdl",
		"models/player/custom_player/legacy/tm_balkan_varianth.mdl",
		"models/player/custom_player/legacy/tm_balkan_varianti.mdl",
		"models/player/custom_player/legacy/tm_balkan_variantj.mdl",
		"models/player/custom_player/legacy/tm_leet_variantf.mdl",
		"models/player/custom_player/legacy/tm_leet_variantg.mdl",
		"models/player/custom_player/legacy/tm_leet_varianth.mdl",
		"models/player/custom_player/legacy/tm_leet_varianti.mdl",
		"models/player/custom_player/legacy/tm_phoenix_variantf.mdl",
		"models/player/custom_player/legacy/tm_phoenix_variantg.mdl",
		"models/player/custom_player/legacy/tm_phoenix_varianth.mdl"
	}; // TODO: fix this hardcode

	static int originalIdx = 0;

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

	pLocal->SetModelIndex(ModelInfoClient->GetModelIndex(models[config.skins.agent_model->get()]));
}

void CSkinChanger::Run() {
	if ( !Cheat.InGame || !Cheat.LocalPlayer || !config.skins.override_knife->get() )
		return;

	auto my_weapons = Cheat.LocalPlayer->m_hMyWeapons( );
	for ( size_t i = 0; my_weapons[ i ] != 0xFFFFFFFF; i++ ) {
		auto weapon = reinterpret_cast< CAttributableItem* >( EntityList->GetClientEntityFromHandle( my_weapons[ i ] ) );

		if ( !weapon )
			return;

		if ( weapon->GetClientClass( )->m_ClassID == C_KNIFE ) {
			ApplyKnifeModel(weapon, knife_models[config.skins.knife_model->get()].model_name.c_str());
		}
	}
}

CSkinChanger* SkinChanger = new CSkinChanger;