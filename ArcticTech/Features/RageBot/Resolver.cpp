#include "Resolver.h"

#include <algorithm>
#include "../../SDK/Interfaces.h"
#include "../../SDK/Misc/CBasePlayer.h"
#include "LagCompensation.h"
#include "../../SDK/Globals.h"
#include "AnimationSystem.h"


CResolver* Resolver = new CResolver;

void CResolver::Reset(CBasePlayer* pl) {
	if (pl) {
		m_CachedRollAngle[pl->EntIndex()] = 0.f;
		brute_force_data[pl->EntIndex()].reset();
		return;
	}

	for (int i = 0; i < 64; ++i) {
		m_CachedRollAngle[i] = 0.f;
		brute_force_data[i].reset();
	}
}

float CResolver::GetRollAngle(CBasePlayer* player) {
	if (player->IsTeammate())
		return 0.f;

	return m_CachedRollAngle[player->EntIndex()];
}

void CResolver::SetRollAngle(CBasePlayer* player, float angle) {
	m_CachedRollAngle[player->EntIndex()] = angle;
	player->m_angEyeAngles().roll = angle;
	player->v_angle().roll = angle;
}

R_PlayerState CResolver::DetectPlayerState(CBasePlayer* player, AnimationLayer* animlayers) {
	if (!(player->m_fFlags() & FL_ONGROUND))
		return R_PlayerState::AIR;

	CCSGOPlayerAnimationState* animstate = player->GetAnimstate();

	if (player->m_vecVelocity().Length2DSqr() > 256.f && animstate->flWalkToRunTransition > 0.8f && animlayers[ANIMATION_LAYER_MOVEMENT_MOVE].m_flPlaybackRate > 0.0001f)
		return R_PlayerState::MOVING;

	return R_PlayerState::STANDING;
}

R_AntiAimType CResolver::DetectAntiAim(CBasePlayer* player, const std::deque<LagRecord>& records) {
	if (records.size() < 6)
		return R_AntiAimType::NONE;

	int jitteredRecords = 0;
	float avgDelta = 0.f;
	float prevEyeYaw = player->m_angEyeAngles().yaw;

	for (int i = records.size() - 2; i > records.size() - 5; i--) {
		const LagRecord* record = &records.at(i);
		float eyeYaw = record->m_angEyeAngles.yaw;

		float delta = std::abs(Math::AngleDiff(eyeYaw, prevEyeYaw));

		avgDelta += delta;

		float maxDeltaDiff = record->m_nChokedTicks > 2 ? 30 : 15;

		if (std::abs(delta - 60.f) < maxDeltaDiff && delta > 30.f)
			jitteredRecords++;

		prevEyeYaw = eyeYaw;
	}

	if (jitteredRecords > 2)
		return R_AntiAimType::JITTER;

	if (avgDelta * 0.5f < 30.f)
		return R_AntiAimType::STATIC;

	return R_AntiAimType::UNKNOWN;
}

void CResolver::SetupResolverLayers(CBasePlayer* player, LagRecord* record) {
	CCSGOPlayerAnimationState* animstate = player->GetAnimstate();

	float eyeYaw = player->m_angEyeAngles().yaw;

	// zero delta
	*animstate = record->unupdated_animstate;
	animstate->flGoalFeetYaw = Math::AngleNormalize(eyeYaw);

	player->UpdateAnimationState(animstate, record->m_angEyeAngles, true); // probably should use animstate->Update();
	memcpy(record->resolver_data.animlayers[0], player->GetAnimlayers(), sizeof(AnimationLayer) * 13);

	// positive delta
	*animstate = record->unupdated_animstate;
	animstate->flGoalFeetYaw = Math::AngleNormalize(eyeYaw + player->GetMaxDesyncDelta());

	player->UpdateAnimationState(animstate, record->m_angEyeAngles, true);
	memcpy(record->resolver_data.animlayers[1], player->GetAnimlayers(), sizeof(AnimationLayer) * 13);

	// negative delta
	*animstate = record->unupdated_animstate;
	animstate->flGoalFeetYaw = Math::AngleNormalize(eyeYaw - player->GetMaxDesyncDelta());

	player->UpdateAnimationState(animstate, record->m_angEyeAngles, true);
	memcpy(record->resolver_data.animlayers[2], player->GetAnimlayers(), sizeof(AnimationLayer) * 13);
}

