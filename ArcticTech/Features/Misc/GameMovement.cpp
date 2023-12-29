#include "GameMovement.h"

#include <cmath>
#include <algorithm>

#include "../../SDK/Interfaces.h"
#include "../../SDK/Globals.h"
#include "../../Utils/Utils.h"
#include "../Visuals/GrenadePrediction.h"


CMovement* Movement = new CMovement;

__forceinline float GetYaw(float a1, float a2) {

	if (a1 != 0.f || a2 != 0.f)
		return atan2f(a2, a1) * 57.295776f;
	return 0.f;
}

void CMovement::Rotate(float angle) {
	float cos_rot = std::cos(DEG2RAD(angle));
	float sin_rot = std::sin(DEG2RAD(angle));

	float new_forwardmove = (cos_rot * ctx.cmd->forwardmove) + (sin_rot * ctx.cmd->sidemove);
	float new_sidemove = (sin_rot * ctx.cmd->forwardmove) + (cos_rot * ctx.cmd->sidemove);

	ctx.cmd->forwardmove = new_forwardmove;
	ctx.cmd->sidemove = new_sidemove;
}

void CMovement::AutoStrafe() {
	if (!Cheat.LocalPlayer || !config.misc.movement.auto_strafe->get() || !Cheat.InGame || Cheat.LocalPlayer->m_iHealth() == 0) return;
	if (Cheat.LocalPlayer->m_MoveType() == MOVETYPE_NOCLIP || Cheat.LocalPlayer->m_MoveType() == MOVETYPE_LADDER || ctx.cmd->buttons & IN_SPEED) return;
	if (Cheat.LocalPlayer->m_fFlags() & FL_ONGROUND) {
		if (ctx.cmd->buttons & IN_JUMP)
			last_strafe_tick = ctx.cmd->command_number;
		return;
	}

	QAngle new_angle = ctx.cmd->viewangles;

	float speed_sqr = ctx.local_velocity.Length2DSqr();
	float target_yaw = new_angle.yaw; // eye yaw at the moment
	float offset = 0.f;
	if (ctx.cmd->sidemove != 0.f || ctx.cmd->forwardmove != 0.f)
		offset = RAD2DEG(std::atan2(-ctx.cmd->sidemove, ctx.cmd->forwardmove));

	target_yaw += offset;
	target_yaw = Math::AngleNormalize(target_yaw);

	if (ctx.cmd->command_number - 1 != last_strafe_tick)
		last_yaw = target_yaw;

	float wish_side = 0.f;

	if (speed_sqr > 256.f) {
		float diff = Math::AngleDiff(target_yaw, last_yaw);
		float max_diff = 20.f - config.misc.movement.auto_strafe_smooth->get() * 0.01f * 17.f;

		if (ctx.cmd->buttons & IN_DUCK)
			max_diff *= 0.7f;

		float correct = std::clamp(diff, -max_diff, max_diff);

		new_angle.yaw = last_yaw + correct;
		wish_side = correct > 0.f ? -1.f : 1.f;

		last_yaw = Math::AngleNormalize(new_angle.yaw);

		if (abs(correct) < max_diff) {
			strafe_side = -strafe_side;

			float strafe_correct = min(90.f, RAD2DEG(std::asin(21.f / sqrt(speed_sqr)))) * strafe_side;
			new_angle.yaw += strafe_correct;
			wish_side = strafe_correct > 0.f ? 1.f : -1.f;
		}
	} else {
		last_yaw = GetYaw(ctx.local_velocity.x, ctx.local_velocity.y);
	}

	new_angle.Normalize();

	if (speed_sqr > 256.f) {
		ctx.cmd->sidemove = wish_side * 450.f;
		ctx.cmd->forwardmove = 0.f;
	} else {
		ctx.cmd->sidemove = 0.f;
		ctx.cmd->forwardmove = 450.f;
	}

	Rotate(Math::AngleDiff(new_angle.yaw, ctx.cmd->viewangles.yaw));

	last_strafe_tick = ctx.cmd->command_number;
}

void CMovement::CompensateThrowable() {
	if (config.misc.movement.super_toss->get() == 0)
		return;

	auto weapon = Cheat.LocalPlayer->GetActiveWeapon();

	if (!weapon->IsGrenade())
		return;

	CBaseGrenade* grenade = reinterpret_cast<CBaseGrenade*>(weapon);
	auto weaponData = weapon->GetWeaponInfo();

	if (!weaponData)
		return;

	if (Cheat.LocalPlayer->m_MoveType() == MOVETYPE_NOCLIP || Cheat.LocalPlayer->m_MoveType() == MOVETYPE_LADDER)
		return;

	if (config.misc.movement.super_toss->get() == 2 && !(Cheat.LocalPlayer->m_fFlags() & FL_ONGROUND))
		ctx.cmd->viewangles.pitch += CalculateThrowPitch(ctx.cmd->viewangles.pitch, ctx.local_velocity, weaponData->flThrowVelocity, grenade->m_flThrowStrength());

	QAngle vangle = ctx.cmd->viewangles;
	vangle.pitch -= (90.f - fabsf(vangle.pitch)) * 10.f / 90.f;
	Vector direction = Math::AngleVectors(vangle);

	Vector smoothed_velocity = (ctx.local_velocity + ctx.last_local_velocity) * 0.5f;

	Vector base_vel = direction * (std::clamp(weaponData->flThrowVelocity * 0.9f, 15.f, 750.f) * (grenade->m_flThrowStrength() * 0.7f + 0.3f));
	Vector curent_vel = ctx.local_velocity * 1.25f + base_vel;

	Vector target_vel = (base_vel + smoothed_velocity * 1.25f).Normalized();
	if (curent_vel.Dot(direction) > 0.f && config.misc.movement.super_toss->get() == 2)
		target_vel = direction;

	float throw_yaw = CalculateThrowYaw(target_vel, ctx.local_velocity, weaponData->flThrowVelocity, grenade->m_flThrowStrength());

	if (config.misc.movement.super_toss->get() == 2 || !(Cheat.LocalPlayer->m_fFlags() & FL_ONGROUND))
		ctx.cmd->viewangles.yaw = throw_yaw;
}

void CMovement::QuickStop() {
	if (!config.misc.movement.quick_stop->get())
		return;

	if (std::abs(ctx.cmd->forwardmove) + std::abs(ctx.cmd->sidemove) > 1.f)
		return;

	if (ctx.cmd->buttons & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT | IN_JUMP))
		return;

	if (Cheat.LocalPlayer->m_vecVelocity().Length2DSqr() < 256.f || !(Cheat.LocalPlayer->m_fFlags() & FL_ONGROUND))
		return;

	Vector vec_speed = Cheat.LocalPlayer->m_vecVelocity();
	QAngle direction = Math::VectorAngles(vec_speed);

	QAngle view; EngineClient->GetViewAngles(&view);
	direction.yaw = view.yaw - direction.yaw;
	direction.Normalize();

	Vector forward;
	Math::AngleVectors(direction, forward);

	Vector nigated_direction = forward * -std::clamp(vec_speed.Q_Length2D(), 0.f, 450.f) * 0.9f;

	ctx.cmd->sidemove = nigated_direction.y;
	ctx.cmd->forwardmove = nigated_direction.x;
}