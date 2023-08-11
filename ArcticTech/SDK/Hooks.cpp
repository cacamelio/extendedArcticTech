#include "Hooks.h"
#include "../Utils/Utils.h"
#include "../Utils/Hash.h"
#include "Config.h"
#include "Globals.h"
#include <intrin.h>
#include "../Utils/Console.h"
#include "../UI/UI.h"

#include "../Features/Misc/AutoStrafe.h"
#include "../Features/Visuals/ESP.h"
#include "../Features/Visuals/Glow.h"
#include "../Features/Lua/Bridge/Bridge.h"
#include "../Features/Visuals/Chams.h"
#include "../Features/Visuals/World.h"
#include "../Features/Visuals/GlovesChanger.h"
#include "../Features/Visuals/GrenadePrediction.h"
#include "../Features/Visuals/Indicators.h"
#include "../Features/Misc/Prediction.h"
#include "../Features/AntiAim/AntiAim.h"
#include "../Features/RageBot/LagCompensation.h"
#include "../Features/RageBot/Exploits.h"
#include "../Features/Misc/AutoPeek.h"
#include "../Features/RageBot/Ragebot.h"
#include "../Features/RageBot/AutoWall.h"
#include "../Features/RageBot/Resolver.h"
#include "../Features/RageBot/AnimationSystem.h"
#include "../Features/Misc/Misc.h"
#include "../Features/Misc/EventListner.h"
#include "../Features/Visuals/SkinChanger.h"
#include "../Features/ShotManager/ShotManager.h"
#include "../Features/Visuals/PreserveKillfeed.h"

GrenadePrediction NadePrediction;

template <typename T>
inline T HookFunction(void* pTarget, void* pDetour) {
	return (T)DetourFunction((PBYTE)pTarget, (PBYTE)pDetour);
}

inline void RemoveHook(void* original, void* detour) {
	DetourRemove((PBYTE)original, (PBYTE)detour);
}

LRESULT CALLBACK hkWndProc(HWND Hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	if (!Render->IsInitialized() || !Menu->IsInitialized())
		return CallWindowProc(oWndProc, Hwnd, Message, wParam, lParam);

	if (Message == WM_KEYDOWN && !EngineClient->Con_IsVisible()) {
		AntiAim->OnKeyPressed(wParam);

		Cheat.KeyStates[wParam] = !Cheat.KeyStates[wParam];
	}
	
	if (Menu->IsOpened())
		Menu->WndProc(Hwnd, Message, wParam, lParam);

	return CallWindowProc(oWndProc, Hwnd, Message, wParam, lParam);
}

HRESULT __stdcall hkReset(IDirect3DDevice9* thisptr, D3DPRESENT_PARAMETERS* params) {
	static tReset oReset = (tReset)Hooks::DirectXDeviceVMT->GetOriginal(16);

	auto result = oReset(thisptr, params);

	Render->Reset();

	return result;
}

HRESULT __stdcall hkPresent(IDirect3DDevice9* thisptr, const RECT* src, const RECT* dest, HWND window, const RGNDATA* dirtyRegion) {
	if (thisptr != DirectXDevice)
		return oPresent(thisptr, src, dest, window, dirtyRegion);

	if (!Render->IsInitialized()) {
		Render->Init(thisptr);
		Menu->Setup();
		return oPresent(thisptr, src, dest, window, dirtyRegion);
	}

	Cheat.InGame = EngineClient->IsConnected() && EngineClient->IsInGame();

	if (Cheat.InGame && !PlayerResource)
		PlayerResource = **(CCSPlayerResource***)(Utils::PatternScan("client.dll", "8B 3D ? ? ? ? 85 FF 0F 84 ? ? ? ? 81 C7", 0x2));
	else if (!Cheat.InGame)
		PlayerResource = nullptr;

	if (thisptr->BeginScene() == D3D_OK) {
		Render->RenderDrawData();
		Menu->Render();
		thisptr->EndScene();
	}

	return oPresent(thisptr, src, dest, window, dirtyRegion);
}

void __fastcall hkHudUpdate(IBaseClientDLL* thisptr, void* edx, bool bActive) {
	static auto oHudUpdate = (tHudUpdate)Hooks::ClientVMT->GetOriginal(11);

	Cheat.LocalPlayer = (CBasePlayer*)EntityList->GetClientEntity(EngineClient->GetLocalPlayer());

	if (!Render->IsInitialized() || !Menu->IsInitialized())
		return;

	Render->BeginFrame();

	Render->UpdateViewMatrix(EngineClient->WorldToScreenMatrix());

	ESP::Draw();
	ESP::DrawGrenades();
	ESP::RenderMarkers();
	NadePrediction.Draw();
	AutoPeek->Draw();
	World->Crosshair();
	if (config.ragebot.aimbot.show_debug_data->get())
		Ragebot->DrawDebugData();
	Indicators->Draw();

	for (auto& callback : Lua->hooks.getHooks(LUA_RENDER))
		callback.func();

	InputSystem->EnableInput(!Menu->IsOpened());

	Render->EndFrame();

	oHudUpdate(thisptr, edx, bActive);
}

void __fastcall hkLockCursor(ISurface* thisptr, void* edx) {
	static tLockCursor oLockCursor = (tLockCursor)Hooks::SurfaceVMT->GetOriginal(67);

	if (Menu->IsOpened())
		return Surface->UnlockCursor();

	oLockCursor(thisptr, edx);
}

MDLHandle_t __fastcall hkFindMdl(void* ecx, void* edx, char* FilePath)
{
	static auto oFindMdlHook = (tFindMdlHook)Hooks::ModelCacheVMT->GetOriginal(10);

	if (strstr(FilePath, "facemask_battlemask.mdl"))
		sprintf(FilePath, "models/player/holiday/facemasks/facemask_dallas.mdl");

	return oFindMdlHook(ecx, edx, FilePath);
}

