#include "AnimationSystem.h"
#include "LagCompensation.h"
#include "../AntiAim/AntiAim.h"
#include "Resolver.h"
#include "../Lua/Bridge/Bridge.h"
#include "../Misc/Prediction.h"

void CAnimationSystem::StoreLocalAnims() {
	auto animstate = Cheat.LocalPlayer->GetAnimstate();

	stored_local_anims.bOnGround = animstate->bOnGround;
	stored_local_anims.flAffectedFraction = animstate->flAffectedFraction;
	stored_local_anims.flDurationInAir = animstate->flDurationInAir;
	stored_local_anims.flEyePitch = animstate->flEyePitch;
	stored_local_anims.flEyeYaw = animstate->flEyeYaw;
	stored_local_anims.flGoalFeetYaw = animstate->flGoalFeetYaw;
	stored_local_anims.flJumpFallVelocity = animstate->flJumpFallVelocity;
	stored_local_anims.flLeanAmount = animstate->flLeanAmount;
	stored_local_anims.flMoveYaw = animstate->flMoveYaw;
	stored_local_anims.poseparams = Cheat.LocalPlayer->m_flPoseParameter();
	stored_local_anims.filled = true;
}

void CAnimationSystem::RestoreLocalAnims() {
	if (!stored_local_anims.filled)
		return;

	auto animstate = Cheat.LocalPlayer->GetAnimstate();

	animstate->bOnGround = stored_local_anims.bOnGround;
	animstate->flAffectedFraction = stored_local_anims.flAffectedFraction;
	animstate->flDurationInAir = stored_local_anims.flDurationInAir;
	animstate->flEyePitch = stored_local_anims.flEyePitch;
	animstate->flEyeYaw = stored_local_anims.flEyeYaw;
	animstate->flGoalFeetYaw = stored_local_anims.flGoalFeetYaw;
	animstate->flJumpFallVelocity = stored_local_anims.flJumpFallVelocity;
	animstate->flLeanAmount = stored_local_anims.flLeanAmount;
	animstate->flMoveYaw = stored_local_anims.flMoveYaw;
	Cheat.LocalPlayer->m_flPoseParameter() = stored_local_anims.poseparams;
	Cheat.LocalPlayer->SetAbsAngles(QAngle(0, stored_local_anims.flGoalFeetYaw, 0));
}

void CAnimationSystem::CorrectLocalMatrix(matrix3x4_t* mat, int size) {
	Utils::MatrixMove(mat, size, sent_abs_origin, Cheat.LocalPlayer->GetAbsOrigin());
}

void CAnimationSystem::OnCreateMove() {
	CCSGOPlayerAnimationState* animstate = Cheat.LocalPlayer->GetAnimstate();

	//if (!(Cheat.LocalPlayer->m_fFlags() & FL_ONGROUND)) {
	//	const float jumpImpulse = cvars.sv_jump_impulse->GetFloat();
	//	const float gravity = cvars.sv_gravity->GetFloat();
	//	const float speed = Cheat.LocalPlayer->m_flFallVelocity();

	//	animstate->flDurationInAir = (jumpImpulse - speed) / gravity;
	//}

	for (auto cb : Lua->hooks.getHooks(LUA_PRE_ANIMUPDATE))
		cb.func(Cheat.LocalPlayer);

	Cheat.LocalPlayer->UpdateAnimationState(animstate, ctx.cmd->viewangles);

	if (ctx.send_packet) {
		//if (AntiAim->desyncing)
		//	animstate->flGoalFeetYaw = AntiAim->realAngle;

		//float maxDelta = Cheat.LocalPlayer->GetMaxDesyncDelta();
		//float goalFeetYawDelta = std::clamp(Math::AngleDiff(animstate->flEyeYaw, animstate->flGoalFeetYaw), -maxDelta, maxDelta);

		//animstate->flGoalFeetYaw = Math::AngleNormalize(animstate->flEyeYaw - goalFeetYawDelta);

		//Cheat.LocalPlayer->m_flPoseParameter()[BODY_YAW] = 0.5f + (goalFeetYawDelta / 120.f);

		Cheat.LocalPlayer->SetAbsAngles(QAngle(0, animstate->flGoalFeetYaw, 0));
		sent_abs_origin = Cheat.LocalPlayer->GetAbsOrigin();

		for (auto cb : Lua->hooks.getHooks(LUA_POST_ANIMUPDATE))
			cb.func(Cheat.LocalPlayer);

		//Cheat.LocalPlayer->m_BoneAccessor().m_ReadableBones = 0;
		//Cheat.LocalPlayer->m_BoneAccessor().m_WritableBones = 0;

		//Cheat.LocalPlayer->m_nOcclusionFrame() = 0;
		//Cheat.LocalPlayer->m_nOcclusionFlags() = 0;

		BuildMatrix(Cheat.LocalPlayer, sent_matrix, 128, BONE_USED_BY_ANYTHING, nullptr);

		Cheat.holdLocalAngles = false;
	}
}

