#include "SkinChanger.h"

bool CSkinChanger::ApplyKnifeModel( attributable_item_t* weapon, const char* model) {

	auto local_player = Cheat.LocalPlayer;

	if ( !local_player )
		return false;

	auto viewmodel = reinterpret_cast< CBaseViewModel* >( EntityList->GetClientEntityFromHandle( local_player->view_model( ) ) );
	if ( !viewmodel )
		return false;

	auto h_view_model_weapon = viewmodel->m_hweapon( );
	if ( !h_view_model_weapon )
		return false;

	auto view_model_weapon = reinterpret_cast< attributable_item_t* >( EntityList->GetClientEntityFromHandle( h_view_model_weapon ) );
	if ( view_model_weapon != weapon )
		return false;

	viewmodel->model_index( ) = ModelInfoClient->GetModelIndex( model );

	return true;
}

void CSkinChanger::AgentChanger( EClientFrameStage stage )
{
	static int originalIdx = 0;

	auto pLocal = Cheat.LocalPlayer;

	if ( !pLocal ) {
		originalIdx = 0;
		return;
	}

	constexpr auto getModel = [ ] ( int team ) constexpr -> const char* {
		constexpr std::array models{
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
		};

		switch ( team ) {
			case 2: return static_cast< std::size_t >( config.skins.agent_model->get( ) ) < models.size( ) ? models[ config.skins.agent_model->get( ) ] : nullptr;
			case 3: return static_cast< std::size_t >( config.skins.agent_model->get( ) ) < models.size( ) ? models[ config.skins.agent_model->get( ) ] : nullptr;
			default: return nullptr;
		}
	};

	if ( const auto model = getModel( pLocal->m_iTeamNum( ) ) ) {
		if ( stage == EClientFrameStage::FRAME_RENDER_START )
			originalIdx = pLocal->m_nModelIndex( );

		const auto idx = stage == EClientFrameStage::FRAME_RENDER_END && originalIdx ? originalIdx : ModelInfoClient->GetModelIndex( model );

		pLocal->SetGloveModelIIndex( idx );
	}
}

void CSkinChanger::Run() {
	if ( !EngineClient->IsConnected( ) && !EngineClient->IsInGame( ) )
		return;

	auto local_player = Cheat.LocalPlayer;

	if ( !local_player )
		return;

	auto active_weapon = local_player->GetActiveWeapon( );
	if ( !active_weapon )
		return;

	const char* model_bayonet = "models/weapons/v_knife_bayonet.mdl";
	const char* model_m9 = "models/weapons/v_knife_m9_bay.mdl";
	const char* model_karambit = "models/weapons/v_knife_karam.mdl";
	const char* model_bowie = "models/weapons/v_knife_survival_bowie.mdl";
	const char* model_butterfly = "models/weapons/v_knife_butterfly.mdl";
	const char* model_falchion = "models/weapons/v_knife_falchion_advanced.mdl";
	const char* model_flip = "models/weapons/v_knife_flip.mdl";
	const char* model_gut = "models/weapons/v_knife_gut.mdl";
	const char* model_huntsman = "models/weapons/v_knife_tactical.mdl";
	const char* model_shadow_daggers = "models/weapons/v_knife_push.mdl";
	const char* model_navaja = "models/weapons/v_knife_gypsy_jackknife.mdl";
	const char* model_stiletto = "models/weapons/v_knife_stiletto.mdl";
	const char* model_talon = "models/weapons/v_knife_widowmaker.mdl";
	const char* model_ursus = "models/weapons/v_knife_ursus.mdl";

	int index_bayonet = ModelInfoClient->GetModelIndex( "models/weapons/v_knife_bayonet.mdl" );
	int index_m9 = ModelInfoClient->GetModelIndex( "models/weapons/v_knife_m9_bay.mdl" );
	int index_karambit = ModelInfoClient->GetModelIndex( "models/weapons/v_knife_karam.mdl" );
	int index_bowie = ModelInfoClient->GetModelIndex( "models/weapons/v_knife_survival_bowie.mdl" );
	int index_butterfly = ModelInfoClient->GetModelIndex( "models/weapons/v_knife_butterfly.mdl" );
	int index_falchion = ModelInfoClient->GetModelIndex( "models/weapons/v_knife_falchion_advanced.mdl" );
	int index_flip = ModelInfoClient->GetModelIndex( "models/weapons/v_knife_flip.mdl" );
	int index_gut = ModelInfoClient->GetModelIndex( "models/weapons/v_knife_gut.mdl" );
	int index_huntsman = ModelInfoClient->GetModelIndex( "models/weapons/v_knife_tactical.mdl" );
	int index_shadow_daggers = ModelInfoClient->GetModelIndex( "models/weapons/v_knife_push.mdl" );
	int index_navaja = ModelInfoClient->GetModelIndex( "models/weapons/v_knife_gypsy_jackknife.mdl" );
	int index_stiletto = ModelInfoClient->GetModelIndex( "models/weapons/v_knife_stiletto.mdl" );
	int index_talon = ModelInfoClient->GetModelIndex( "models/weapons/v_knife_widowmaker.mdl" );
	int index_ursus = ModelInfoClient->GetModelIndex( "models/weapons/v_knife_ursus.mdl" );

	auto my_weapons = local_player->m_hMyWeapons( );
	for ( size_t i = 0; my_weapons[ i ] != 0xFFFFFFFF; i++ ) {
		auto weapon = reinterpret_cast< attributable_item_t* >( EntityList->GetClientEntityFromHandle( my_weapons[ i ] ) );

		if ( !weapon )
			return;

		if ( active_weapon->GetClientClass( )->m_ClassID == C_KNIFE ) {
			switch ( config.skins.knife_model->get() ) {
				case 0:
					break;
				case 1:
					ApplyKnifeModel( weapon, model_bayonet );
					break;
				case 2:
					ApplyKnifeModel( weapon, model_m9 );
					break;
				case 3:
					ApplyKnifeModel( weapon, model_karambit );
					break;
				case 4:
					ApplyKnifeModel( weapon, model_bowie );
					break;
				case 5:
					ApplyKnifeModel( weapon, model_butterfly );
					break;
				case 6:
					ApplyKnifeModel( weapon, model_falchion );
					break;
				case 7:
					ApplyKnifeModel( weapon, model_flip );
					break;
				case 8:
					ApplyKnifeModel( weapon, model_gut );
					break;
				case 9:
					ApplyKnifeModel( weapon, model_huntsman );
					break;
				case 10:
					ApplyKnifeModel( weapon, model_shadow_daggers );
					break;
				case 11:
					ApplyKnifeModel( weapon, model_navaja );
					break;
				case 12:
					ApplyKnifeModel( weapon, model_stiletto );
					break;
				case 13:
					ApplyKnifeModel( weapon, model_talon );
					break;
				case 14:
					ApplyKnifeModel( weapon, model_ursus );
					break;
			}
		}
	}
}

CSkinChanger* SkinChanger = new CSkinChanger;