void __stdcall CreateMove(int sequence_number, float sample_frametime, bool active, bool& bSendPacket) {
	static auto oCHLCCreateMove = (tCHLCCreateMove)Hooks::ClientVMT->GetOriginal(22);

	oCHLCCreateMove(Client, sequence_number, sample_frametime, active);

	CUserCmd* cmd = Input->GetUserCmd(sequence_number);
	CVerifiedUserCmd* verified = Input->GetVerifiedCmd(sequence_number);

	Miscelleaneus::Clantag();

	Exploits->DefenseiveThisTick() = false;

	if (!cmd || !cmd->command_number || !Cheat.LocalPlayer || !Cheat.LocalPlayer->IsAlive())
		return;

	CBaseCombatWeapon* weapon = Cheat.LocalPlayer->GetActiveWeapon();

	if (!cmd || !cmd->command_number)
		return;

	ctx.cmd = cmd;
	ctx.send_packet = true;

	if (config.misc.movement.infinity_duck->get())
		ctx.cmd->buttons |= IN_BULLRUSH;

	if (config.misc.movement.auto_jump->get()) {
		if (!(Cheat.LocalPlayer->m_fFlags() & FL_ONGROUND) && Cheat.LocalPlayer->m_MoveType() != MOVETYPE_NOCLIP && Cheat.LocalPlayer->m_MoveType() != MOVETYPE_LADDER)
			cmd->buttons &= ~IN_JUMP;
	}

	if (config.misc.movement.quick_stop->get() && (cmd->buttons & (IN_MOVELEFT | IN_MOVERIGHT | IN_FORWARD | IN_BACK | IN_JUMP)) == 0 && Cheat.LocalPlayer->m_vecVelocity().LengthSqr() > 256 && Cheat.LocalPlayer->m_fFlags() & FL_ONGROUND) {
		Vector vec_speed = Cheat.LocalPlayer->m_vecVelocity();
		QAngle direction = Math::VectorAngles(vec_speed);

		QAngle view; EngineClient->GetViewAngles(&view);
		direction.yaw = view.yaw - direction.yaw;
		direction.Normalize();

		Vector forward;
		Math::AngleVectors(direction, forward);

		Vector nigated_direction = forward * -std::clamp(vec_speed.Q_Length2D(), 0.f, 450.f) * 0.9f;

		cmd->sidemove = nigated_direction.y;
		cmd->forwardmove = nigated_direction.x;
	}

	Miscelleaneus::AutoStrafe();


	if (Exploits->IsShifting()) {
		if (ClientState->m_nDeltaTick > 0)
			Prediction->Update(ClientState->m_nDeltaTick, ClientState->m_nDeltaTick > 0, ClientState->m_nLastCommandAck, ClientState->m_nLastOutgoingCommand + ClientState->m_nChokedCommands);

		EnginePrediction->Start(cmd);

		Cheat.ServerTime = GlobalVars->curtime;

		QAngle eyeYaw = cmd->viewangles;

		AutoPeek->CreateMove();
		Ragebot->Run();

		AntiAim->Angles();
		AntiAim->SlowWalk();

		ctx.send_packet = bSendPacket = ctx.tickbase_shift == 1;
		cmd->buttons &= ~(IN_ATTACK | IN_ATTACK2);

		EnginePrediction->End();

		cmd->viewangles.Normalize();
		Utils::FixMovement(cmd, eyeYaw);

		ctx.lc_exploit_prev = ctx.lc_exploit;
		ctx.lc_exploit = false;

		if (!ctx.lc_exploit && ctx.lc_exploit_prev)
			ctx.lc_exploit_shift = cmd->command_number;

		ctx.shifted_commands.emplace_back(cmd->command_number);
		ctx.sented_commands.emplace_back(cmd->command_number);
		ctx.teleported_last_tick = true;

		verified->m_cmd = *cmd;
		verified->m_crc = cmd->GetChecksum();
		return;
	}

	// pre_prediction

	ctx.is_peeking = AntiAim->IsPeeking();

	if (ctx.is_peeking && config.ragebot.aimbot.doubletap_options->get(1))
		Exploits->DefenseiveThisTick() = true;

	AntiAim->SlowWalk();

	QAngle storedAng = cmd->viewangles;

	if (ClientState->m_nDeltaTick > 0)
		Prediction->Update(ClientState->m_nDeltaTick, ClientState->m_nDeltaTick > 0, ClientState->m_nLastCommandAck, ClientState->m_nLastOutgoingCommand + ClientState->m_nChokedCommands);

	EnginePrediction->Start(cmd);

	ShotManager->DetectUnregisteredShots();

	CUserCmd_lua lua_cmd;
	lua_cmd.command_number = cmd->command_number;
	lua_cmd.tickcount = cmd->tick_count;
	lua_cmd.hasbeenpredicted = cmd->hasbeenpredicted;
	lua_cmd.move = Vector(cmd->sidemove, cmd->forwardmove, cmd->upmove);
	lua_cmd.viewangles = cmd->viewangles;
	lua_cmd.random_seed = cmd->random_seed;
	lua_cmd.buttons = cmd->buttons;

	for (auto& callback : Lua->hooks.getHooks(LUA_CREATEMOVE)) {
		callback.func(&lua_cmd);
	}

	cmd->buttons = lua_cmd.buttons;
	cmd->tick_count = lua_cmd.tickcount;
	Exploits->AllowDefensive() = lua_cmd.allow_defensive;

	if (lua_cmd.override_defensive) {
		Exploits->DefenseiveThisTick() = lua_cmd.override_defensive.as<bool>();
	}

	ctx.last_local_velocity = ctx.local_velocity;
	ctx.local_velocity = Cheat.LocalPlayer->m_vecVelocity();

	if (ctx.local_velocity.LengthSqr() > 0.f)
		Cheat.freezetime = false;

	Miscelleaneus::CompensateThrowable();

	// prediction

	Cheat.ServerTime = GlobalVars->curtime;

	if (config.misc.movement.edge_jump->get() && !(Cheat.LocalPlayer->m_fFlags() & FL_ONGROUND) && EnginePrediction->m_fFlags & FL_ONGROUND)
		cmd->buttons |= IN_JUMP;

	if (weapon->ShootingWeapon() && !weapon->CanShoot() && weapon->m_iItemDefinitionIndex() != Revolver)
		cmd->buttons &= ~IN_ATTACK;

	if (weapon->ShootingWeapon() && weapon->CanShoot() && cmd->buttons & IN_ATTACK) {
		if (Exploits->GetExploitType() == CExploits::E_DoubleTap)
			Exploits->ForceTeleport();
		else if (Exploits->GetExploitType() == CExploits::E_HideShots)
			Exploits->HideShot();

		ShotManager->ProcessManualShot();
	}

	AntiAim->FakeLag();
	AntiAim->FakeDuck();
	AntiAim->Angles();

	AutoPeek->CreateMove();

	Ragebot->Run();

	Exploits->DefensiveDoubletap();

	cmd->viewangles.Normalize(config.misc.miscellaneous.anti_untrusted->get());

	if (ctx.send_packet && !(Exploits->GetExploitType() == CExploits::E_HideShots && Exploits->shot_cmd == cmd->command_number)) {
		Cheat.thirdpersonAngles = cmd->viewangles;
		if (!config.antiaim.angles.body_yaw_options->get(1) || Utils::RandomInt(0, 10) > 5)
			AntiAim->jitter = !AntiAim->jitter;
		ctx.should_update_local_anims = true;

		ctx.breaking_lag_compensation = (ctx.local_sent_origin - Cheat.LocalPlayer->m_vecOrigin()).LengthSqr() > 4096;
		ctx.local_sent_origin = Cheat.LocalPlayer->m_vecOrigin();
	}

	AnimationSystem->OnCreateMove();

	EnginePrediction->End();

	// createmove

	ctx.lc_exploit_prev = ctx.lc_exploit;
	ctx.lc_exploit = Exploits->ShouldBreakLC();

	if (ctx.lc_exploit && !ctx.lc_exploit_prev)
		ctx.lc_exploit_charge = cmd->command_number;
	else if (!ctx.lc_exploit && ctx.lc_exploit_prev)
		ctx.lc_exploit_shift = cmd->command_number;

	Utils::FixMovement(cmd, storedAng);

	AntiAim->LegMovement();

	bSendPacket = ctx.send_packet;
	ctx.sented_commands.emplace_back(cmd->command_number);
	ctx.teleported_last_tick = false;

	verified->m_cmd = *cmd;
	verified->m_crc = cmd->GetChecksum();
}

__declspec(naked) void __fastcall hkCHLCCreateMove(IBaseClientDLL* thisptr, void*, int sequence_number, float input_sample_frametime, bool active) {
	__asm {
		push ebp
		mov  ebp, esp
		push ebx
		push esp
		push dword ptr[active]
		push dword ptr[input_sample_frametime]
		push dword ptr[sequence_number]
		call CreateMove
		pop  ebx
		pop  ebp
		retn 0Ch
	}
}