void CAnimationSystem::UpdateLocalAnimations() {
	static int last_update_tick = 0;
	static float old_sim_time = 0.f;

	CCSGOPlayerAnimationState* animstate = Cheat.LocalPlayer->GetAnimstate();
	animstate->iLastUpdateFrame = GlobalVars->framecount;
}

void CAnimationSystem::FrameStageNotify(EClientFrameStage stage) {
	switch (stage)
	{
	case FRAME_NET_UPDATE_POSTDATAUPDATE_END:
		for (int i = 0; i < ClientState->m_nMaxClients; ++i) {
			CBasePlayer* player = reinterpret_cast<CBasePlayer*>(EntityList->GetClientEntity(i));

			if (player && player != Cheat.LocalPlayer)
				DisableInterpolationFlags(player);
		}
		break;
	case FRAME_NET_UPDATE_END:
		break;
	case FRAME_RENDER_START:
		if (Cheat.LocalPlayer && Cheat.LocalPlayer->IsAlive())
			UpdateLocalAnimations();
		break;
	default:
		break;
	}
}

void CAnimationSystem::BuildMatrix(CBasePlayer* player, matrix3x4_t* boneToWorld, int maxBones, int mask, AnimationLayer* animlayers) {
	hook_info.setup_bones = true;

	player->InvalidatePhysicsRecursive(ANIMATION_CHANGED);
	player->InvalidateBoneCache();

	if (animlayers != nullptr)
		memcpy(player->GetAnimlayers(), animlayers, sizeof(AnimationLayer) * 13);

	bool backupMaintainSequenceTransitions = player->m_bMaintainSequenceTransitions();
	int backupEffects = player->m_fEffects();

	player->m_fEffects() |= EF_NOINTERP;
	player->m_bMaintainSequenceTransitions() = false;

	player->SetupBones(boneToWorld, maxBones, mask, player->m_flSimulationTime());

	player->m_fEffects() = backupEffects;
	player->m_bMaintainSequenceTransitions() = backupMaintainSequenceTransitions;

	hook_info.setup_bones = false;
}

void CAnimationSystem::DisableInterpolationFlags(CBasePlayer* player) {
	auto& var_mapping = player->m_VarMapping();

	for (int i = 0; i < var_mapping.m_nInterpolatedEntries; ++i)
		var_mapping.m_Entries[i].m_bNeedsToInterpolate = false;
}