void CResolver::DetectFreestand(CBasePlayer* player, LagRecord* record) {
	Vector eyePos = player->m_vecOrigin() + Vector(0, 0, 64 - player->m_flDuckAmount() * 16.f);

	Vector forward = (Cheat.LocalPlayer->m_vecOrigin() - player->m_vecOrigin()).Q_Normalized();

	float notModifiedYaw = player->m_angEyeAngles().yaw;

	if (record->prev_record) {
		notModifiedYaw += Math::AngleDiff(record->prev_record->m_angEyeAngles.yaw, notModifiedYaw) * 0.5f;
	}

	notModifiedYaw = Math::AngleNormalize(notModifiedYaw);

	Vector right = Math::AngleVectors(QAngle(0, notModifiedYaw + 90.f, 0));

	Vector negPos = eyePos - right * 16.f;
	Vector posPos = eyePos + right * 16.f;

	CTraceFilterWorldAndPropsOnly filter;
	Ray_t rayNeg(negPos, Cheat.LocalPlayer->GetShootPosition());
	Ray_t rayPos(posPos, Cheat.LocalPlayer->GetShootPosition());
	CGameTrace negTrace, posTrace;

	EngineTrace->TraceRay(rayNeg, MASK_SHOT_HULL | CONTENTS_GRATE, &filter, &negTrace);
	EngineTrace->TraceRay(rayPos, MASK_SHOT_HULL | CONTENTS_GRATE, &filter, &posTrace);

	if (negTrace.startsolid && posTrace.startsolid) {
		record->resolver_data.side = 0;
		record->resolver_data.resolver_type = ResolverType::NONE;
		return;
	}
	else if (negTrace.startsolid) {
		record->resolver_data.side = -1;
		record->resolver_data.resolver_type = ResolverType::FREESTAND;
		return;
	}
	else if (posTrace.startsolid) {
		record->resolver_data.side = 1;
		record->resolver_data.resolver_type = ResolverType::FREESTAND;
		return;
	}

	if (negTrace.fraction == 1.f && posTrace.fraction == 1.f) {
		record->resolver_data.side = 0;
		record->resolver_data.resolver_type = ResolverType::NONE;
		return;
	}
	record->resolver_data.side = negTrace.fraction < posTrace.fraction ? -1 : 1;
	record->resolver_data.resolver_type = ResolverType::FREESTAND;
}

void CResolver::Apply(LagRecord* record, bool use_roll) {
	if (record->resolver_data.side != 0)
		record->player->GetAnimstate()->flGoalFeetYaw = Math::NormalizeYaw(record->player->m_angEyeAngles().yaw + (record->player->GetMaxDesyncDelta() * record->resolver_data.side));
}