void* __fastcall hkAllocKeyValuesMemory(IKeyValuesSystem* thisptr, void* edx, int iSize)
{
	static auto oAllocKeyValuesMemory = (void*(__fastcall*)(IKeyValuesSystem*, void*, int))Hooks::KeyValuesVMT->GetOriginal(2);

	// return addresses of check function
	// @credits: danielkrupinski
	static const void* uAllocKeyValuesEngine = Utils::PatternScan("engine.dll", "55 8B EC 56 57 8B F9 8B F2 83 FF 11 0F 87 ? ? ? ? 85 F6 0F 84 ? ? ? ?", 0x4A);
	static const void* uAllocKeyValuesClient = Utils::PatternScan("client.dll", "55 8B EC 56 57 8B F9 8B F2 83 FF 11 0F 87 ? ? ? ? 85 F6 0F 84 ? ? ? ?", 0x3E);

	// doesn't call it yet, but have checking function
	//static const std::uintptr_t uAllocKeyValuesMaterialSystem = MEM::FindPattern(MATERIALSYSTEM_DLL, XorStr("FF 52 04 85 C0 74 0C 56")) + 0x3;
	//static const std::uintptr_t uAllocKeyValuesStudioRender = MEM::FindPattern(STUDIORENDER_DLL, XorStr("FF 52 04 85 C0 74 0C 56")) + 0x3;

	if (const void* uReturnAddress = _ReturnAddress(); uReturnAddress == uAllocKeyValuesEngine || uReturnAddress == uAllocKeyValuesClient)
		return nullptr;

	return oAllocKeyValuesMemory(thisptr, edx, iSize);
}

char* __fastcall hk_get_halloween_mask_model_addon( void* ecx, void* edx )
{
	return ( char* )"models/player/holiday/facemasks/facemask_dallas.mdl";
}

bool __fastcall hkSetSignonState(void* thisptr, void* edx, int state, int count, const void* msg) {
	bool result = oSetSignonState(thisptr, edx, state, count, msg);

	if (state == 6) { // SIGNONSTATE_FULL
		ctx.update_nightmode = true;
		ctx.update_remove_blood = true;

		World->SkyBox();
		World->Fog();
		World->Smoke();

		static ConVar* cl_threaded_bone_setup = CVar->FindVar("cl_threaded_bone_setup");

		cl_threaded_bone_setup->SetInt(1);

		GrenadePrediction::PrecacheParticles();
		Ragebot->CalcSpreadValues();
		ShotManager->Reset();
		Resolver->Reset();

		for (auto& callback : Lua->hooks.getHooks(LUA_LEVELINIT))
			callback.func();
	}

	return result;
}

void __fastcall hkLevelShutdown(IBaseClientDLL* thisptr, void* edx) {
	static auto oLevelShutdown = (void(__thiscall*)(IBaseClientDLL*))Hooks::ClientVMT->GetOriginal(7);

	Exploits->target_tickbase_shift = ctx.tickbase_shift = 0;
	ctx.reset();
	LagCompensation->Reset();
	AnimationSystem->ResetInterpolation();
	ShotManager->Reset();

	oLevelShutdown(thisptr);
}

void __fastcall hkOverrideView(IClientMode* thisptr, void* edx, CViewSetup* setup) {
	static tOverrideView oOverrideView = (tOverrideView)Hooks::ClientModeVMT->GetOriginal(18);

	if (setup->fov == 90 || config.visuals.effects.removals->get(5)) {
		setup->fov = config.visuals.effects.fov->get();
	}

	World->ProcessCamera(setup);

	if (Cheat.InGame && Cheat.LocalPlayer && Cheat.LocalPlayer->m_iHealth() > 0) {
		NadePrediction.Start(setup->angles, setup->origin);
	}

	if (Cheat.LocalPlayer && Cheat.LocalPlayer->m_iHealth() > 0 && config.antiaim.misc.fake_duck->get())
		setup->origin = Cheat.LocalPlayer->GetAbsOrigin() + Vector(0, 0, 64);

	setup->angles.roll = 0;

	oOverrideView(thisptr, edx, setup);
}

void __fastcall hkPaintTraverse(IPanel* thisptr, void* edx, unsigned int panel, bool bForceRepaint, bool bForce) {
	static tPaintTraverse oPaintTraverse = (tPaintTraverse)Hooks::PanelVMT->GetOriginal(41);
	static unsigned int hud_zoom_panel = 0;

	if (!Cheat.LocalPlayer->IsAlive())
	{
		KillFeed->ClearDeathNotice = true;
	}

	if (!hud_zoom_panel) {
		std::string panelName = VPanel->GetName(panel);

		if (panelName == "HudZoom")
			hud_zoom_panel = panel;
	}

	if (hud_zoom_panel == panel && config.visuals.effects.remove_scope->get() > 0)
		return;

	oPaintTraverse(thisptr, edx, panel, bForceRepaint, bForce);
}

void __fastcall hkDoPostScreenEffects(IClientMode* thisptr, void* edx, CViewSetup* setup) {
	static tDoPostScreenEffects oDoPostScreenEffects = (tDoPostScreenEffects)Hooks::ClientModeVMT->GetOriginal(44);

	Chams->RenderShotChams();
	Glow::Run();

	oDoPostScreenEffects(thisptr, edx, setup);
}

bool __fastcall hkIsPaused(IVEngineClient* thisptr, void* edx) {
	static void* addr = Utils::PatternScan("client.dll", "FF D0 A1 ?? ?? ?? ?? B9 ?? ?? ?? ?? D9 1D ?? ?? ?? ?? FF 50 34 85 C0 74 22 8B 0D ?? ?? ?? ??", 0x29);
	static tIsPaused oIsPaused = (tIsPaused)Hooks::EngineVMT->GetOriginal(90);

	if (_ReturnAddress() == addr)
		return true;

	return oIsPaused(thisptr, edx);
}

void __fastcall hkDrawModelExecute(IVModelRender* thisptr, void* edx, void* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld) {
	static tDrawModelExecute oDrawModelExecute = (tDrawModelExecute)Hooks::ModelRenderVMT->GetOriginal(21);

	if (hook_info.in_draw_static_props)
		return oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);

	if (StudioRender->IsForcedMaterialOverride())
		return oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);

	if (Chams->OnDrawModelExecute(ctx, state, pInfo, pCustomBoneToWorld))
		return;

	oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
}

int __fastcall hkGetBool(ConVar* thisptr, void* edx) {
	static tGetBool oGetBool = (tGetBool)Hooks::ConVarVMT->GetOriginal(13);
	static void* cameraThink = Utils::PatternScan("client.dll", "85 C0 75 30 38 87");

	if (_ReturnAddress() == cameraThink)
		return 1;

	return oGetBool(thisptr, edx);
}

