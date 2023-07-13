#include "Prediction.h"
#include "../RageBot/DoubleTap.h"
#include "../../SDK/Interfaces.h"
#include "../../SDK/Globals.h"

void CPrediction::StartCommand(CBasePlayer* player, CUserCmd* cmd) {
	*Cheat.LocalPlayer->GetCurrentCommand() = cmd;
	Cheat.LocalPlayer->GetLastCommand() = *cmd;
	*predictionRandomSeed = cmd->random_seed;
	*predictionEntity = Cheat.LocalPlayer;
}

void CPrediction::RunPreThink(CBasePlayer* player) {
	if (!player->PhysicsRunThink(0))
		return;

	player->PreThink();
}

void CPrediction::RunThink(CBasePlayer* player, double frametime) {
	static auto SetNextThink = reinterpret_cast<void(__thiscall*)(int)>(Utils::PatternScan("client.dll", "55 8B EC 56 57 8B F9 8B B7 ? ? ? ? 8B C6"));
	int thinktick = player->m_nNextThinkTick();

	if (thinktick <= 0 || thinktick > player->m_nTickBase())
		return;

	SetNextThink(0);

	player->Think();
}

void CPrediction::BackupData() {
	if (Cheat.LocalPlayer->GetActiveWeapon()) {
		if (Cheat.LocalPlayer->GetActiveWeapon()->IsGrenade())
			m_fThrowTime = reinterpret_cast<CBaseGrenade*>(Cheat.LocalPlayer->GetActiveWeapon())->m_flThrowTime();
		m_flNextPrimaryAttack = Cheat.LocalPlayer->GetActiveWeapon()->m_flNextPrimaryAttack();
	}

	m_fFlags = Cheat.LocalPlayer->m_fFlags();
	m_vecVelocity = Cheat.LocalPlayer->m_vecVelocity();
	m_vecAbsVelocity = Cheat.LocalPlayer->m_vecAbsVelocity();

	flOldCurrentTime = GlobalVars->curtime;
	flOldFrameTime = GlobalVars->frametime;
	iOldTickCount = GlobalVars->tickcount;
}

