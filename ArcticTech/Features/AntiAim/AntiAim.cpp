#include "AntiAim.h"
#include "../RageBot/DoubleTap.h"
#include "../../SDK/Interfaces.h"
#include "../../SDK/Globals.h"
#include <algorithm>
#include "../Misc/Prediction.h"
#include "../RageBot/AutoWall.h"
#include "../../Utils/Console.h"

void CAntiAim::FakeLag() {
	static ConVar* sv_maxusrcmdprocessticks = CVar->FindVar("sv_maxusrcmdprocessticks");

	if (Cheat.LocalPlayer->m_MoveType() == MOVETYPE_NOCLIP || Cheat.LocalPlayer->m_fFlags() & FL_FROZEN || Cheat.freezetime)
		return;

	CBaseCombatWeapon* activeWeapon = Cheat.LocalPlayer->GetActiveWeapon();

	if (!activeWeapon)
		return;

	if (activeWeapon->IsGrenade() && EnginePrediction->m_fThrowTime > 0) {
		ctx.send_packet = true;
		return;
	}
	else if (ctx.cmd->buttons & IN_ATTACK && activeWeapon->ShootingWeapon() && activeWeapon->CanShoot()) {
		if (!config.antiaim.misc.fake_duck->get())
			ctx.send_packet = true;
		return;
	}

	int fakelagTicks = 0;
	int fakelagLimit = min(14, config.antiaim.fakelag.limit->get());
	int fakelagMin = 1;

	if (ctx.tickbase_shift > 0) {
		fakelagLimit = max((sv_maxusrcmdprocessticks->GetInt() - 2) - ctx.tickbase_shift, 0);
	}

	if (config.antiaim.misc.fake_duck->get())
		fakelagMin = fakelagLimit = 14;

	if (config.antiaim.fakelag.enabled->get()) {
		if (Cheat.LocalPlayer->m_vecVelocity().Length() < 50) {
			fakelagTicks = 2;
		}
		else {
			fakelagTicks = int(64.0f / (Cheat.LocalPlayer->m_vecVelocity().Length() * GlobalVars->interval_per_tick) + 1);
		}
	}

	fakelagMin = min(fakelagMin, fakelagLimit);

	if (config.antiaim.fakelag.variability->get() && config.antiaim.fakelag.enabled->get())
		fakelagTicks = std::clamp(fakelagTicks - abs(rand() % int(config.antiaim.fakelag.variability->get())), fakelagMin, fakelagLimit);
	else
		fakelagTicks = std::clamp(fakelagTicks, fakelagMin, fakelagLimit);
	
	if (ctx.teleported_last_tick)
		ctx.send_packet = true;
	else
		ctx.send_packet = ClientState->m_nChokedCommands >= fakelagTicks;

	static bool hasPeeked = false;

	if (ctx.is_peeking) {
		if (!hasPeeked) {
			hasPeeked = true;

			if (ClientState->m_nChokedCommands > 0)
				ctx.send_packet = true;
		}
	}
	else {
		hasPeeked = false;
	}
}