void CResolver::Run(CBasePlayer* player, LagRecord* record, std::deque<LagRecord>& records) {
	if (Cheat.freezetime || player->m_fFlags() & FL_FROZEN || !Cheat.LocalPlayer->IsAlive())
		return;

	LagRecord* prevRecord = record->prev_record;

	record->roll = 0;

	if (!record->m_nChokedTicks || player->m_bIsDefusing()) {
		record->resolver_data.side = 0;
		record->resolver_data.resolver_type = ResolverType::NONE;
		return;
	}

	record->resolver_data.player_state = DetectPlayerState(player, record->animlayers);
	record->resolver_data.antiaim_type = DetectAntiAim(player, records);

	SetupResolverLayers(player, record);

	record->resolver_data.resolver_type = ResolverType::NONE;

	record->resolver_data.delta_center = abs(record->animlayers[ANIMATION_LAYER_MOVEMENT_MOVE].m_flPlaybackRate - record->resolver_data.animlayers[0][ANIMATION_LAYER_MOVEMENT_MOVE].m_flPlaybackRate) * 1000.f;
	record->resolver_data.delta_positive = abs(record->animlayers[ANIMATION_LAYER_MOVEMENT_MOVE].m_flPlaybackRate - record->resolver_data.animlayers[1][ANIMATION_LAYER_MOVEMENT_MOVE].m_flPlaybackRate) * 1000.f;
	record->resolver_data.delta_negative = abs(record->animlayers[ANIMATION_LAYER_MOVEMENT_MOVE].m_flPlaybackRate - record->resolver_data.animlayers[2][ANIMATION_LAYER_MOVEMENT_MOVE].m_flPlaybackRate) * 1000.f;

	float flLastDelta = 1000.f;

	if (record->resolver_data.delta_center < flLastDelta) {
		record->resolver_data.resolver_type = ResolverType::NONE;
		record->resolver_data.side = 0;
		flLastDelta = record->resolver_data.delta_center;
	}

	if (record->resolver_data.delta_positive < flLastDelta) {
		record->resolver_data.resolver_type = ResolverType::ANIM;
		record->resolver_data.side = 1;
		flLastDelta = record->resolver_data.delta_positive;
	}

	if (record->resolver_data.delta_negative < flLastDelta) {
		record->resolver_data.resolver_type = ResolverType::ANIM;
		record->resolver_data.side = -1;
		flLastDelta = record->resolver_data.delta_negative;
	}

	if (player->m_vecVelocity().LengthSqr() < 64.f || 
		player->GetAnimstate()->flWalkToRunTransition < 0.8f || 
		record->resolver_data.resolver_type == ResolverType::NONE || 
		!(player->m_fFlags() & FL_ONGROUND)
		) 
	{
		if (record->resolver_data.antiaim_type == R_AntiAimType::JITTER && prevRecord) {
			float eyeYaw = player->m_angEyeAngles().yaw;
			float prevEyeYaw = prevRecord->m_angEyeAngles.yaw;
			float delta = Math::AngleDiff(eyeYaw, prevEyeYaw);

			if (delta > 0.f)
				record->resolver_data.side = -1;
			else
				record->resolver_data.side = 1;

			record->resolver_data.resolver_type = ResolverType::LOGIC;
		}
		else {
			DetectFreestand(player, record);
		}
	}

	BruteForceData_t* bf_data = &brute_force_data[player->EntIndex()];
	if (bf_data->use && record->resolver_data.antiaim_type != R_AntiAimType::JITTER && GlobalVars->realtime - bf_data->last_shot < 2.f) { // don't bruteforce if data is too old or player using jitter antiaim
		record->resolver_data.side = bf_data->current_side;
		record->resolver_data.resolver_type = ResolverType::BRUTEFORCE;
	}
	else {
		bf_data->use = false;
	}

	if (record->resolver_data.side != 0) {
		player->GetAnimstate()->flGoalFeetYaw = Math::NormalizeYaw(player->m_angEyeAngles().yaw + (player->GetMaxDesyncDelta() * record->resolver_data.side));
	}

	if (config.ragebot.aimbot.roll_resolver->get()) {
		if (record->resolver_data.side != 0)
			record->roll = config.ragebot.aimbot.roll_angle->get() * record->resolver_data.side;
		else
			record->roll = config.ragebot.aimbot.roll_angle->get();
	}
}

void CResolver::OnMiss(CBasePlayer* player, LagRecord* record) {
	BruteForceData_t* bf_data = &brute_force_data[player->EntIndex()];

	if (!bf_data->use) {
		bf_data->current_side = record->resolver_data.side == 0 ? 1 : -record->resolver_data.side;
	}
	else {
		bf_data->current_side = bf_data->current_side == 0 ? 1 : -bf_data->current_side;
	}

	bf_data->use = true;
	bf_data->last_shot = GlobalVars->realtime;
}

void CResolver::OnHit(CBasePlayer* player, LagRecord* record) {
	BruteForceData_t* bf_data = &brute_force_data[player->EntIndex()];

	bf_data->use = true;
	bf_data->current_side = record->resolver_data.side;
	bf_data->last_shot = GlobalVars->realtime;
}