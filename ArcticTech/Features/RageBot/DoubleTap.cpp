#include "DoubleTap.h"
#include "../Misc/Prediction.h"
#include "../../SDK/Globals.h"
#include "../AntiAim/AntiAim.h"

bool CDoubleTap::ShouldCharge() {
	if (GlobalVars->realtime - Cheat.LocalPlayer->m_flSpawnTime() < 0.2f)
		return false;

	return ctx.tickbase_shift < target_tickbase_shift;
}

void CDoubleTap::Run() {
	if (!ctx.cmd)
		return;

	CBaseCombatWeapon* weapon = Cheat.LocalPlayer->GetActiveWeapon();

	if (!weapon)
		return;

	if (teleport_next_tick) {
		if ((weapon->m_iItemDefinitionIndex() == Ssg08 || weapon->m_iItemDefinitionIndex() == Awp) && defensive_ticks > 2 && !config.ragebot.aimbot.doubletap_options->get(2))
			return;

		teleport_next_tick = false;
		target_tickbase_shift = 0;
		last_teleport_time = GlobalVars->realtime;
		return;
	}

	if (!config.ragebot.aimbot.doubletap->get() || config.antiaim.misc.fake_duck->get() || config.ragebot.aimbot.force_teleport->get()) {
		target_tickbase_shift = 0;
		return;
	}

	const int max_tickbase_charge = MaxTickbaseShift();

	if (target_tickbase_shift < max_tickbase_charge) {
		if (!(weapon->IsGrenade() || GlobalVars->realtime - last_teleport_time < 0.3f || GetAsyncKeyState(VK_LBUTTON) & 0x8000 || block_charge || !ctx.send_packet)) {
			target_tickbase_shift = max_tickbase_charge;
			Cheat.tickbaseshift = max_tickbase_charge;
			charged_command = ctx.cmd->command_number;
			ctx.shifted_last_tick = 0;
		}
	}
	else if (target_tickbase_shift > max_tickbase_charge) {
		target_tickbase_shift = max_tickbase_charge;
		Cheat.tickbaseshift = max_tickbase_charge;
	}

	block_charge = false;
}

void CDoubleTap::HandleTeleport(CL_Move_t cl_move, float extra_samples) {
	shifting_tickbase = true;

	for (; ctx.tickbase_shift > target_tickbase_shift; --ctx.tickbase_shift) {
		cl_move(extra_samples, ctx.tickbase_shift == target_tickbase_shift); // bFinalTick does nothing to be honest
	}

	shifting_tickbase = false;
}

int CDoubleTap::MaxTickbaseShift() {
	return 13;


	// fuck this shit 13 always on

	//CBaseCombatWeapon* activeWeapon = Cheat.LocalPlayer->GetActiveWeapon();

	//if (!activeWeapon || !activeWeapon->ShootingWeapon())
	//	return 13;

	//CCSWeaponData* weapon_data = activeWeapon->GetWeaponInfo();

	//if (!weapon_data)
	//	return 13;

	//return min(weapon_data->flCycleTime / GlobalVars->interval_per_tick, 13);
}

void CDoubleTap::ForceTeleport() {
	teleport_next_tick = true;
}


void CDoubleTap::DefensiveDoubletap() {
	if (defensive_this_tick || defensive_ticks != 0)
		defensive_ticks++;

	if (defensive_ticks == 16)
		defensive_ticks = 0;
}

bool CDoubleTap::ShouldBreakLC() {
	if (!Cheat.LocalPlayer)
		return false;

	if (ctx.tickbase_shift == 0)
		return false;

	if (shifting_tickbase)
		return false;

	if (CBaseCombatWeapon* weapon = Cheat.LocalPlayer->GetActiveWeapon()) {
		if (weapon->IsGrenade() && reinterpret_cast<CBaseGrenade*>(weapon)->m_flThrowTime() > 0.f) {
			triggered_fast_throw = true;
		}
		else if (!weapon->IsGrenade()) {
			triggered_fast_throw = false;
		}

		if (triggered_fast_throw)
			return false;
	}

	bool result = false;

	if (config.ragebot.aimbot.doubletap_options->get(0)) {
		result = true;
	}

	if (config.ragebot.aimbot.doubletap_options->get(1) && allow_defensive) {
		if (defensive_ticks > 0) {
			result = defensive_ticks > 2;
		}
	}

	return result;
}

bool CDoubleTap::IsDefensiveActive() {
	return config.ragebot.aimbot.doubletap_options->get(1) && defensive_ticks > 2;
}

CDoubleTap* DoubleTap = new CDoubleTap;