#include "ShotManager.h"

#include "../../SDK/Interfaces.h"
#include "../../SDK/Globals.h"
#include "../../Utils/Console.h"

#include "../RageBot/LagCompensation.h"
#include "../Visuals/ESP.h"


CShotManager* ShotManager = new CShotManager;

bool CShotManager::OnEvent(IGameEvent* event) {
	const std::string event_name(event->GetName());

	if (event_name == "player_hurt") {
		const int attacker = EngineClient->GetPlayerForUserID(event->GetInt("attacker"));
		CBasePlayer* player = reinterpret_cast<CBasePlayer*>(EntityList->GetClientEntity(EngineClient->GetPlayerForUserID(event->GetInt("userid"))));

		if (attacker != EngineClient->GetLocalPlayer())
			return false;

		RegisteredShot_t* shot = nullptr;

		for (auto it = m_RegisteredShots.rbegin(); it != m_RegisteredShots.rend(); it++) {
			if (it->acked)
				break;

			if (it->record->player == player)
				shot = &*it;
		}

		if (!shot)
			return false; // manual fire or miss

		shot->ack_tick = GlobalVars->tickcount;
		shot->damage = event->GetInt("dmg_health");
		shot->damagegroup = HitgroupToDamagegroup(event->GetInt("hitgroup"));

		return true;
	}
	else if (event_name == "bullet_impact") {
		const int userid = EngineClient->GetPlayerForUserID(event->GetInt("userid"));

		if (userid != EngineClient->GetLocalPlayer())
			return false;

		RegisteredShot_t* shot = nullptr;

		for (auto it = m_RegisteredShots.rbegin(); it != m_RegisteredShots.rend(); it++) {
			if (it->acked)
				break;

			shot = &*it;
		}

		if (!shot || shot->impacts.size() > 4)
			return false; // manual fire

		Vector point(event->GetFloat("x"), event->GetFloat("y"), event->GetFloat("z"));

		shot->impacts.emplace_back(point);
		shot->ack_tick = GlobalVars->tickcount;

		float min_distance = (shot->hit_point - shot->target_pos).LengthSqr();
		float cur_distance = (point - shot->target_pos).LengthSqr();

		if (cur_distance < min_distance)
			shot->hit_point = point;

		return false;
	}
	else if (event_name == "player_death") {
		const int userid = EngineClient->GetPlayerForUserID(event->GetInt("userid"));
		const int attacker = EngineClient->GetPlayerForUserID(event->GetInt("attacker"));

		if (userid == EngineClient->GetLocalPlayer()) {
			for (auto it = m_RegisteredShots.rbegin(); it != m_RegisteredShots.rend(); it++) {
				if (it->acked)
					break;

				it->acked = true;
				it->unregistered = true;
				it->death = true;

				Console->ArcticTag();
				Console->ColorPrint("missed shot due to ", Color(230, 230, 230));
				Console->ColorPrint("death\n", Color(255, 20, 20));
			}

			return false;
		}

		for (auto it = m_RegisteredShots.rbegin(); it != m_RegisteredShots.rend(); it++) {
			if (it->acked)
				break;

			if (it->record->player->EntIndex() == userid && attacker != EngineClient->GetLocalPlayer()) {
				it->ack_tick = GlobalVars->tickcount;
				it->player_death = true;
			}
		}
		return false;
	}
}