void CAnimationSystem::UpdateAnimations(CBasePlayer* player, LagRecord* record, std::deque<LagRecord>& records) {
	CCSGOPlayerAnimationState* animstate = player->GetAnimstate();
	const int idx = player->EntIndex();

	if (!animstate)
		return;
	
	record->player = player;
	record->unupdated_animstate = *animstate;

	auto backupRealtime = GlobalVars->realtime;
	auto backupCurtime = GlobalVars->curtime;
	auto backupFrametime = GlobalVars->frametime;
	auto backupAbsFrametime = GlobalVars->absoluteframetime;
	auto backupInterp = GlobalVars->interpolation_amount;
	auto backupTickcount = GlobalVars->tickcount;
	auto backupFramecount = GlobalVars->framecount;
	auto backupAbsOrigin = player->GetAbsOrigin();
	auto backupAbsVelocity = player->m_vecAbsVelocity();
	auto backupLBY = player->m_flLowerBodyYawTarget();
	auto nOcclusionMask = player->m_nOcclusionFlags();
	auto nOcclusionFrame = player->m_nOcclusionFrame();
	auto backupAbsAngles = player->GetAbsAngles();

	GlobalVars->realtime = player->m_flSimulationTime();
	GlobalVars->curtime = player->m_flSimulationTime();
	GlobalVars->frametime = GlobalVars->interval_per_tick;
	GlobalVars->absoluteframetime = GlobalVars->interval_per_tick;
	GlobalVars->interpolation_amount = 0.f;
	GlobalVars->tickcount = TIME_TO_TICKS(player->m_flSimulationTime());
	GlobalVars->framecount = TIME_TO_TICKS(player->m_flSimulationTime());

	memcpy(record->animlayers, player->GetAnimlayers(), 13 * sizeof(AnimationLayer));
	record->animlayers[12].m_flWeight = 0.f;
	auto pose_params = player->m_flPoseParameter();

	player->m_iEFlags() &= ~(EFL_DIRTY_ABSTRANSFORM | EFL_DIRTY_ABSVELOCITY);

	player->m_BoneAccessor().m_ReadableBones = 0;
	player->m_BoneAccessor().m_WritableBones = 0;

	player->m_nOcclusionFrame() = 0;
	player->m_nOcclusionFlags() = 0;

	player->SetAbsVelocity(player->m_vecVelocity());
	player->SetAbsOrigin(player->m_vecOrigin());

	if (!player->IsTeammate()) {
		Resolver->Run(player, record, records);

		*animstate = record->unupdated_animstate;
		player->UpdateClientSideAnimation();

		Resolver->Apply(record);
	}
	else {
		player->UpdateClientSideAnimation();
	}

	if (player->m_fFlags() & FL_ONGROUND) {
		animstate->flDurationInAir = 0;
	}
	else {
		animstate->flDurationInAir = (cvars.sv_jump_impulse->GetFloat() - player->m_flFallVelocity()) / cvars.sv_gravity->GetFloat();
	}

	hook_info.disable_clamp_bones = true;
	BuildMatrix(player, record->aim_matrix, 128, BONE_USED_BY_ANYTHING, record->animlayers);
	hook_info.disable_clamp_bones = false;

	interpolate_data_t* lerp_data = &interpolate_data[idx];

	lerp_data->net_origin = player->m_vecOrigin();

	memcpy(record->bone_matrix, record->aim_matrix, sizeof(matrix3x4_t) * 128);
	player->ClampBonesInBBox(record->bone_matrix, BONE_USED_BY_ANYTHING);
	memcpy(lerp_data->original_matrix, record->bone_matrix, sizeof(matrix3x4_t) * 128);

	record->bone_matrix_filled = true;

	if (player->IsEnemy()) {
		float deltaOriginal = Math::AngleDiff(animstate->flEyeYaw, animstate->flGoalFeetYaw);
		float eyeYawNew = Math::AngleNormalize(animstate->flEyeYaw + deltaOriginal);
		player->SetAbsAngles(QAngle(0, eyeYawNew, 0));
		player->m_flPoseParameter()[BODY_YAW] = 1.f - player->m_flPoseParameter()[BODY_YAW]; // opposite side
		Resolver->SetRollAngle(player, 0.f);
		BuildMatrix(player, record->opposite_matrix, 128, BONE_USED_BY_HITBOX, record->animlayers);
	}

	player->m_nOcclusionFrame() = nOcclusionFrame;
	player->m_nOcclusionFlags() = nOcclusionMask;

	player->SetAbsOrigin(backupAbsOrigin);
	player->m_vecAbsVelocity() = backupAbsVelocity;

	GlobalVars->realtime = backupRealtime;
	GlobalVars->curtime = backupCurtime;
	GlobalVars->frametime = backupFrametime;
	GlobalVars->absoluteframetime = backupAbsFrametime;
	GlobalVars->interpolation_amount = backupInterp;
	GlobalVars->tickcount = backupTickcount;
	GlobalVars->framecount = backupFramecount;
	
	player->SetAbsAngles(backupAbsAngles);
	player->m_flLowerBodyYawTarget() = backupLBY;
	memcpy(player->GetAnimlayers(), record->animlayers, sizeof(AnimationLayer) * 13);
	memcpy(player->GetCachedBoneData().Base(), record->bone_matrix, sizeof(matrix3x4_t) * player->GetCachedBoneData().Count());
	player->m_flPoseParameter() = pose_params;

	player->InvalidatePhysicsRecursive(ANIMATION_CHANGED);
}

Vector CAnimationSystem::GetInterpolated(CBasePlayer* player) {
	return interpolate_data[player->EntIndex()].origin;
}

void CAnimationSystem::RunInterpolation() {
	for (int i = 0; i < 64; i++) {
		CBasePlayer* player = reinterpret_cast<CBasePlayer*>(EntityList->GetClientEntity(i));

		if (!player || player == Cheat.LocalPlayer || !player->IsAlive() || player->m_bDormant())
			continue;

		interpolate_data_t* data = &interpolate_data[i];

		if ((data->net_origin - data->origin).LengthSqr() > 8192) {
			data->origin = data->net_origin;
			data->valid = false;
			continue;
		}

		data->valid = true;
		data->origin += (data->net_origin - data->origin) * std::clamp(GlobalVars->frametime * 32, 0.f, 0.8f);
	}
}

void CAnimationSystem::InterpolateModel(CBasePlayer* player, matrix3x4_t* matrix) {
	if (player == Cheat.LocalPlayer)
		return;

	interpolate_data_t* data = &interpolate_data[player->EntIndex()];

	if (!data->valid)
		return;

	Utils::MatrixMove(data->original_matrix, matrix, player->GetCachedBoneData().Count(), data->net_origin, data->origin);
}

void CAnimationSystem::ResetInterpolation() {
	for (int i = 0; i < 64; i++)
		interpolate_data[i].valid = false;
}

void CAnimationSystem::InvalidateInterpolation(int i) {
	interpolate_data[i].valid = false;
}

CAnimationSystem* AnimationSystem = new CAnimationSystem;