void CPrediction::Start(CUserCmd* cmd) {
	if (!MoveHelper)
		return;

	local_data[cmd->command_number % MULTIPLAYER_BACKUP].init(cmd);

	BackupData();
	StartCommand(Cheat.LocalPlayer, cmd);

	const bool bOldIsFirstPrediction = Prediction->bIsFirstTimePredicted;
	const bool bOldInPrediction = Prediction->bInPrediction;

	GlobalVars->curtime = TICKS_TO_TIME(Cheat.LocalPlayer->m_nTickBase() - ctx.tickbase_shift);
	GlobalVars->frametime = Prediction->bEnginePaused ? 0.f : GlobalVars->interval_per_tick;
	GlobalVars->tickcount = Cheat.LocalPlayer->m_nTickBase() - ctx.tickbase_shift;

	Prediction->bIsFirstTimePredicted = false;
	Prediction->bInPrediction = true;

	MoveHelper->SetHost(Cheat.LocalPlayer);

	GameMovement->StartTrackPredictionErrors(Cheat.LocalPlayer);

	float m_nSequence[2]{ 0, 0 };
	float m_nAnimationParity[2]{ 0, 0 };
	
	for (int i = 0; i < MAX_VIEWMODELS; i++) {
		CBaseViewModel* vm = reinterpret_cast<CBaseViewModel*>(EntityList->GetClientEntityFromHandle(Cheat.LocalPlayer->m_hViewModel()[i]));

		if (vm) {
			m_nSequence[i] = vm->m_nSequence();
			m_nAnimationParity[i] = vm->m_nAnimationParity();
		}
	}
	
	Prediction->SetLocalViewAngles(cmd->viewangles);

	Prediction->SetupMove(Cheat.LocalPlayer, cmd, MoveHelper, &moveData);

	const float backup_velocity_modifier = Cheat.LocalPlayer->m_flVelocityModifier();
	GameMovement->ProcessMovement(Cheat.LocalPlayer, &moveData);
	Cheat.LocalPlayer->m_flVelocityModifier() = backup_velocity_modifier;

	Prediction->FinishMove(Cheat.LocalPlayer, cmd, &moveData);
	MoveHelper->ProcessImpacts();

	GameMovement->FinishTrackPredictionErrors(Cheat.LocalPlayer);
	MoveHelper->SetHost(nullptr);
	GameMovement->Reset();


	for (int i = 0; i < MAX_VIEWMODELS; i++) { 
		CBaseViewModel* vm = reinterpret_cast<CBaseViewModel*>(EntityList->GetClientEntityFromHandle(Cheat.LocalPlayer->m_hViewModel()[i]));

		if (vm) {
			vm->m_nSequence() = m_nSequence[i];
			vm->m_nAnimationParity() = m_nAnimationParity[i];
		}
	}


	if (const auto weapon = Cheat.LocalPlayer->GetActiveWeapon()) {
		//const auto backup_vel = Cheat.LocalPlayer->m_vecVelocity();
		//const auto backup_abs_vel = Cheat.LocalPlayer->m_vecAbsVelocity();
		//const auto backup_flags = Cheat.LocalPlayer->m_iEFlags();

		//Cheat.LocalPlayer->m_iEFlags() &= ~EFL_DIRTY_ABSVELOCITY;
		//Cheat.LocalPlayer->m_vecVelocity() = m_vecVelocity;
		//Cheat.LocalPlayer->m_vecAbsVelocity() = m_vecVelocity;

		weapon->UpdateAccuracyPenality();
		weaponInaccuracy = weapon->GetInaccuracy();
		weaponSpread = weapon->GetSpread();

		//Cheat.LocalPlayer->m_vecVelocity() = backup_vel;
		//Cheat.LocalPlayer->m_vecAbsVelocity() = backup_abs_vel;
		//Cheat.LocalPlayer->m_iEFlags() = backup_flags;
	}

	Prediction->bInPrediction = bOldInPrediction;
	Prediction->bIsFirstTimePredicted = bOldIsFirstPrediction;
}

void CPrediction::End() {
	if (!MoveHelper)
		return;

	GlobalVars->curtime = flOldCurrentTime;
	GlobalVars->frametime = flOldFrameTime;
	GlobalVars->tickcount = iOldTickCount;

	*Cheat.LocalPlayer->GetCurrentCommand() = nullptr;
	*predictionRandomSeed = -1;
	*predictionEntity = nullptr;
}

void CPrediction::PatchAttackPacket(CUserCmd* cmd, bool restore)
{
	static bool m_bLastAttack = false;
	static bool m_bInvalidCycle = false;
	static float m_flLastCycle = 0.f;

	if (!Cheat.LocalPlayer)
		return;

	if (restore)
	{
		m_bLastAttack = cmd->weaponselect || (cmd->buttons & IN_ATTACK);
		m_flLastCycle = Cheat.LocalPlayer->m_flCycle();
	}
	else if (m_bLastAttack && !m_bInvalidCycle)
		m_bInvalidCycle = Cheat.LocalPlayer->m_flCycle() == 0.f && m_flLastCycle > 0.f;

	if (m_bInvalidCycle)
		Cheat.LocalPlayer->m_flCycle() = m_flLastCycle;
}