void CAntiAim::Angles() {
	desyncing = false;

	if (Cheat.LocalPlayer->m_MoveType() == MOVETYPE_NOCLIP || Cheat.LocalPlayer->m_MoveType() == MOVETYPE_LADDER || Cheat.LocalPlayer->m_fFlags() & FL_FROZEN || Cheat.freezetime)
		return;

	CBaseCombatWeapon* activeWeapon = Cheat.LocalPlayer->GetActiveWeapon();

	if (!activeWeapon)
		return;

	if (activeWeapon->IsGrenade() && EnginePrediction->m_fThrowTime > 0)
		return;

	if (!activeWeapon->IsGrenade() && (ctx.cmd->buttons & IN_ATTACK && Cheat.LocalPlayer->GetActiveWeapon()->CanShoot()))
		return;

	target = GetNearestTarget();

	if (!(ctx.cmd->buttons & IN_USE)) {
		switch (config.antiaim.angles.pitch->get()) {
		case 0:
			break;
		case 1:
			ctx.cmd->viewangles.pitch = 89;
			break;
		}

		float originalYaw = ctx.cmd->viewangles.yaw;

		switch (config.antiaim.angles.yaw->get()) {
		case 0:
			break;
		case 1:
			ctx.cmd->viewangles.yaw -= 180;
			break;
		case 2:
			ctx.cmd->viewangles.yaw = AtTargets();
			break;
		}

		if (manualAngleState) {
			ctx.cmd->viewangles.yaw = originalYaw + ((manualAngleState == 1) ? 90 : -90);
		}

		notModifiedYaw = ctx.cmd->viewangles.yaw;

		if (config.antiaim.angles.yaw_jitter->get() && !Cheat.LocalPlayer->m_bIsDefusing())
			ctx.cmd->viewangles.yaw += jitter ? -config.antiaim.angles.modifier_value->get() : config.antiaim.angles.modifier_value->get();
	}
	else {
		notModifiedYaw = ctx.cmd->viewangles.yaw;
	}

	Desync();
}

void CAntiAim::Desync() {
	if (!config.antiaim.angles.body_yaw->get() || DoubleTap->IsShifting() || Cheat.LocalPlayer->m_bIsDefusing())
		return;

	bool inverter = config.antiaim.angles.inverter->get();

	if (config.antiaim.angles.body_yaw_options->get(0) && !(ctx.cmd->buttons & IN_USE))
		inverter = jitter;

	if (config.antiaim.angles.body_yaw_options->get(3)) {
		int fs_side = DesyncFreestand();

		if (fs_side != 0)
			inverter = fs_side == -1;
	}

	float desyncAngle = 0.f;

	if (config.antiaim.angles.body_yaw_limit->get() < 58)
		desyncAngle = inverter ? -config.antiaim.angles.body_yaw_limit->get() : config.antiaim.angles.body_yaw_limit->get();
	else
		desyncAngle = inverter ? -120 : 120;

	if (!ctx.send_packet) {
		realAngle = Math::AngleNormalize(ctx.cmd->viewangles.yaw + std::clamp(desyncAngle, -Cheat.LocalPlayer->GetMaxDesyncDelta(), Cheat.LocalPlayer->GetMaxDesyncDelta()));

		ctx.cmd->viewangles.yaw += desyncAngle;
	}

	if (config.antiaim.angles.body_yaw_options->get(2) && ctx.send_packet && (!ctx.tickbase_shift || ctx.cmd->buttons & IN_DUCK || config.antiaim.misc.slow_walk->get() || !(Cheat.LocalPlayer->m_fFlags() & FL_ONGROUND && Cheat.LocalPlayer->m_vecVelocity().LengthSqr() > 400.f))) {
		ctx.cmd->viewangles.roll = desyncAngle < 0 ? 64: -64;
	}

	desyncing = true;

	if (ctx.local_velocity.LengthSqr() < 5.f)
		ctx.cmd->sidemove = ctx.cmd->buttons & IN_DUCK ? (ctx.cmd->tick_count % 2 == 0 ? -3.01f : 3.01f) : (ctx.cmd->tick_count % 2 == 0 ? -1.01f : 1.01f);
}

CBasePlayer* CAntiAim::GetNearestTarget() {
	float nearestFov = 360.f;
	Vector eyePos = Cheat.LocalPlayer->GetEyePosition();
	QAngle viewAngle; EngineClient->GetViewAngles(&viewAngle);
	CBasePlayer* nearestPlayer = nullptr;

	for (int i = 0; i < ClientState->m_nMaxClients; i++) {
		CBasePlayer* pl = (CBasePlayer*)EntityList->GetClientEntity(i);

		if (!pl)
			continue;

		if (!pl->IsPlayer() || pl->IsTeammate() || !pl->IsAlive())
			continue;

		if (GlobalVars->curtime - pl->m_flSimulationTime() > 5.f) // old dormant
			continue;

		QAngle angleToPlayer = Math::VectorAngles(pl->m_vecOrigin() - eyePos);

		float fov = Utils::GetFOV(viewAngle, angleToPlayer);

		if (fov < nearestFov) {
			nearestFov = fov;
			nearestPlayer = pl;
		}
	}

	return nearestPlayer;
}