void __fastcall hkFrameStageNotify(IBaseClientDLL* thisptr, void* edx, EClientFrameStage stage) {
	static auto oFrameStageNotify = (tFrameStageNotify)Hooks::ClientVMT->GetOriginal(37);
	Cheat.LocalPlayer = (CBasePlayer*)EntityList->GetClientEntity(EngineClient->GetLocalPlayer());

	switch (stage) {
	case FRAME_RENDER_START:
		AnimationSystem->RunInterpolation();
		Chams->UpdateSettings();
		World->SunDirection();

		cvars.r_aspectratio->SetFloat(config.visuals.effects.aspect_ratio->get());
		cvars.mat_postprocessing_enable->SetInt(!config.visuals.effects.removals->get(0));
		cvars.cl_csm_shadows->SetInt(!config.visuals.effects.removals->get(2));
		cvars.cl_foot_contact_shadows->SetInt(0);
		cvars.r_drawsprites->SetInt(!config.visuals.effects.removals->get(6));
		cvars.zoom_sensitivity_ratio_mouse->SetInt(!config.visuals.effects.removals->get(5));
		SkinChanger->Run(false);

		if (Cheat.LocalPlayer && config.visuals.effects.removals->get(4))
			Cheat.LocalPlayer->m_flFlashDuration() = 0.f;

		break;
	case FRAME_RENDER_END:
		if (ctx.update_nightmode) {
			World->Modulation();
			ctx.update_nightmode = false;
		}
		SkinChanger->Run(true);

		if (ctx.update_remove_blood) {
			World->RemoveBlood();
			ctx.update_remove_blood = false;
		}
		break;
	case FRAME_NET_UPDATE_START:
		ShotManager->OnNetUpdate();
		ESP::ProcessSounds();
		break;
	case FRAME_NET_UPDATE_END:
		LagCompensation->OnNetUpdate();
		if (Cheat.InGame) {
			EngineClient->FireEvents();
		}
		break;
	case FRAME_NET_UPDATE_POSTDATAUPDATE_START:
		SkinChanger->AgentChanger();
		SkinChanger->MaskChanger();
		SkinChanger->Run(true);
		break;
	case FRAME_NET_UPDATE_POSTDATAUPDATE_END:
		break;
	}

	if (Cheat.InGame && Cheat.LocalPlayer && Exploits->ShouldCharge())
		GlobalVars->interpolation_amount = 0.f;

	AnimationSystem->FrameStageNotify(stage);

	oFrameStageNotify(thisptr, edx, stage);
}

void __fastcall hkUpdateClientSideAnimation(CBasePlayer* thisptr, void* edx) {
	if (hook_info.update_csa)
		return oUpdateClientSideAnimation(thisptr, edx);
	if (thisptr == Cheat.LocalPlayer)
		return oUpdateClientSideAnimation(thisptr, edx);
	if (!thisptr->IsPlayer())
		return oUpdateClientSideAnimation(thisptr, edx);
}

bool __fastcall hkShouldSkipAnimationFrame(void* thisptr, void* edx) {
	return false;
}

bool __fastcall hkShouldInterpolate(CBasePlayer* thisptr, void* edx) {
	if (!Exploits->ShouldCharge() || thisptr != Cheat.LocalPlayer)
		return oShouldInterpolate(thisptr, edx);

	AnimationSystem->DisableInterpolationFlags(thisptr);

	return false;
}

void __fastcall hkDoExtraBoneProcessing(CBaseEntity* player, void* edx, CStudioHdr* hdr, Vector* pos, Quaternion* q, const matrix3x4_t& matrix, uint8_t* bone_list, void* contex) {

}

void __fastcall hkStandardBlendingRules(CBasePlayer* thisptr, void* edx, void* hdr, void* pos, void* q, float current_time, int bone_mask) {
	if (thisptr == Cheat.LocalPlayer)
		thisptr->m_fEffects() |= EF_NOINTERP;
	oStandardBlendingRules(thisptr, edx, hdr, pos, q, current_time, bone_mask);
	if (thisptr == Cheat.LocalPlayer)
		thisptr->m_fEffects() &= ~EF_NOINTERP;
}

bool __fastcall hkIsHLTV(IVEngineClient* thisptr, void* edx) {
	static auto oIsHLTV = (tIsHLTV)Hooks::EngineVMT->GetOriginal(93);

	if (hook_info.setup_bones || hook_info.update_csa)
		return true;

	static const auto setup_velocity = Utils::PatternScan("client.dll", "84 C0 75 38 8B 0D ? ? ? ? 8B 01 8B 80 ? ? ? ? FF D0");
	static const auto accumulate_layers = Utils::PatternScan("client.dll", "84 C0 75 0D F6 87");

	if (_ReturnAddress() == setup_velocity || _ReturnAddress() == accumulate_layers)
		return true;

	return oIsHLTV(thisptr, edx);
}

void __fastcall hkBuildTransformations(CBaseEntity* thisptr, void* edx, void* hdr, void* pos, void* q, const void* camera_transform, int bone_mask, void* bone_computed) {
	if (thisptr)
		thisptr->m_isJiggleBonesEnabled() = false;

	oBuildTransformations(thisptr, edx, hdr, pos, q, camera_transform, bone_mask, bone_computed);
}

void __fastcall hkSetUpLean(CCSGOPlayerAnimationState* thisptr, void* edx) {
	if (config.antiaim.misc.animations->get(0) && thisptr->pEntity == Cheat.LocalPlayer)
		oSetUpLean(thisptr, edx);
}

bool __fastcall hkSetupBones(CBaseEntity* thisptr, void* edx, matrix3x4_t* pBoneToWorld, int maxBones, int mask, float curTime) {
	if (hook_info.setup_bones || !thisptr)
		return oSetupBones(thisptr, edx, pBoneToWorld, maxBones, mask, curTime);

	CBasePlayer* ent = (CBasePlayer*)((uintptr_t)thisptr - 0x4);

	if (!ent->IsPlayer() || !ent->IsAlive())
		return oSetupBones(thisptr, edx, pBoneToWorld, maxBones, mask, curTime);

	if (ent == Cheat.LocalPlayer) {
		memcpy(ent->GetCachedBoneData().Base(), AnimationSystem->GetLocalBoneMatrix(), ent->GetCachedBoneData().Count() * sizeof(matrix3x4_t));
		AnimationSystem->CorrectLocalMatrix(ent->GetCachedBoneData().Base(), ent->GetCachedBoneData().Count());

		if (mask & BONE_USED_BY_ATTACHMENT)
			Cheat.LocalPlayer->SetupBones_AttachmentHelper();

		if (pBoneToWorld && maxBones != -1) {
			ent->CopyBones(pBoneToWorld);
		}

		return true;
	}

	if (!hook_info.disable_interpolation && mask & BONE_USED_BY_ATTACHMENT) {
		AnimationSystem->InterpolateModel(ent, ent->GetCachedBoneData().Base());

		//const auto backup_abs_origin = ent->GetAbsOrigin();
		ent->SetAbsOrigin(AnimationSystem->GetInterpolated(ent));
		//ent->m_BoneAccessor().m_ReadableBones |= BONE_USED_BY_ATTACHMENT;
		//ent->m_BoneAccessor().m_WritableBones |= BONE_USED_BY_ATTACHMENT;
		//ent->CopyBones(ent->m_BoneAccessor().m_pBones);
		ent->SetupBones_AttachmentHelper();
		//ent->SetAbsOrigin(backup_abs_origin);
	}

	if (pBoneToWorld && maxBones != -1) {
		ent->CopyBones(pBoneToWorld);
	}

	return true;
}