void CPrediction::StoreNetvars(int place) {
	CBasePlayer* local = Cheat.LocalPlayer;

	auto& nv = local_netvars[place % MULTIPLAYER_BACKUP];
	auto weapon = local->GetActiveWeapon();
	auto viewmodels = local->m_hViewModel();

	nv.m_aimPunchAngle = local->m_aimPunchAngle();
	nv.m_aimPunchAngleVel = local->m_aimPunchAngleVel();
	nv.m_flDuckAmount = local->m_flDuckAmount();
	nv.m_flFallVelocity = local->m_flFallVelocity();
	nv.m_vecOrigin = local->m_vecOrigin();
	nv.m_vecViewOffset = local->m_vecViewOffset();
	nv.m_viewPunchAngle = local->m_viewPunchAngle();
	nv.m_vecVelocity = local->m_vecVelocity();
	nv.m_vecAbsVelocity = local->m_vecAbsVelocity();
	nv.m_vecAbsOrigin = local->GetAbsOrigin();
	nv.m_nTickBase = local->m_nTickBase();
	nv.m_fFlags = local->m_fFlags();
	nv.m_MoveType = local->m_MoveType();
	nv.m_flDuckSpeed = local->m_flDuckSpeed();
	nv.m_flVelocityModifier = local->m_flVelocityModifier();
	nv.m_flThirdpersonRecoil = local->m_flThirdpersonRecoil();

	for (int i = 0; i < MAX_VIEWMODELS; i++) {
		CBaseViewModel* vm = reinterpret_cast<CBaseViewModel*>(EntityList->GetClientEntityFromHandle(viewmodels[i]));

		if (vm) {
			nv.m_nSequence[i] = vm->m_nSequence();
			nv.m_nAnimationParity[i] = vm->m_nAnimationParity();
		}
	}
}

void CPrediction::RestoreNetvars(int place) {
	CBasePlayer* local = Cheat.LocalPlayer;

	auto& nv = local_netvars[place % MULTIPLAYER_BACKUP];
	auto weapon = local->GetActiveWeapon();
	auto viewmodels = local->m_hViewModel();

	local->m_aimPunchAngle() = nv.m_aimPunchAngle;
	local->m_aimPunchAngleVel() = nv.m_aimPunchAngleVel;
	local->m_flDuckAmount() = nv.m_flDuckAmount;
	local->m_flFallVelocity() = nv.m_flFallVelocity;
	local->m_vecOrigin() = nv.m_vecOrigin;
	local->m_vecViewOffset() = nv.m_vecViewOffset;
	local->m_viewPunchAngle() = nv.m_viewPunchAngle;
	local->m_vecVelocity() = nv.m_vecVelocity;
	local->m_vecAbsVelocity() = nv.m_vecAbsVelocity;
	local->SetAbsOrigin(nv.m_vecAbsOrigin);
	//local->m_nTickBase() = nv.m_nTickBase;
	local->m_fFlags() = nv.m_fFlags;
	local->m_MoveType() = nv.m_MoveType;
	local->m_flDuckSpeed() = nv.m_flDuckSpeed;
	local->m_flVelocityModifier() = nv.m_flVelocityModifier;
	local->m_flThirdpersonRecoil() = nv.m_flThirdpersonRecoil;

	for (int i = 0; i < MAX_VIEWMODELS; i++) {
		CBaseViewModel* vm = reinterpret_cast<CBaseViewModel*>(EntityList->GetClientEntityFromHandle(viewmodels[i]));

		if (vm) {
			vm->m_nSequence() = nv.m_nSequence[i]; 
			vm->m_nAnimationParity() = nv.m_nAnimationParity[i];
		}
	}
}

void CPrediction::PredictNetvars(int place) {
	static ConVar* sv_gravity = CVar->FindVar("sv_gravity");

	CBasePlayer* local = Cheat.LocalPlayer;

	auto& nv = local_netvars[place % MULTIPLAYER_BACKUP];
	auto& prevNv = local_netvars[(place - 1) % MULTIPLAYER_BACKUP];

	if (!(Cheat.LocalPlayer->m_fFlags() & FL_ONGROUND)) {
		float flFallVelocity = nv.m_vecVelocity.z - sv_gravity->GetFloat() * GlobalVars->interval_per_tick;

		local->m_vecVelocity() += nv.m_vecVelocity - prevNv.m_vecVelocity;

		local->m_flFallVelocity() = -flFallVelocity;
		local->m_vecVelocity().z = flFallVelocity;
		local->m_vecOrigin() += local->m_vecVelocity() * GlobalVars->interval_per_tick;
	}
}

CPrediction* EnginePrediction = new CPrediction;