float CAntiAim::AtTargets() {
	if (!target)
		return ctx.cmd->viewangles.yaw - 180.f;

	QAngle backwardAngle = Math::VectorAngles(Cheat.LocalPlayer->m_vecOrigin() - target->m_vecOrigin());
	return backwardAngle.yaw;
}

int CAntiAim::DesyncFreestand() {
	if (!target)
		return 0;

	Vector forward = (target->m_vecOrigin() - Cheat.LocalPlayer->m_vecOrigin()).Q_Normalized();
	Vector eyePos = Cheat.LocalPlayer->GetEyePosition();

	Vector right = Math::AngleVectors(QAngle(0, notModifiedYaw + 90.f, 0));

	Vector negPos = eyePos - right * 16.f;
	Vector posPos = eyePos + right * 16.f;

	CGameTrace negTrace = EngineTrace->TraceRay(negPos, negPos + forward * 100.f, MASK_SHOT_HULL | CONTENTS_GRATE, Cheat.LocalPlayer);
	CGameTrace posTrace = EngineTrace->TraceRay(posPos, posPos + forward * 100.f, MASK_SHOT_HULL | CONTENTS_GRATE, Cheat.LocalPlayer);

	if (negTrace.startsolid && posTrace.startsolid)
		return 0;
	else if (negTrace.startsolid)
		return -1;
	else if (posTrace.startsolid)
		return 1;

	if (negTrace.fraction == 1.f && posTrace.fraction == 1.f)
		return 0;

	return negTrace.fraction < posTrace.fraction ? -1 : 1;
}

void CAntiAim::SlowWalk() {
	if (!config.antiaim.misc.slow_walk->get() || !Cheat.LocalPlayer || Cheat.LocalPlayer->m_iHealth() == 0 || !(GetAsyncKeyState(VK_SHIFT) & 0x8000))
		return;

	CBaseCombatWeapon* weapon = Cheat.LocalPlayer->GetActiveWeapon();

	ctx.cmd->buttons &= ~IN_WALK;

	if (!weapon)
		return;

	CCSWeaponData* data = weapon->GetWeaponInfo();

	float maxSpeed = (Cheat.LocalPlayer->m_bIsScoped() ? data->flMaxSpeedAlt : data->flMaxSpeed) * 0.3f;

	float movespeed = Math::Q_sqrt(ctx.cmd->sidemove * ctx.cmd->sidemove + ctx.cmd->forwardmove * ctx.cmd->forwardmove);
	
	if (movespeed == 0)
		return;

	float modifier = maxSpeed / movespeed;

	ctx.cmd->sidemove = min(modifier * ctx.cmd->sidemove, 450);
	ctx.cmd->forwardmove = min(modifier * ctx.cmd->forwardmove, 450);
}

void CAntiAim::FakeDuck() {
	if (!Cheat.LocalPlayer || Cheat.LocalPlayer->m_iHealth() == 0 || !config.antiaim.misc.fake_duck->get())
		return;

	ctx.cmd->buttons |= IN_BULLRUSH;

	if (ClientState->GetChokedCommands() < 7)
		ctx.cmd->buttons &= ~IN_DUCK;
	else
		ctx.cmd->buttons |= IN_DUCK;
}