void __fastcall hkRunCommand(IPrediction* thisptr, void* edx, CBasePlayer* player, CUserCmd* cmd, IMoveHelper* moveHelper) {
	static auto oRunCommand = (tRunCommand)(Hooks::PredictionVMT->GetOriginal(19));

	if (!player || !cmd || player != Cheat.LocalPlayer)
		return oRunCommand(thisptr, edx, player, cmd, moveHelper);

	int max_ticbase_shift = Exploits->MaxTickbaseShift();

	if (ctx.lc_exploit_shift == cmd->command_number)
		player->m_nTickBase() += Exploits->TargetTickbaseShift() > 0 ? max_ticbase_shift : max_ticbase_shift - 1;
	if (ctx.lc_exploit_charge == cmd->command_number)
		player->m_nTickBase() -= max_ticbase_shift;

	const int backup_tickbase = player->m_nTickBase();
	const float backup_velocity_modifier = player->m_flVelocityModifier();

	oRunCommand(thisptr, edx, player, cmd, moveHelper);

	//EnginePrediction->PatchAttackPacket(cmd, true);

	player->m_flVelocityModifier() = backup_velocity_modifier;

	for (auto i = ctx.shifted_commands.begin(); i != ctx.shifted_commands.end();) {
		auto command = *i;

		if (cmd->command_number - command > 32) {
			i = ctx.shifted_commands.erase(i);
			continue;
		}

		if (command == cmd->command_number) {
			player->m_nTickBase() = backup_tickbase;
		}

		++i;
	}

	//EnginePrediction->PatchAttackPacket(cmd, false);

	MoveHelper = moveHelper;
}

void __fastcall hkPhysicsSimulate(CBasePlayer* thisptr, void* edx) {
	const int tick_base = thisptr->m_nTickBase();
	C_CommandContext* c_ctx = thisptr->GetCommandContext();

	if (thisptr != Cheat.LocalPlayer || !Cheat.LocalPlayer->IsAlive() || thisptr->m_nSimulationTick() == GlobalVars->tickcount || !c_ctx->needsprocessing)
		return oPhysicsSimulate(thisptr, edx);

	auto& local_data = EnginePrediction->GetLocalData(c_ctx->command_number);

	if (c_ctx->command_number == Exploits->charged_command + 1) {
		thisptr->m_nTickBase() = local_data.m_nTickBase + ctx.shifted_last_tick + 1;
		//EnginePrediction->RestoreNetvars(last_simulated_tick % 150);
	}

	oPhysicsSimulate(thisptr, edx);

	EnginePrediction->StoreNetvars(c_ctx->command_number % 150);
}

void __fastcall hkPacketStart(CClientState* thisptr, void* edx, int incoming_sequence, int outgoing_acknowledged) {
	if (!Cheat.InGame || !Cheat.LocalPlayer || !Cheat.LocalPlayer->IsAlive())
		return oPacketStart(thisptr, edx, incoming_sequence, outgoing_acknowledged);

	for (auto it = ctx.sented_commands.begin(); it != ctx.sented_commands.end(); it++) {
		if (*it == outgoing_acknowledged) {
			oPacketStart(thisptr, edx, incoming_sequence, outgoing_acknowledged);
			break;
		}
	}

	ctx.sented_commands.erase(
		std::remove_if(
			ctx.sented_commands.begin(),
			ctx.sented_commands.end(),
			[&](auto const& command) { return command < outgoing_acknowledged; }),
		ctx.sented_commands.end());
}

void __fastcall hkPacketEnd(CClientState* thisptr, void* edx) {
	if (!Cheat.LocalPlayer || ClientState->m_ClockDriftMgr.m_nServerTick != ClientState->m_nDeltaTick)
		return oPacketEnd(thisptr, edx);

	//const auto& local_data = EnginePrediction->GetLocalData(ClientState->m_nCommandAck % 150);
	//if (local_data.shift_amount > 0 && local_data.m_nTickBase > Cheat.LocalPlayer->m_nTickBase())
	//	Cheat.LocalPlayer->m_nTickBase() = local_data.m_nTickBase + 1;

	//ESP::ProcessSounds();

	oPacketEnd(thisptr, edx);
}

void __fastcall hkClampBonesInBBox(CBasePlayer* thisptr, void* edx, matrix3x4_t* bones, int boneMask) {
	if (config.antiaim.angles.legacy_desync->get() || hook_info.disable_clamp_bones)
		return;

	if (thisptr->m_fFlags() & FL_FROZEN) {
		thisptr->m_vecMaxs().z = 72.f; // abobus fix

		auto collidable = thisptr->GetCollideable();

		if (collidable)
			collidable->OBBMaxs().z = 72.f;
	}

	return oClampBonesInBBox(thisptr, edx, bones, boneMask);
}

void __cdecl hkCL_Move(float accamulatedExtraSamples, bool bFinalTick) {
	Cheat.LocalPlayer = (CBasePlayer*)EntityList->GetClientEntity(EngineClient->GetLocalPlayer());

	if (!Cheat.LocalPlayer || !Cheat.LocalPlayer->IsAlive())
		return oCL_Move(accamulatedExtraSamples, bFinalTick);

	auto nc = ClientState->m_NetChannel;
	int out_seq_nr = nc ? nc->m_nOutSequenceNr : 0;

	Exploits->Run();

	if (Exploits->ShouldCharge()) {
		ctx.tickbase_shift++;
		ctx.shifted_last_tick++;
		return;
	}

	oCL_Move(accamulatedExtraSamples, bFinalTick);

	Exploits->HandleTeleport(oCL_Move, accamulatedExtraSamples);
}

QAngle* __fastcall hkGetEyeAngles(CBasePlayer* thisptr, void* edx) {
	if (thisptr != Cheat.LocalPlayer)
		return oGetEyeAngles(thisptr, edx);

	static void* EyeAnglesPitch = Utils::PatternScan("client.dll", "8B CE F3 0F 10 00 8B 06 F3 0F 11 45 ? FF 90 ? ? ? ? F3 0F 10 55 ?");
	static void* EyeAnglesYaw = Utils::PatternScan("client.dll", "F3 0F 10 55 ? 51 8B 8E ? ? ? ?");
	static void* ShitAnimRoll = Utils::PatternScan("client.dll", "8B 55 0C 8B C8 E8 ? ? ? ? 83 C4 08 5E 8B E5");

	if (_ReturnAddress() != EyeAnglesPitch && _ReturnAddress() != EyeAnglesYaw && _ReturnAddress() != ShitAnimRoll)
		return oGetEyeAngles(thisptr, edx);

	return &Cheat.thirdpersonAngles;
}

bool __fastcall hkSendNetMsg(INetChannel* thisptr, void* edx, INetMessage& msg, bool bForceReliable, bool bVoice) {
	if (msg.GetType() == 14) // Return and don't send messsage if its FileCRCCheck
		return true;

	if (msg.GetGroup() == 9) { // Fix lag when transmitting voice and fakelagging
		bVoice = true;
	}
	return oSendNetMsg(thisptr, edx, msg, bForceReliable, bVoice);
}