void CShotManager::OnNetUpdate() {
	for (auto it = m_RegisteredShots.rbegin(); it != m_RegisteredShots.rend(); it++) {
		if (it->acked)
			break;

		if (!it->ack_tick || it->impacts.size() == 0) { // dont recieved events yet or unregistered
			int max_register_delay = 24;
			INetChannelInfo* nci = EngineClient->GetNetChannelInfo();
			if (nci) {
				max_register_delay += TIME_TO_TICKS(nci->GetAvgLatency(FLOW_INCOMING) + nci->GetAvgLatency(FLOW_OUTGOING));
			}

			if (GlobalVars->tickcount - it->shot_tick > max_register_delay) {
				it->acked = true;
				it->unregistered = true;

				Console->ArcticTag();
				Console->ColorPrint("missed shot due to ", Color(230, 230, 230));
				Console->ColorPrint("unregistered\n", Color(255, 20, 20));
			}
			continue;
		}

		RegisteredShot_t* shot = &*it;

		shot->acked = true;

		Vector direction;
		
		const int total_impacts = shot->impacts.size();
		if (total_impacts > 1) { // we can correct get correct shoot pos & angle
			direction = shot->impacts[total_impacts - 1] - shot->impacts[total_impacts - 2]; // impacts should be sorted by distance

			shot->angle = Math::VectorAngles(direction);
			shot->shoot_pos = EngineTrace->ClosestPoint(shot->impacts[total_impacts - 1], shot->impacts[total_impacts - 2], shot->client_shoot_pos); // correct shoot pos by trace
		}
		else {
			shot->angle = Math::VectorAngles(shot->impacts.back() - shot->client_shoot_pos);
			shot->shoot_pos = shot->client_shoot_pos;

			direction = Math::AngleVectors(shot->client_angle);
		}

		shot->end_pos = shot->impacts.back();

		if (shot->player_death) {
			Console->Log("missed shot due to player death");
		}
		else {
			CBasePlayer* player = shot->record->player;
			LagRecord* backup_record = &LagCompensation->records(player->EntIndex()).back(); // just updated player, so latest record is correct

			LagCompensation->BacktrackEntity(shot->record);

			Console->ArcticTag();
			if (shot->damage > 0) {
				Console->ColorPrint(std::format("hit {}'s {}({}) for {}({}) ({} remaining) [mismatch: ", player->GetName(), GetDamagegroupName(shot->damagegroup), GetDamagegroupName(shot->wanted_damagegroup), shot->damage, shot->wanted_damage, player->m_iHealth()), Color(240, 240, 240));

				if (config.visuals.esp.hitmarker->get())
					ESP::AddHitmarker(shot->hit_point);

				if (config.visuals.esp.damage_marker->get())
					ESP::AddDamageMarker(shot->record->m_vecOrigin + Vector(0, 0, 80), shot->damage);

				if (shot->wanted_damagegroup != shot->damagegroup) {
					CGameTrace trace;
					Ray_t ray(shot->shoot_pos, shot->shoot_pos + direction * 8192);

					if (!EngineTrace->ClipRayToPlayer(ray, MASK_SHOT_HULL | CONTENTS_GRATE, player, &trace) || trace.hit_entity != player) {
						Console->ColorPrint("resolver", Color(200, 255, 0));
					}
					else {
						if (HitboxToDamagegroup(trace.hitgroup) == shot->wanted_damagegroup) {
							Console->ColorPrint("resolver", Color(200, 255, 0));
						}
						else {
							if ((shot->shoot_pos - shot->client_shoot_pos).LengthSqr() > 1.f) {
								Console->ColorPrint("pred. error", Color(255, 200, 0));
							}
							else {
								Console->ColorPrint("spread", Color(255, 200, 0));
							}
						}
					}
				}
				else {
					Console->ColorPrint("none", Color(255, 255, 255));
				}

				Console->ColorPrint("]\n", Color(240, 240, 240));
			}
			else {
				CGameTrace trace;
				Ray_t ray(shot->shoot_pos, shot->shoot_pos + direction * 8192);

				Console->ColorPrint("missed shot due to ", Color(240, 240, 240));

				if (!EngineTrace->ClipRayToPlayer(ray, MASK_SHOT_HULL | CONTENTS_GRATE, player, &trace) || trace.hit_entity != player) {
					if ((shot->shoot_pos - shot->client_shoot_pos).LengthSqr() > 1.f) {
						Console->ColorPrint("pred. error", Color(255, 200, 0));
						Console->ColorPrint(std::format(" [diff: {:.4f}]\n", (shot->shoot_pos - shot->client_shoot_pos).Q_Length()), Color(240, 240, 240));
					}
					else {
						Console->ColorPrint("spread\n", Color(255, 200, 0));
					}
				}
				else {
					if ((shot->hit_point - shot->target_pos).LengthSqr() < 36.f) {
						Console->ColorPrint("damage rejection\n", Color(255, 20, 20)); // correct naming: sin shluhi s gmom
					}
					else {
						Console->ColorPrint("resolver\n", Color(200, 255, 0));

						Resolver->OnMiss(player, shot->record);
					}
				}
			}

			LagCompensation->BacktrackEntity(backup_record);
		}
	}
}

void CShotManager::AddShot(const Vector& shoot_pos, const Vector& target_pos, int damage, int damagegroup, int hitchance, LagRecord* record) {
	RegisteredShot_t* shot = &m_RegisteredShots.emplace_back();

	shot->client_shoot_pos = shoot_pos;
	shot->target_pos = target_pos;
	shot->client_angle = Math::VectorAngles(target_pos - shoot_pos);
	shot->shot_tick = GlobalVars->tickcount;
	shot->hitchance = hitchance;
	shot->backtrack = TIME_TO_TICKS(record->player->m_flSimulationTime() - record->m_flSimulationTime);
	shot->record = record;
	shot->wanted_damage = damage;
	shot->wanted_damagegroup = damagegroup;
}

void CShotManager::Reset() {
	m_RegisteredShots.clear();
}