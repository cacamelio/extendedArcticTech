#include "LagCompensation.h"
#include "AnimationSystem.h"
#include "../../SDK/Interfaces.h"
#include "../../SDK/Globals.h"
#include "../AntiAim/AntiAim.h"
#include "DoubleTap.h"
#include "../../Utils/Utils.h"
#include <algorithm>
#include "../../SDK/NetMessages.h"
#include "../Visuals/ESP.h"

LagRecord* CLagCompensation::BackupData(CBasePlayer* player) {
	LagRecord* record = new LagRecord;

	record->player = player;
	RecordDataIntoTrack(player, record);

	return record;
}

void CLagCompensation::RecordDataIntoTrack(CBasePlayer* player, LagRecord* record) {
	record->player = player;

	record->m_viewAngle = player->m_angEyeAngles();
	record->m_flSimulationTime = player->m_flSimulationTime();
	record->m_vecOrigin = player->m_vecOrigin();
	record->m_fFlags = player->m_fFlags();
	record->m_flCycle = player->m_flCycle();
	record->m_nSequence = player->m_nSequence();
	record->m_flDuckAmout = player->m_flDuckAmount();
	record->m_flDuckSpeed = player->m_flDuckSpeed();
	record->m_vecMaxs = player->m_vecMaxs();
	record->m_vecMins = player->m_vecMins();
	record->m_vecVelocity = player->m_vecVelocity();
	record->m_vecAbsAngles = player->GetAbsAngles();
	record->flPoseParamaters = player->m_flPoseParameter();

	if (!record->boneMatrixFilled)
		memcpy(record->boneMatrix, player->GetCachedBoneData().Base(), sizeof(matrix3x4_t) * player->GetCachedBoneData().Count());

	memcpy(record->animlayers, player->GetAnimlayers(), sizeof(AnimationLayer) * 13);
}

void CLagCompensation::BacktrackEntity(LagRecord* record) {
	CBasePlayer* player = record->player;

	float flSimulationTime = player->m_flSimulationTime();

	player->m_flSimulationTime() = record->m_flSimulationTime;
	player->m_vecOrigin() = record->m_vecOrigin;
	player->SetAbsOrigin(record->m_vecOrigin);
	player->m_fFlags() = record->m_fFlags;
	player->m_flCycle() = record->m_flCycle;
	player->m_nSequence() = record->m_nSequence;
	player->m_flDuckAmount() = record->m_flDuckAmout;
	player->m_flDuckSpeed() = record->m_flDuckSpeed;
	player->m_vecMins() = record->m_vecMins;
	player->m_vecMaxs() = record->m_vecMaxs;
	player->m_vecVelocity() = record->m_vecVelocity;
	player->m_flPoseParameter() = record->flPoseParamaters;
	player->SetAbsAngles(record->m_vecAbsAngles);

	if (record->rollMatrixFilled && abs(flSimulationTime - record->m_flSimulationTime) > 0.002f)
		memcpy(player->GetCachedBoneData().Base(), record->rollMatrix, player->GetCachedBoneData().Count() * sizeof(matrix3x4_t));
	else
		memcpy(player->GetCachedBoneData().Base(), record->boneMatrix, player->GetCachedBoneData().Count() * sizeof(matrix3x4_t));
}