void __fastcall hkCalculateView(CBasePlayer* thisptr, void* edx, Vector& eyeOrigin, QAngle& eyeAngle, float& z_near, float& z_far, float& fov) {
	if (!thisptr || thisptr != Cheat.LocalPlayer)
		return oCalculateView(thisptr, edx, eyeOrigin, eyeAngle, z_near, z_far, fov);

	static const uintptr_t m_bUseNewAnimstate_offset = *(uintptr_t*)Utils::PatternScan("client.dll", "80 BE ? ? ? ? ? 0F 84 ? ? ? ? 83 BE ? ? ? ? ? 0F 84", 0x2);

	bool& m_bUseNewAnimstate = *(bool*)(thisptr + m_bUseNewAnimstate_offset);
	const bool backup = m_bUseNewAnimstate;

	m_bUseNewAnimstate = false;
	oCalculateView(thisptr, edx, eyeOrigin, eyeAngle, z_near, z_far, fov);
	m_bUseNewAnimstate = backup;
}

void __fastcall hkRenderSmokeOverlay(void* thisptr, void* edx, bool bPreViewModel) {
	if (!config.visuals.effects.removals->get(3))
		oRenderSmokeOverlay(thisptr, edx, bPreViewModel);
}

void __stdcall hkFX_FireBullets(
	CBaseCombatWeapon* weapon,
	int iPlayer,
	int nItemDefIndex,
	const Vector& vOrigin,
	const QAngle& vAngles,
	int	iMode,
	int iSeed,
	float fInaccuracy,
	float fSpread,
	float fAccuracyFishtail,
	float flSoundTime,
	int sound_type,
	float flRecoilIndex) // TODO: find correct signature
{
	oFX_FireBullets(weapon, iPlayer, nItemDefIndex, vOrigin, vAngles, iMode, iSeed, fInaccuracy, fSpread, fAccuracyFishtail, flSoundTime, sound_type, flRecoilIndex);
}

void __fastcall hkProcessMovement(IGameMovement* thisptr, void* edx, CBasePlayer* player, CMoveData* mv) {
	mv->bGameCodeMovedPlayer = false;
	oProcessMovement(thisptr, edx, player, mv);
}

int __fastcall hkLogDirect(void* loggingSystem, void* edx, int channel, int serverity, Color color, const char* text) {
	if (hook_info.console_log)
		return oLogDirect(loggingSystem, edx, channel, serverity, color, text);

	if (!config.misc.miscellaneous.filter_console->get())
		return oLogDirect(loggingSystem, edx, channel, serverity, color, text);

	return 0;
}

CNewParticleEffect* hkCreateNewParticleEffect(void* pDef, CBaseEntity* pOwner, Vector const& vecAggregatePosition, const char* pDebugName, int nSplitScreenUser) {
	CNewParticleEffect* result;

	__asm {
		mov edx, pDef
		push nSplitScreenUser
		push pDebugName
		push vecAggregatePosition
		push pOwner
		call oCreateNewParticleEffect
		mov result, eax
	}

	if (IEFFECTS::bCaptureEffect)
		IEFFECTS::pCapturedEffect = result;

	return result;
}

__declspec(naked) void hkCreateNewParticleEffect_proxy() {
	__asm
	{
		push edx
		call hkCreateNewParticleEffect
		pop edx
		retn
	}
}

bool __fastcall hkSVCMsg_VoiceData(CClientState* clientstate, void* edx, const CSVCMsg_VoiceData& msg) {
	if (NetMessages->OnVoiceDataRecieved(msg)) {
		return true;
	}

	return oSVCMsg_VoiceData(clientstate, edx, msg);
}

void __stdcall hkDrawStaticProps(void* thisptr, IClientRenderable** pProps, const void* pInstances, int count, bool bShadowDepth, bool drawVCollideWireframe) {
	hook_info.in_draw_static_props = true;
	oDrawStaticProps(thisptr, pProps, pInstances, count, bShadowDepth, drawVCollideWireframe);
	hook_info.in_draw_static_props = false;
}

bool __fastcall hkWriteUserCmdDeltaToBuffer(CInput* thisptr, void* edx, int slot, void* buf, int from, int to, bool isnewcommand) {
	if (!Cheat.InGame || !Cheat.LocalPlayer || !Cheat.LocalPlayer->IsAlive() || !Exploits->ShouldBreakLC())
		return oWriteUserCmdDeltaToBuffer(thisptr, edx, slot, buf, from, to, isnewcommand);

	if (from != -1)
		return true;

	auto p_new_commands = (int*)((DWORD)buf - 0x2C);
	auto p_backup_commands = (int*)((DWORD)buf - 0x30);
	auto new_commands = *p_new_commands;

	auto next_cmd_nr = ClientState->m_nLastOutgoingCommand + ClientState->m_nChokedCommands + 1;

	auto total_new_commands = (Exploits->GetExploitType() == CExploits::E_DoubleTap) ? 16 : 11;

	from = -1;

	// ������ ��� CL_SendMove ��� ���� ��(��� �� ����� CL_SendMove ����� ���� ��������� ���� ������� ��� �� ��������)
	auto CL_SendMove = [ ] ( )
	{
		using CL_SendMove_t = void( __fastcall* )( void );
		static CL_SendMove_t CL_SendMoveF = ( CL_SendMove_t )Utils::PatternScan( "engine.dll", "55 8B EC A1 ? ? ? ? 81 EC ? ? ? ? B9 ? ? ? ? 53 8B 98" );

		CL_SendMoveF( );
	};

	*p_new_commands = total_new_commands;
	*p_backup_commands = 0;

	for (to = next_cmd_nr - new_commands + 1; to <= next_cmd_nr; to++)
	{
		if (!oWriteUserCmdDeltaToBuffer(thisptr, edx, slot, buf, from, to, true))
			return false;

		from = to;
	}

	CUserCmd* last_real_cmd = Input->GetUserCmd(slot, from);
	CUserCmd from_cmd;

	if (last_real_cmd)
		memcpy(&from_cmd, last_real_cmd, sizeof(CUserCmd));

	CUserCmd to_cmd;
	memcpy(&to_cmd, &from_cmd, sizeof(CUserCmd));

	to_cmd.command_number++;
	to_cmd.tick_count += 200;

	for (int i = new_commands; i <= total_new_commands; i++)
	{
		static void* write_user_cmd = Utils::PatternScan("client.dll", "55 8B EC 83 E4 F8 51 53 56 8B D9 8B 0D");

		__asm
		{
			mov     ecx, buf
			mov     edx, to_cmd
			push    from_cmd
			call    write_user_cmd
			add     esp, 4
		}
		memcpy(&from_cmd, &to_cmd, sizeof(CUserCmd));
		to_cmd.command_number++;
		to_cmd.tick_count++;
	}

	return true;
}

bool __fastcall hkShouldDrawViewModel(void* thisptr, void* edx) {
	if (!Cheat.LocalPlayer || !Cheat.LocalPlayer->IsAlive() || config.visuals.effects.viewmodel_scope_alpha->get() == 0)
		return oShouldDrawViewModel(thisptr, edx);

	return true;
}

void __fastcall hkPerformScreenOverlay(void* viewrender, void* edx, int x, int y, int w, int h) {
	if (config.misc.miscellaneous.ad_block->get())
		return;

	oPerformScreenOverlay(viewrender, edx, x, y, w, h);
}

