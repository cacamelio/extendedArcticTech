#include "EventListner.h"

#include "../../SDK/Interfaces.h"
#include "../../SDK/Globals.h"
#include "../../Utils/Console.h"
#include "../RageBot/Exploits.h"
#include "../RageBot/AnimationSystem.h"
#include "../RageBot/LagCompensation.h"
#include "../Visuals/ESP.h"
#include "../Visuals/Chams.h"
#include "../ShotManager/ShotManager.h"
#include "../Visuals/PreserveKillfeed.h"

CEventListner* EventListner = new CEventListner;

static std::vector<const char*> s_RgisterEvents = {
	"player_hurt",
	"player_death",
	"player_spawned",
	"player_disconnect",
	"bomb_planted",
	"bomb_defused",
	"bomb_begindefuse",
	"bomb_beginplant",
	"bomb_abortplant",
	"bomb_abortdefuse",
	"bomb_exploded",
	"round_start",
	"round_end",
	"item_purchase",
	"round_freeze_end",
	"bullet_impact",
	"item_equip"
};

void CEventListner::FireGameEvent(IGameEvent* event) {
	const std::string name = event->GetName();

	bool skip_hurt = ShotManager->OnEvent(event);

	if (name == "player_hurt") {
		CBasePlayer* victim = reinterpret_cast<CBasePlayer*>(EntityList->GetClientEntity(EngineClient->GetPlayerForUserID(event->GetInt("userid"))));

		if (EngineClient->GetPlayerForUserID(event->GetInt("attacker")) == EngineClient->GetLocalPlayer()) {
			if (config.visuals.esp.hitsound->get())
				EngineClient->ExecuteClientCmd("play buttons/arena_switch_press_02.wav");

			if (!skip_hurt) {
				if (config.visuals.esp.damage_marker->get())
					ESP::AddDamageMarker(victim->m_vecOrigin() + Vector(0, 0, 80), event->GetInt("dmg_health"));
			}

			if (config.misc.miscellaneous.logs->get(0) && !skip_hurt) {
				Console->Log(std::format("hurt {}'s {} for {} damage ({} remaining)", victim->GetName(), GetHitgroupName(event->GetInt("hitgroup")), event->GetInt("dmg_health"), event->GetInt("health")));
			}
		}

		if (victim && victim->m_bDormant()) {
			ESPInfo[victim->EntIndex()].m_nHealth = event->GetInt("health");
		}
	}
	else if (name == "player_death") {
		int userid = EngineClient->GetPlayerForUserID(event->GetInt("userid"));

		if (userid == EngineClient->GetLocalPlayer()) {
			ctx.reset();
			Exploits->target_tickbase_shift = 0;
			ctx.tickbase_shift = 0;
			config.ragebot.aimbot.peek_assist_keybind->set(false);
		}

		Resolver->Reset((CBasePlayer*)EntityList->GetClientEntity(userid));
		AnimationSystem->InvalidateInterpolation(userid);
	}
	else if (name == "round_start") {
		LagCompensation->Reset();
		Cheat.freezetime = true;

		std::string buy_command = "";
		if (config.misc.miscellaneous.auto_buy->get(0))
			buy_command += "buy awp; ";
		if (config.misc.miscellaneous.auto_buy->get(1))
			buy_command += "buy ssg08; ";
		if (config.misc.miscellaneous.auto_buy->get(2))
			buy_command += "buy scar20; buy g3sg1; ";
		if (config.misc.miscellaneous.auto_buy->get(3))
			buy_command += "buy deagle; buy revolver; ";
		if (config.misc.miscellaneous.auto_buy->get(4))
			buy_command += "buy fn57; buy tec9; ";
		if (config.misc.miscellaneous.auto_buy->get(5))
			buy_command += "buy taser; ";
		if (config.misc.miscellaneous.auto_buy->get(6))
			buy_command += "buy vesthelm; ";
		if (config.misc.miscellaneous.auto_buy->get(7))
			buy_command += "buy smokegrenade; ";
		if (config.misc.miscellaneous.auto_buy->get(8))
			buy_command += "buy molotov; buy incgrenade; ";
		if (config.misc.miscellaneous.auto_buy->get(9))
			buy_command += "buy hegrenade; ";
		if (config.misc.miscellaneous.auto_buy->get(10))
			buy_command += "buy flashbang; ";
		if (config.misc.miscellaneous.auto_buy->get(11))
			buy_command += "buy defuser; ";

		if (!buy_command.empty() && Cheat.LocalPlayer && Cheat.LocalPlayer->m_iAccount() >= 1000)
			EngineClient->ExecuteClientCmd(buy_command.c_str());

		KillFeed->ClearDeathNotice = true;

		for (int i = 0; i < ClientState->m_nMaxClients; i++)
			ESPInfo[i].m_nHealth = 100;
	}
	else if (name == "round_freeze_end") {
		Cheat.freezetime = false;
	}
	else if (name == "bullet_impact") {
		if (EngineClient->GetPlayerForUserID(event->GetInt("userid")) == EngineClient->GetLocalPlayer() && config.visuals.effects.server_impacts->get() && Cheat.LocalPlayer) {
			Vector pos = Vector(event->GetFloat("x"), event->GetFloat("y"), event->GetFloat("z"));
			Color col = config.visuals.effects.server_impacts_color->get();
			DebugOverlay->AddBoxOverlay(pos, Vector(-1, -1, -1), Vector(1, 1, 1), QAngle(), col.r, col.g, col.b, col.a, config.visuals.effects.impacts_duration->get());
		}
	}
	else if (name == "item_equip") {
		CBasePlayer* player = reinterpret_cast<CBasePlayer*>(EntityList->GetClientEntity(EngineClient->GetPlayerForUserID(event->GetInt("userid"))));

		if (player && player->m_bDormant()) {
			long new_weapon = event->GetInt("defindex");
			unsigned long* weapons = player->m_hMyWeapons();

			for (int i = 0; i < MAX_WEAPONS; i++) {
				unsigned long weapon_handle = weapons[i];
				CBaseCombatWeapon* weapon = reinterpret_cast<CBaseCombatWeapon*>(EntityList->GetClientEntityFromHandle(weapon_handle));

				if (!weapon)
					continue;

				if (weapon->m_iItemDefinitionIndex() == new_weapon) {
					player->m_hActiveWeapon() = weapon_handle;
					break;
				}
			}
		}
	}
	else if (name == "player_spawned") {
		CBasePlayer* player = reinterpret_cast<CBasePlayer*>(EntityList->GetClientEntity(EngineClient->GetPlayerForUserID(event->GetInt("userid"))));

		if (player)
			ESPInfo[player->EntIndex()].m_nHealth = 100;
	}
	else if (name == "player_disconnect") {
		Chams->RemoveShotChams(EngineClient->GetPlayerForUserID(event->GetInt("userid")));
	}
}

int CEventListner::GetEventDebugID() {
	return 42;
}

void CEventListner::Register() {
	m_iDebugId = 42;

	for (auto name : s_RgisterEvents) {
		GameEventManager->AddListener(this, name, false);
	}
}

void CEventListner::Unregister() {
	GameEventManager->RemoveListener(this);
}