void CLagCompensation::OnNetUpdate() {
	if (!Cheat.InGame)
		return;

	INetChannel* nc = ClientState->m_NetChannel;

	for (int i = 0; i < ClientState->m_nMaxClients; i++) {
		CBasePlayer* pl = (CBasePlayer*)EntityList->GetClientEntity(i);

		if (!pl || !pl->IsAlive() || pl == Cheat.LocalPlayer || pl->m_bDormant())
			continue;

		auto& records = lag_records[pl->EntIndex()];

		if (records.empty() || pl->m_flSimulationTime() != pl->m_flOldSimulationTime()) {
			LagRecord* prev_record = !records.empty() ? &records.back() : nullptr;
			LagRecord* new_record = &records.emplace_back();

			new_record->m_nChokedTicks = GlobalVars->tickcount - last_update_tick[i];
			last_update_tick[i] = GlobalVars->tickcount;

			AnimationSystem->UpdateAnimations(pl, new_record, records);
			RecordDataIntoTrack(pl, new_record);

			if (prev_record)
				new_record->breaking_lag_comp = (prev_record->m_vecOrigin - new_record->m_vecOrigin).LengthSqr() > 4096.f;

			new_record->shifting_tickbase = max_simulation_time[i] > new_record->m_flSimulationTime;

			if (new_record->m_flSimulationTime > max_simulation_time[i] || abs(max_simulation_time[i] - new_record->m_flSimulationTime) > 1.f)
				max_simulation_time[i] = new_record->m_flSimulationTime;

			if (config.visuals.esp.shared_esp->get() && nc) {
				if (config.visuals.esp.share_with_enemies->get() || !pl->IsTeammate()) {
					CCLCMsg_VoiceData_t msg;

					msg.has_bits() = VoiceData_Has::Xuid | VoiceData_Has::SectionNumber | VoiceData_Has::SequenceBytes | VoiceData_Has::UncompressedSampleOffset;

					msg.xuid_low() = NET_ARCTIC_CODE;
					*(char*)&msg.xuid_high() = i;
					char& flags = *(char*)(reinterpret_cast<uintptr_t>(&msg.xuid_high()) + 1);
					
					if (pl->m_bIsScoped())
						flags |= Shared_Scoped;
					if (ESPInfo[i].m_nFakeDuckTicks > 12)
						flags |= Shared_FakeDuck;
					if (new_record->shifting_tickbase)
						flags |= Shared_Exploiting;
					if (new_record->breaking_lag_comp)
						flags |= Shared_BreakLC;

					*(short*)(reinterpret_cast<uintptr_t>(&msg.xuid_high()) + 2) = pl->GetActiveWeapon() ? pl->GetActiveWeapon()->m_iItemDefinitionIndex() : 0;
					*(Vector*)(reinterpret_cast<uintptr_t>(&msg.sequence_bytes())) = new_record->m_vecOrigin;

					nc->SendNetMsg(&msg, false, true);
				}
			}
			while (records.size() > TIME_TO_TICKS(1)) {
				records.pop_front();
			}
		}

		INetChannelInfo* nci = EngineClient->GetNetChannelInfo();
		if (config.visuals.esp.show_server_hitboxes->get() && nci && nci->IsLoopback())
			pl->DrawServerHitboxes(GlobalVars->interval_per_tick, true);
	}
}

float CLagCompensation::GetLerpTime() {
	return cvars.cl_interp_ratio->GetFloat() * GlobalVars->interval_per_tick;
}

bool CLagCompensation::ValidRecord(LagRecord* record) {
	if (record->shifting_tickbase || record->breaking_lag_comp)
		return false;

	float correct = 0.0f;

	// Get true latency
	INetChannelInfo* nci = EngineClient->GetNetChannelInfo();
	if (nci)
	{
		// add network latency
		correct += nci->GetLatency(FLOW_OUTGOING) + nci->GetLatency(FLOW_INCOMING);
	}

	// NOTE:  do these computations in float time, not ticks, to avoid big roundoff error accumulations in the math
	// add view interpolation latency see C_BaseEntity::GetInterpolationAmount()
	correct += GetLerpTime();

	// check bounds [0,sv_maxunlag]
	correct = std::clamp(correct, 0.0f, cvars.sv_maxunlag->GetFloat());

	// correct tick send by player 
	float flTargetTime = record->m_flSimulationTime;

	int tick_base = Cheat.LocalPlayer->m_nTickBase() - ctx.tickbase_shift;

	// calculate difference between tick sent by player and our latency based tick
	float deltaTime = correct - (TICKS_TO_TIME(tick_base) - flTargetTime);

	return std::abs(deltaTime) < 0.2f;
}

void CLagCompensation::Reset() {
	for (int i = 0; i < lag_records.size(); i++) {
		lag_records[i].clear();
		max_simulation_time[i] = 0.f;
		last_update_tick[i] = 0;
	}
}

CLagCompensation* LagCompensation = new CLagCompensation;