void CAntiAim::LegMovement() {
	if (!Cheat.LocalPlayer || Cheat.LocalPlayer->m_MoveType() == MOVETYPE_LADDER)
		return;

	ctx.cmd->buttons &= ~(IN_FORWARD | IN_BACK | IN_MOVERIGHT | IN_MOVELEFT);

	switch (config.antiaim.misc.leg_movement->get()) {
	case 0:
		if (ctx.cmd->forwardmove > 0)
			ctx.cmd->buttons |= IN_FORWARD;
		else
			ctx.cmd->buttons |= IN_BACK;
		if (ctx.cmd->sidemove > 0)
			ctx.cmd->buttons |= IN_MOVERIGHT;
		else
			ctx.cmd->buttons |= IN_MOVELEFT;
		break;
	case 1:
		if (ctx.cmd->forwardmove < 0)
			ctx.cmd->buttons |= IN_FORWARD;
		else
			ctx.cmd->buttons |= IN_BACK;
		if (ctx.cmd->sidemove < 0)
			ctx.cmd->buttons |= IN_MOVERIGHT;
		else
			ctx.cmd->buttons |= IN_MOVELEFT;
		break;
	default:
		break;
	}
}

bool CAntiAim::IsPeeking() {
	Vector velocity = Cheat.LocalPlayer->m_vecVelocity();

	if (velocity.LengthSqr() < 64.f)
		return false;

	Vector move_factor = velocity.Q_Normalized() * 21.f;

	matrix3x4_t backup_matrix[128];
	Cheat.LocalPlayer->CopyBones(backup_matrix);

	Vector backup_abs_orgin = Cheat.LocalPlayer->GetAbsOrigin();
	Vector backup_origin = Cheat.LocalPlayer->m_vecOrigin();

	Utils::MatrixMove(Cheat.LocalPlayer->GetCachedBoneData().Base(), Cheat.LocalPlayer->GetCachedBoneData().Count(), Cheat.LocalPlayer->GetAbsOrigin(), Cheat.LocalPlayer->GetAbsOrigin() + move_factor);
	Cheat.LocalPlayer->SetAbsOrigin(backup_abs_orgin + move_factor);
	Cheat.LocalPlayer->m_vecOrigin() += move_factor;
	Cheat.LocalPlayer->ForceBoneCache();

	Vector scan_points[] = {
		Cheat.LocalPlayer->GetHitboxCenter(HITBOX_HEAD),
		Cheat.LocalPlayer->GetHitboxCenter(HITBOX_LEFT_FOOT),
		Cheat.LocalPlayer->GetHitboxCenter(HITBOX_RIGHT_FOOT),
		Cheat.LocalPlayer->GetHitboxCenter(HITBOX_PELVIS),
	};

	CBasePlayer* nearest = GetNearestTarget();

	bool peeked = false;
	if (nearest) {
		FireBulletData_t data;

		Vector enemyShootPos = nearest->GetShootPosition();

		for (auto& point : scan_points) {
			if (AutoWall->FireBullet(nearest, enemyShootPos, point, data, Cheat.LocalPlayer) && data.damage >= 4.f) {
				peeked = true;
				break;
			}
		}
	}

	Cheat.LocalPlayer->SetAbsOrigin(backup_abs_orgin);
	Cheat.LocalPlayer->m_vecOrigin() = backup_origin;
	memcpy(Cheat.LocalPlayer->GetCachedBoneData().Base(), backup_matrix, sizeof(matrix3x4_t) * Cheat.LocalPlayer->GetCachedBoneData().Count());

	return peeked;
}

void CAntiAim::OnKeyPressed(WPARAM key) {
	if (key == config.antiaim.angles.manual_left->key) {
		if (manualAngleState == 0 || manualAngleState == 2)
			manualAngleState = 1;
		else if (manualAngleState == 1)
			manualAngleState = 0;
	}
	else if (key == config.antiaim.angles.manual_right->key) {
		if (manualAngleState == 0 || manualAngleState == 1)
			manualAngleState = 2;
		else if (manualAngleState == 2)
			manualAngleState = 0;
	}
}

CAntiAim* AntiAim = new CAntiAim();