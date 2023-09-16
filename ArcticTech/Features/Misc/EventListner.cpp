#include "EventListner.h"

#include "../../SDK/Interfaces.h"
#include "../../SDK/Globals.h"
#include "../../Utils/Console.h"
#include "../RageBot/Exploits.h"
#include "../RageBot/AnimationSystem.h"
#include "../RageBot/LagCompensation.h"
#include "../RageBot/Ragebot.h"
#include "../Visuals/ESP.h"
#include "../Visuals/Chams.h"
#include "../ShotManager/ShotManager.h"
#include "../Visuals/PreserveKillfeed.h"
#include "../Lua/Bridge/Bridge.h"
#include "AutoPeek.h"

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
			AutoPeek->returning = false;
			Ragebot->ClearTargets();
		}

		Resolver->Reset((CBasePlayer*)EntityList->GetClientEntity(userid));
		AnimationSystem->InvalidateInterpolation(userid);
		LagCompensation->Invalidate(userid);
		ESPInfo[userid].m_nHealth = 0;
	}
	else if (name == "round_start") {
		LagCompensation->Reset();
		ctx.should_buy = true;

		//Utils::ForceFullUpdate();

		KillFeed->ClearDeathNotice = true;

		for (int i = 0; i < ClientState->m_nMaxClients; i++) {
			CBasePlayer* pl = reinterpret_cast<CBasePlayer*>(EntityList->GetClientEntity(i));
			if (pl && pl->IsEnemy() && pl->m_lifeState() == LIFE_DEAD && pl->m_iTeamNum() != 1) {
				pl->m_iHealth() = 100;
				pl->m_lifeState() = LIFE_ALIVE;
			}
			ESPInfo[i].m_nHealth = 100;
		}
	}
	else if (name == "bullet_impact") {
		if (EngineClient->GetPlayerForUserID(event->GetInt("userid")) == EngineClient->GetLocalPlayer() && config.visuals.effects.server_impacts->get() && Cheat.LocalPlayer) {
			Vector pos = Vector(event->GetFloat("x"), event->GetFloat("y"), event->GetFloat("z"));
			Color col = config.visuals.effects.server_impacts_color->get();
			DebugOverlay->AddBoxOverlay(pos, Vector(-1, -1, -1), Vector(1, 1, 1), QAngle(), col.r, col.g, col.b, col.a, config.visuals.effects.impacts_duration->get());
		}
	}
	else if (name == "item_equip") {
		int pl_id = EngineClient->GetPlayerForUserID(event->GetInt("userid"));
		CBasePlayer* player = reinterpret_cast<CBasePlayer*>(EntityList->GetClientEntity(pl_id));

		if (player && player->m_bDormant()) {
			ESPInfo[pl_id].m_iActiveWeapon = event->GetInt("defindex");
		}
	}
	else if (name == "player_disconnect") {
		int id = EngineClient->GetPlayerForUserID(event->GetInt("userid"));
		Chams->RemoveShotChams(id);
		LagCompensation->Reset(id);
		ESPInfo[id].reset();
		AnimationSystem->InvalidateInterpolation(id);
	}

	for (auto& func : Lua->hooks.getHooks(LUA_GAMEEVENTS))
		func.func(event);
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