int __fastcall hkListLeavesInBox(void* ecx, void* edx, const Vector& mins, const Vector& maxs, unsigned int* list, int size) {
	static void* insert_into_tree = Utils::PatternScan("client.dll", "56 52 FF 50 18", 0x5);

	if (!config.visuals.chams.disable_model_occlusion->get() || _ReturnAddress() != insert_into_tree)
		return oListLeavesInBox(ecx, edx, mins, maxs, list, size);

	// get current renderable info from stack ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L1470 )
	auto info = *(RenderableInfo_t**)((uintptr_t)_AddressOfReturnAddress() + 0x14);
	if (!info || !info->m_pRenderable)
		return oListLeavesInBox(ecx, edx, mins, maxs, list, size);

	// check if disabling occulusion for players ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L1491 )
	auto base_entity = info->m_pRenderable->GetIClientUnknown()->GetBaseEntity();
	if (!base_entity || !base_entity->IsPlayer())
		return oListLeavesInBox(ecx, edx, mins, maxs, list, size);

	// fix render order, force translucent group ( https://www.unknowncheats.me/forum/2429206-post15.html )
	// AddRenderablesToRenderLists: https://i.imgur.com/hcg0NB5.png ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L2473 )
	info->m_Flags &= ~0x100;
	info->m_bRenderInFastReflection |= 0xC0;

	// extend world space bounds to maximum ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L707 )
	static const Vector map_min = Vector(-16384.0f, -16384.0f, -16384.0f);
	static const Vector map_max = Vector(16384.0f, 16384.0f, 16384.0f);
	auto count = oListLeavesInBox(ecx, edx, map_min, map_max, list, size);
	return count;
}

bool __fastcall hkInPrediction(IPrediction* ecx, void* edx) {
	static auto setup_bones = Utils::PatternScan("client.dll", "84 C0 74 0A F3 0F 10 05 ? ? ? ? EB 05");

	if (_ReturnAddress() == setup_bones)
		return false;

	return oInPrediction(ecx, edx);
}

void Hooks::Initialize() {
	oWndProc = (WNDPROC)(SetWindowLongPtr(FindWindowA("Valve001", nullptr), GWL_WNDPROC, (LONG_PTR)hkWndProc));

	ESP::RegisterCallback();
	Chams->LoadChams();
	SkinChanger->LoadKnifeModels();

	DirectXDeviceVMT = new VMT(DirectXDevice);
	SurfaceVMT = new VMT(Surface);
	ClientModeVMT = new VMT(ClientMode);
	PanelVMT = new VMT(VPanel);
	EngineVMT = new VMT(EngineClient);
	ModelRenderVMT = new VMT(ModelRender);
	ConVarVMT = new VMT(cvars.sv_cheats);
	ClientVMT = new VMT(Client);
	PredictionVMT = new VMT(Prediction);
	ModelCacheVMT = new VMT(MDLCache);
	KeyValuesVMT = new VMT(KeyValuesSystem);

	// vmt hooking for directx doesnt work for some reason
	oPresent = HookFunction<tPresent>(Utils::PatternScan("gameoverlayrenderer.dll", "55 8B EC 83 EC 4C 53"), hkPresent);
	//oReset = HookFunction<tReset>(Utils::PatternScan("d3d9.dll", "8B FF 55 8B EC 83 E4 F8 81 EC ? ? ? ? A1 ? ? ? ? 33 C4 89 84 24 ? ? ? ? 53 8B 5D 08 8B CB"), hkReset);

	while (!Menu->IsInitialized())
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

	DirectXDeviceVMT->Hook(16, hkReset);
	SurfaceVMT->Hook(67, hkLockCursor);
	ClientModeVMT->Hook(18, hkOverrideView);
	ClientModeVMT->Hook(44, hkDoPostScreenEffects);
	PanelVMT->Hook(41, hkPaintTraverse);
	EngineVMT->Hook(90, hkIsPaused);
	EngineVMT->Hook(93, hkIsHLTV);
	ModelRenderVMT->Hook(21, hkDrawModelExecute);
	ConVarVMT->Hook(13, hkGetBool);
	ClientVMT->Hook(37, hkFrameStageNotify);
	ClientVMT->Hook(11, hkHudUpdate);
	ClientVMT->Hook(22, hkCHLCCreateMove);
	//ModelCacheVMT->Hook(10 ,hkFindMdl);
	ClientVMT->Hook(7, hkLevelShutdown);
	PredictionVMT->Hook(19, hkRunCommand);
	KeyValuesVMT->Hook(2, hkAllocKeyValuesMemory);

	oUpdateClientSideAnimation = HookFunction<tUpdateClientSideAnimation>(Utils::PatternScan("client.dll", "55 8B EC 51 56 8B F1 80 BE ? ? ? ? ? 74"), hkUpdateClientSideAnimation);
	oDoExtraBoneProcessing = HookFunction<tDoExtraBoneProcessing>(Utils::PatternScan("client.dll", "55 8B EC 83 E4 F8 81 EC ? ? ? ? 53 56 8B F1 57 89 74 24 1C"), hkDoExtraBoneProcessing);
	oShouldSkipAnimationFrame = HookFunction<tShouldSkipAnimationFrame>(Utils::PatternScan("client.dll", "57 8B F9 8B 07 8B 80 ? ? ? ? FF D0 84 C0 75 02"), hkShouldSkipAnimationFrame);
	oStandardBlendingRules = HookFunction<tStandardBlendingRules>(Utils::PatternScan("client.dll", "55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 56 8B 75 08 57 8B F9 85 F6"), hkStandardBlendingRules);
	oBuildTransformations = HookFunction<tBuildTransformations>(Utils::PatternScan("client.dll", "55 8B EC 83 E4 F0 81 ? ? ? ? ? 56 57 8B F9 8B"), hkBuildTransformations);
	oSetUpLean = HookFunction<tSetUpLean>(Utils::PatternScan("client.dll", "55 8B EC 83 E4 F8 A1 ? ? ? ? 83 EC 20 F3"), hkSetUpLean);
	oSetupBones = HookFunction<tSetupBones>(Utils::PatternScan("client.dll", "55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 56 57"), hkSetupBones);
	oCL_Move = HookFunction<tCL_Move>(Utils::PatternScan("engine.dll", "55 8B EC 81 EC ? ? ? ? 53 56 8A F9 F3 0F 11 45 ? 8B 4D 04"), hkCL_Move);
	oPhysicsSimulate = HookFunction<tPhysicsSimulate>(Utils::PatternScan("client.dll", "56 8B F1 8B 8E ? ? ? ? 83 F9 FF 74 23 0F B7 C1 C1 E0 04 05 ? ? ? ?"), hkPhysicsSimulate);
	oClampBonesInBBox = HookFunction<tClampBonesInBBox>(Utils::PatternScan("client.dll", "55 8B EC 83 E4 F8 83 EC 70 56 57 8B F9 89 7C 24 38"), hkClampBonesInBBox);
	oCalculateView = HookFunction<tCalculateView>(Utils::PatternScan("client.dll", "55 8B EC 83 EC 14 53 56 57 FF 75 18"), hkCalculateView);
	oSendNetMsg = HookFunction<tSendNetMsg>(Utils::PatternScan("engine.dll", "55 8B EC 83 EC 08 56 8B F1 8B 4D 04 E8 ? ? ? ? 8B 86 ? ? ? ? 85 C0 74 24 48 83 F8 02 77 2C 83 BE ? ? ? ? ? 8D 8E ? ? ? ? 74 06 32 C0 84 C0"), hkSendNetMsg);
	oGetEyeAngles = HookFunction<tGetEyeAngles>(Utils::PatternScan("client.dll", "56 8B F1 85 F6 74 32"), hkGetEyeAngles);
	oRenderSmokeOverlay = HookFunction<tRenderSmokeOverlay>(Utils::PatternScan("client.dll", "55 8B EC 83 EC 30 80 7D 08 00"), hkRenderSmokeOverlay);
	oShouldInterpolate = HookFunction<tShouldInterpolate>(Utils::PatternScan("client.dll", "56 8B F1 E8 ? ? ? ? 3B F0"), hkShouldInterpolate);
	oPacketStart = HookFunction<tPacketStart>(Utils::PatternScan("engine.dll", "55 8B EC 8B 45 08 89 81 ? ? ? ? 8B 45 0C 89 81 ? ? ? ? 5D C2 08 00 CC CC CC CC CC CC CC 56"), hkPacketStart);
	oPacketEnd = HookFunction<tPacketEnd>(Utils::PatternScan("engine.dll", "56 8B F1 E8 ? ? ? ? 8B 8E ? ? ? ? 3B 8E ? ? ? ? 75 34"), hkPacketEnd);
	//oFX_FireBullets = HookFunction<tFX_FireBullets>(Utils::PatternScan("client.dll", "55 8B EC 83 E4 C0 F3 0F 10 45"), hkFX_FireBullets);
	oProcessMovement = HookFunction<tProcessMovement>(Utils::PatternScan("client.dll", "55 8B EC 83 E4 C0 83 EC 38 A1 ? ? ? ?"), hkProcessMovement);
	oLogDirect = HookFunction<tLogDirect>(Utils::PatternScan("tier0.dll", "55 8B EC 83 E4 F8 8B 45 08 83 EC 14 53 56 8B F1 57 85 C0 0F 88 ? ? ? ?"), hkLogDirect);
	oSetSignonState = HookFunction<tSetSignonState>(Utils::PatternScan("engine.dll", "55 8B EC 83 E4 F8 81 EC ? ? ? ? 53 56 57 FF 75 10"), hkSetSignonState);
	oCreateNewParticleEffect = HookFunction<tCreateNewParticleEffect>(Utils::PatternScan("client.dll", "55 8B EC 83 EC 0C 53 56 8B F2 89 75 F8 57"), hkCreateNewParticleEffect_proxy);
	oSVCMsg_VoiceData = HookFunction<tSVCMsg_VoiceData>(Utils::PatternScan("engine.dll", "55 8B EC 83 E4 F8 A1 ? ? ? ? 81 EC ? ? ? ? 53 56 8B F1 B9 ? ? ? ? 57 FF 50 34 8B 7D 08 85 C0 74 13 8B 47 08 40 50"), hkSVCMsg_VoiceData);
	oDrawStaticProps = HookFunction<tDrawStaticProps>(Utils::PatternScan("engine.dll", "55 8B EC 56 57 8B F9 8B 0D ? ? ? ? 8B B1 ? ? ? ? 85 F6 74 16 6A 04 6A 00 68"), hkDrawStaticProps);
	oWriteUserCmdDeltaToBuffer = HookFunction<tWriteUserCmdDeltaToBuffer>(Utils::PatternScan("client.dll", "55 8B EC 83 EC 68 53 56 8B D9 C7 45 ? ? ? ? ? 57 8D 4D 98"), hkWriteUserCmdDeltaToBuffer);
	oShouldDrawViewModel = HookFunction<tShouldDrawViewModel>(Utils::PatternScan("client.dll", "55 8B EC 51 57 E8"), hkShouldDrawViewModel);
	oPerformScreenOverlay = HookFunction<tPerformScreenOverlay>(Utils::PatternScan("client.dll", "55 8B EC 51 A1 ? ? ? ? 53 56 8B D9 B9 ? ? ? ? 57 89 5D FC FF 50 34 85 C0 75 36"), hkPerformScreenOverlay);
	oListLeavesInBox = HookFunction<tListLeavesInBox>(Utils::PatternScan("engine.dll", "55 8B EC 83 EC 18 8B 4D 0C"), hkListLeavesInBox);
	oInPrediction = HookFunction<tInPrediction>(Utils::PatternScan("client.dll", "8A 41 08 C3"), hkInPrediction);

	EventListner->Register();
}

