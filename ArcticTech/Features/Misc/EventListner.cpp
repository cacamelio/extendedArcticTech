#include "EventListner.h"

#include "../../SDK/Interfaces.h"
#include "../../SDK/Globals.h"
#include "../../Utils/Console.h"
#include "../RageBot/DoubleTap.h"
#include "../RageBot/AnimationSystem.h"
#include "../RageBot/LagCompensation.h"
#include "../Visuals/ESP.h"
#include "../ShotManager/ShotManager.h"

CEventListner* EventListner = new CEventListner;

static std::vector<const char*> s_RgisterEvents = {
	"player_hurt",
	"player_death",
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

			Resolver->OnHit(victim);

			if (!skip_hurt) {
				if (config.visuals.esp.damage_marker->get())
					ESP::AddDamageMarker(victim->m_vecOrigin() + Vector(0, 0, 80), event->GetInt("dmg_health"));
			}

			if (config.misc.miscellaneous.logs->get(0) && !skip_hurt) {
				Console->Log(std::format("hurt {}'s {} for {} damage ({} remaining)", victim->GetName(), GetHitgroupName(event->GetInt("hitgroup")), event->GetInt("dmg_health"), event->GetInt("health")));
			}
		}

		if (victim && victim->m_bDormant())
			ESPInfo[victim->EntIndex()].m_nHealth = event->GetInt("health");
	}
	else if (name == "player_death") {
		if (EngineClient->GetPlayerForUserID(event->GetInt("userid")) == EngineClient->GetLocalPlayer()) {
			ctx.reset();
			DoubleTap->target_tickbase_shift = 0;
			ctx.tickbase_shift = 0;
			Resolver->Reset();
		}

		Resolver->Reset((CBasePlayer*)EntityList->GetClientEntity(EngineClient->GetPlayerForUserID(event->GetInt("userid"))));
		AnimationSystem->InvalidateInterpolation(EngineClient->GetPlayerForUserID(event->GetInt("userid")));
	}
	else if (name == "round_start") {
		LagCompensation->Reset();
		Cheat.freezetime = true;
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