void Hooks::End() {
	SetWindowLongPtr(FindWindowA("Valve001", nullptr), GWL_WNDPROC, (LONG_PTR)oWndProc);

	EventListner->Unregister();
	//Ragebot->TerminateThreads();
	Lua->UnloadAll();
	Menu->Release();

	DirectXDeviceVMT->UnHook(16);
	SurfaceVMT->UnHook(67);
	ClientModeVMT->UnHook(18);
	ClientModeVMT->UnHook(44);
	PanelVMT->UnHook(41);
	EngineVMT->UnHook(90);
	EngineVMT->UnHook(93);
	ModelRenderVMT->UnHook(21);
	ConVarVMT->UnHook(13);
	ClientVMT->UnHook(37);
	ClientVMT->UnHook(11);
	ClientVMT->UnHook(22);
	ClientVMT->UnHook(7);
	PredictionVMT->UnHook(19);
	//ClientVMT->UnHook(40);
	KeyValuesVMT->UnHook(2);
	ModelCacheVMT->UnHook(10);

	RemoveHook(oPresent, hkPresent);
	//RemoveHook(oReset, hkReset);
	RemoveHook(oUpdateClientSideAnimation, hkUpdateClientSideAnimation);
	RemoveHook(oDoExtraBoneProcessing, hkDoExtraBoneProcessing);
	RemoveHook(oShouldSkipAnimationFrame, hkShouldSkipAnimationFrame);
	RemoveHook(oStandardBlendingRules, hkStandardBlendingRules);
	RemoveHook(oBuildTransformations, hkBuildTransformations);
	RemoveHook(oSetUpLean, hkSetUpLean);
	RemoveHook(oSetupBones, hkSetupBones);
	RemoveHook(oCL_Move, hkCL_Move);
	RemoveHook(oPhysicsSimulate, hkPhysicsSimulate);
	RemoveHook(oClampBonesInBBox, hkClampBonesInBBox);
	RemoveHook(oCalculateView, hkCalculateView);
	RemoveHook(oSendNetMsg, hkSendNetMsg);
	RemoveHook(oGetEyeAngles, hkGetEyeAngles);
	RemoveHook(oRenderSmokeOverlay, hkRenderSmokeOverlay);
	RemoveHook(oShouldInterpolate, hkShouldInterpolate);
	RemoveHook(oPacketStart, hkPacketStart);
	RemoveHook(oPacketEnd, hkPacketEnd);
	//RemoveHook(oFX_FireBullets, hkFX_FireBullets);
	RemoveHook(oProcessMovement, hkProcessMovement);
	RemoveHook(oLogDirect, hkLogDirect);
	RemoveHook(oSetSignonState, hkSetSignonState);
	RemoveHook(oCreateNewParticleEffect, hkCreateNewParticleEffect_proxy);
	RemoveHook(oSVCMsg_VoiceData, hkSVCMsg_VoiceData);
	RemoveHook(oDrawStaticProps, hkDrawStaticProps);
	RemoveHook(oWriteUserCmdDeltaToBuffer, hkWriteUserCmdDeltaToBuffer);
	RemoveHook(oShouldDrawViewModel, hkShouldDrawViewModel);
	RemoveHook(oPerformScreenOverlay, hkPerformScreenOverlay);
	RemoveHook(oListLeavesInBox, hkListLeavesInBox);
	RemoveHook(oInPrediction, hkInPrediction);
}