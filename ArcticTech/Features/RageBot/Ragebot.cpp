#include "Ragebot.h"
#include "AutoWall.h"
#include "../Misc/Prediction.h"
#include "../../SDK/Interfaces.h"
#include "../../SDK/Globals.h"
#include "../../Utils/Utils.h"
#include <algorithm>
#include "../Misc/AutoPeek.h"
#include "Exploits.h"
#include "../../Utils/Console.h"
#include "../ShotManager/ShotManager.h"
#include "../Visuals/Chams.h"
#include "../Visuals/ESP.h"

void CRagebot::CalcSpreadValues() {
	for (int i = 0; i < 100; i++) {
		Utils::RandomSeed(i);

		float a = Utils::RandomFloat(0.f, 1.f);
		float b = Utils::RandomFloat(0.f, 6.2831853071795864f);
		float c = Utils::RandomFloat(0.f, 1.f);
		float d = Utils::RandomFloat(0.f, 6.2831853071795864f);

		spread_values[i].a = a;
		spread_values[i].bcos = std::cos(b);
		spread_values[i].bsin = std::sin(b);
		spread_values[i].c = c;
		spread_values[i].dcos = std::cos(d);
		spread_values[i].dsin = std::sin(d);
	}
}

weapon_settings_t CRagebot::GetWeaponSettings(int weaponId) {
	weapon_settings_t settings = config.ragebot.weapons.global;

	switch (weaponId) {
	case Scar20:
	case G3SG1:
		settings = config.ragebot.weapons.autosniper;
		break;
	case Ssg08:
		settings = config.ragebot.weapons.scout;
		break;
	case Awp:
		settings = config.ragebot.weapons.awp;
		break;
	case Deagle:
		settings = config.ragebot.weapons.deagle;
		break;
	case Revolver:
		settings = config.ragebot.weapons.revolver;
		break;
	case Fiveseven:
	case Glock:
	case Usp_s:
	case Tec9:
		settings = config.ragebot.weapons.pistol;
		break;
	}

	return settings;
}

float CRagebot::CalcMinDamage(CBasePlayer* player) {
	int minimum_damage = settings.minimum_damage->get();
	if (config.ragebot.aimbot.minimum_damage_override_key->get())
		minimum_damage = settings.minimum_damage_override->get();

	if (minimum_damage >= 100) {
		return minimum_damage - 100 + player->m_iHealth();
	}
	else {
		return min(minimum_damage, player->m_iHealth() + 1);
	}
}

void CRagebot::AutoStop() {
	Vector vec_speed = Cheat.LocalPlayer->m_vecVelocity();
	QAngle direction = Math::VectorAngles(vec_speed);

	float target_speed = ctx.active_weapon->MaxSpeed() * 0.2f;

	if (vec_speed.LengthSqr() < target_speed * target_speed + 9.f && !settings.auto_stop->get(0)) {
		float cmd_speed = Math::Q_sqrt(ctx.cmd->forwardmove * ctx.cmd->forwardmove + ctx.cmd->sidemove * ctx.cmd->sidemove);
	
		if (cmd_speed > target_speed) {
			float factor = target_speed / cmd_speed;
			ctx.cmd->forwardmove *= factor;
			ctx.cmd->sidemove *= factor;
		}

		return;
	}

	QAngle view; EngineClient->GetViewAngles(&view);
	direction.yaw = view.yaw - direction.yaw;
	direction.Normalize();

	Vector forward;
	Math::AngleVectors(direction, forward);

	float wish_speed = std::clamp(vec_speed.Q_Length2D(), 0.f, 450.f);

	if (!(Cheat.LocalPlayer->m_fFlags() & FL_ONGROUND)) {
		static ConVar* sv_airaccelerate = CVar->FindVar("sv_airaccelerate");

		wish_speed = std::clamp((vec_speed.Q_Length2D() * 0.93f) / (sv_airaccelerate->GetFloat() * GlobalVars->frametime), 0.f, 450.f);
	}

	Vector nigated_direction = forward * -wish_speed;

	ctx.cmd->sidemove = nigated_direction.y;
	ctx.cmd->forwardmove = nigated_direction.x;
}

float CRagebot::FastHitchance(LagRecord* target, float inaccuracy, int hitbox_radius) {
	if (inaccuracy == -1.f)
		inaccuracy = std::tan(EnginePrediction->WeaponInaccuracy());

	return min(hitbox_radius / ((ctx.shoot_position - (target->m_vecOrigin + Vector(0, 0, 32))).Q_Length() * inaccuracy), 1.f);
}

float CRagebot::CalcHitchance(QAngle angles, LagRecord* target, int damagegroup) {
	Vector forward, right, up;
	Math::AngleVectors(angles, forward, right, up);

	int hits = 0;

	for (int i = 0; i < 100; i++)
	{
		SpreadValues_t& s_val = spread_values[i];

		float a = s_val.a;
		float c = s_val.c;

		float inaccuracy = a * EnginePrediction->WeaponInaccuracy();
		float spread = c * EnginePrediction->WeaponSpread();

		if (ctx.active_weapon->m_iItemDefinitionIndex() == 64)
		{
			a = 1.f - a * a;
			a = 1.f - c * c;
		}

		Vector direction = forward + (right * (s_val.bcos * inaccuracy + s_val.dcos * spread)) + (up * (s_val.bsin * inaccuracy + s_val.dsin * spread));
		direction.Q_Normalized();

		if (!EngineTrace->RayIntersectPlayer(ctx.shoot_position, ctx.shoot_position + (direction * 8192.f), target->player, target->clamped_matrix, damagegroup))
			continue;

			hits++;
	}

	return hits * 0.01f;
}

void CRagebot::FindTargets() {
	targets.clear();

	for (int i = 0; i < ClientState->m_nMaxClients; i++) {
		CBasePlayer* player = reinterpret_cast<CBasePlayer*>(EntityList->GetClientEntity(i));

		if (!player || !player->IsAlive() || player->IsTeammate() || player->m_bDormant() || player->m_bGunGameImmunity())
			continue;

		targets.emplace_back(player);
	}
}

bool CRagebot::CompareRecords(LagRecord* a, LagRecord* b) {
	const Vector vec_diff = a->m_vecOrigin - b->m_vecOrigin;

	if (vec_diff.LengthSqr() > 1.f)
		return false;

	QAngle angle_diff = a->m_angEyeAngles - b->m_angEyeAngles;
	angle_diff.Normalize();

	if (angle_diff.yaw > 90.f)
		return false;

	if (angle_diff.pitch > 10.f)
		return false;

	if (a->breaking_lag_comp != b->breaking_lag_comp)
		return false;

	if (a->shifting_tickbase != b->shifting_tickbase)
		return false;

	return true;
}

std::vector<LagRecord*> CRagebot::SelectRecords(CBasePlayer* player){
	std::vector<LagRecord*> target_records;
	auto& records = LagCompensation->records(player->EntIndex());

	if (records.empty())
		return target_records;

	LagRecord* last_valid_record{ nullptr };
	for (auto i = records.rbegin(); i != records.rend(); i = std::next(i)) {
		const auto record = &*i;
		if (!LagCompensation->ValidRecord(record)) {
			if (record->breaking_lag_comp)
				break;

			continue;
		}

		if (config.ragebot.aimbot.extended_backtrack->get()) {
			target_records.emplace_back(record);
			continue;
		}

		if (target_records.empty()) {
			target_records.emplace_back(record);
		} else {
			if (record->shooting)
				target_records.emplace_back(record);

			last_valid_record = record;
		}
	}

	if (!config.ragebot.aimbot.extended_backtrack->get() && last_valid_record && last_valid_record != &records.back()) {
		target_records.emplace_back(last_valid_record);
	}

	if (target_records.empty()) {
		// TODO: should be extrapolation here

		if (!records.back().shifting_tickbase)
			target_records.push_back(&records.back());
	}

	return target_records;
}

void CRagebot::GetMultipoints(LagRecord* record, int hitbox_id, float scale, std::vector<AimPoint_t>& points) {
	studiohdr_t* studiomodel = ModelInfoClient->GetStudioModel(record->player->GetModel());

	if (!studiomodel)
		return;

	mstudiobbox_t* hitbox = studiomodel->GetHitboxSet(record->player->m_nHitboxSet())->GetHitbox(hitbox_id);

	if (!hitbox)
		return;

	if (hitbox->flCapsuleRadius <= 0) // do not scan multipoints for feet
		return;

	matrix3x4_t boneMatrix = record->clamped_matrix[hitbox->bone];

	Vector mins, maxs;
	Math::VectorTransform(hitbox->bbmin, boneMatrix, &mins);
	Math::VectorTransform(hitbox->bbmax, boneMatrix, &maxs);

	Vector center = (mins + maxs) * 0.5f;
	Vector sideDirection = center - mins;
	const float width = sideDirection.Normalize() + hitbox->flCapsuleRadius;
	const float radius = hitbox->flCapsuleRadius;

	Vector verts[]{
		Vector(0, radius * scale, 0),
		Vector(0, -radius * scale, 0),
		Vector(0, 0, width * scale),
		Vector(0, 0, -width * scale),
	};

	//if (hitbox_id == HITBOX_HEAD) {
	//	verts[2].z = radius * scale;
	//	verts[3].z = -radius * scale;
	//}

	for (const auto& vert : verts)
		points.push_back(AimPoint_t{ Math::VectorTransform(vert, boneMatrix), hitbox_id, true });

	if (hitbox_id == HITBOX_HEAD)
		points.push_back(AimPoint_t{ Vector(center.x, center.y, center.z + width * scale * 0.89f), hitbox_id, true });
}

int CRagebot::CalcPointsCount() {
	int count = 0;

	for (int hitbox = 0; hitbox < HITBOX_MAX; hitbox++) {
		if (!hitbox_enabled(hitbox))
			continue;

		count++;

		if (multipoints_enabled(hitbox)) {
			if (hitbox == HITBOX_HEAD)
				count += 5;
			else if (hitbox >= HITBOX_PELVIS && hitbox <= HITBOX_UPPER_CHEST)
				count += 4;
		}
	}

	return count;
}

bool CRagebot::IsArmored(int hitbox) {
	switch (hitbox)
	{
	case HITBOX_LEFT_CALF:
	case HITBOX_LEFT_FOOT:
	case HITBOX_LEFT_THIGH:
	case HITBOX_RIGHT_THIGH:
	case HITBOX_RIGHT_FOOT:
	case HITBOX_RIGHT_CALF:
		return false;
	default:
		return true;
	}
}

std::vector<AimPoint_t> CRagebot::SelectPoints(LagRecord* record) {
	std::vector<AimPoint_t> points;

	points.reserve(CalcPointsCount());

	for (int hitbox = 0; hitbox < HITBOX_MAX; hitbox++) {
		if (!hitbox_enabled(hitbox) || (cvars.mp_damage_headshot_only->GetInt() > 0 && hitbox != HITBOX_HEAD))
			continue;

		float max_possible_damage = ctx.weapon_info->iDamage;
		record->player->ScaleDamage(HitboxToHitgroup(hitbox), ctx.weapon_info, max_possible_damage);

		if (!settings.auto_stop->get(1) && max_possible_damage < CalcMinDamage(record->player))
			continue;

		points.push_back(AimPoint_t({ record->player->GetHitboxCenter(hitbox, record->clamped_matrix), hitbox, false }));

		if (multipoints_enabled(hitbox))
			GetMultipoints(record, hitbox, hitbox == HITBOX_HEAD ? settings.head_point_scale->get() * 0.01f : settings.body_point_scale->get() * 0.01f, points);
	}

	return points;
}

ScannedPoint_t CRagebot::SelectBestPoint(ScannedTarget_t target) {
	ScannedPoint_t best_body_point;
	ScannedPoint_t best_head_point;

	for (const auto& point : target.points) {
		if (point.hitbox == HITBOX_HEAD) {
			if (best_head_point.damage < target.minimum_damage || (point.priotity > best_head_point.priotity && point.damage > target.minimum_damage))
				best_head_point = point;
		}
		else if (point.hitbox != HITBOX_HEAD) {
			if (best_body_point.damage < target.minimum_damage || (point.priotity > best_body_point.priotity && point.damage > target.minimum_damage))
				best_body_point = point;
		}
	}

	int& delay = delayed_ticks[target.player->EntIndex()];
	if (best_body_point.damage > 1.f && best_head_point.damage > 1.f && settings.delay_shot->get() > 0) {
		if (delay < settings.delay_shot->get())
			best_head_point.damage = -1.f;
		delay++;
	}
	else {
		delay = 0;
	}

	if (best_body_point.damage < target.player->m_iHealth() && best_head_point.damage > best_body_point.damage && best_head_point.record->shooting)
		return best_head_point;

	if ((target.player != last_target && ctx.tickbase_shift == 0 && config.ragebot.aimbot.doubletap->get() && !config.ragebot.aimbot.force_teleport->get())) {
		if (best_body_point.damage > target.player->m_iHealth())
			return best_body_point;

		if (best_head_point.damage < target.player->m_iHealth())
			return best_body_point;

		return best_head_point;
	}

	if (best_body_point.damage > target.minimum_damage)
		return best_body_point;

	return best_head_point;
}

void CRagebot::CreateThreads() {
	for (int i = 0; i < GetRagebotThreads(); i++) {
		threads[i] = Utils::CreateSimpleThread(&CRagebot::ThreadScan, (void*)i);
		inited_threads++;
	}
}

void CRagebot::TerminateThreads() {
	remove_threads = true;
	scan_condition.notify_all();

	for (int i = 0; i < inited_threads; i++) {
		Utils::ThreadJoin(threads[i], 1000000);
		threads[i] = 0;
	}
	inited_threads = 0;
	remove_threads = false;
}

void CRagebot::ScanTargets() {
	scanned_targets.clear();
	scanned_targets.reserve(targets.size());

	selected_targets = targets.size();
	scan_condition.notify_one();

	{
		std::unique_lock<std::mutex> lock(target_mutex);
		result_condition.wait(lock, [this]() { return scanned_targets.size() == selected_targets; });
	}
}

uintptr_t CRagebot::ThreadScan(int threadId) {
	while (true) {
		std::unique_lock<std::mutex> scan_lock(Ragebot->target_mutex);

		Ragebot->scan_condition.wait(scan_lock, []() { return !Ragebot->targets.empty() || Ragebot->remove_threads; });

		if (Ragebot->remove_threads) {
			break;
		}

		CBasePlayer* target = Ragebot->targets.back();
		Ragebot->targets.pop_back();
		scan_lock.unlock();
		
		Ragebot->scan_condition.notify_one();

		ScannedTarget_t scan = Ragebot->ScanTarget(target);

		std::unique_lock<std::mutex> lock(Ragebot->scan_mutex);
		Ragebot->scanned_targets.push_back(scan);

		if (Ragebot->scanned_targets.size() >= Ragebot->selected_targets)
			Ragebot->result_condition.notify_one();
	}

	return 0;
}

ScannedTarget_t CRagebot::ScanTarget(CBasePlayer* target) {
	std::vector<LagRecord*> records = SelectRecords(target);

	if (records.empty())
		return ScannedTarget_t{};

	float minimum_damage = CalcMinDamage(target);

	ScannedTarget_t result;
	result.player = target;
	result.minimum_damage = minimum_damage;

	LagRecord* backup_record = LagCompensation->BackupData(target);

	for (int i = 0; i < records.size(); i++) {
		LagRecord* record = records[i];

		LagCompensation->BacktrackEntity(record, false);
		record->BuildMatrix();

		memcpy(record->player->GetCachedBoneData().Base(), record->clamped_matrix, sizeof(matrix3x4_t) * record->player->GetCachedBoneData().Count());

		std::vector<AimPoint_t> points = SelectPoints(record);


		for (const auto& point : points) {
			if (config.ragebot.aimbot.show_aimpoints->get())
				DebugOverlay->AddBoxOverlay(point.point, Vector(-1, -1, -1), Vector(1, 1, 1), QAngle(0, 0, 0), 255, 255, 255, 200, GlobalVars->interval_per_tick * 2);

			FireBulletData_t bullet;
			if (!AutoWall->FireBullet(Cheat.LocalPlayer, ctx.shoot_position, point.point, bullet, target))
				continue;

			int priority = 2;
			bool jitter_safe = false;
			if (point.multipoint)
				priority--;

			if (EngineTrace->RayIntersectPlayer(ctx.shoot_position, point.point, target, record->opposite_matrix, HitboxToDamagegroup(point.hitbox)) && 
				EngineTrace->RayIntersectPlayer(ctx.shoot_position, point.point, target, record->clamped_matrix, HitboxToDamagegroup(point.hitbox))) {

				priority += 2;
				jitter_safe = true;
			}

			if (point.hitbox >= HITBOX_PELVIS && point.hitbox <= HITBOX_UPPER_CHEST)
				priority += 2;

			if (record->shooting)
				priority += 2;

			result.points.emplace_back(ScannedPoint_t{
				record,
				point.point,
				point.hitbox,
				priority,
				bullet.damage,
				jitter_safe,
				bullet.impacts
			});

			if (bullet.damage > 5.f)
				Exploits->block_charge = true;
		}
	}

	result.best_point = SelectBestPoint(result);
	if (!result.best_point.record || result.best_point.damage < 5) {
		LagCompensation->BacktrackEntity(backup_record);
		delete backup_record;
		return result;
	}

	result.angle = Math::VectorAngles_p(result.best_point.point - ctx.shoot_position);
	result.hitchance = CalcHitchance(result.angle, result.best_point.record, HitboxToDamagegroup(result.best_point.hitbox));

	LagCompensation->BacktrackEntity(backup_record);
	delete backup_record;

	return result;
}

void CRagebot::Run() {
	if (!config.ragebot.aimbot.enabled->get() || !ctx.active_weapon)
		return;

	AutoRevolver();

	if (Exploits->IsShifting()) {
		if (doubletap_stop) {
			float current_vel = Math::Q_sqrt(ctx.cmd->sidemove * ctx.cmd->sidemove + ctx.cmd->forwardmove + ctx.cmd->forwardmove);
			const float max_speed = doubletap_stop_speed * 0.5f;

			if (current_vel > 1.f) {
				float factor = max_speed / current_vel;
				ctx.cmd->sidemove *= factor;
				ctx.cmd->forwardmove *= factor;
			}
		}
		return;
	}

	if (Cheat.LocalPlayer->m_fFlags() & FL_FROZEN || GameRules()->IsFreezePeriod())
		return;

	if (ctx.active_weapon->IsGrenade())
		return;
	
	if (ctx.active_weapon->m_iItemDefinitionIndex() == Taser) {
		Zeusbot();
		return;
	}

	if (!ctx.active_weapon->ShootingWeapon())
		return;

	settings = GetWeaponSettings(ctx.active_weapon->m_iItemDefinitionIndex());

	if (config.ragebot.aimbot.dormant_aim->get())
		DormantAimbot();

	doubletap_stop = false;

	FindTargets();

	hook_info.disable_interpolation = true;
	ScanTargets();
	hook_info.disable_interpolation = false;

	ScannedTarget_t best_target;
	bool should_autostop = false;

	bool local_on_ground = Cheat.LocalPlayer->m_fFlags() & FL_ONGROUND && EnginePrediction->m_fFlags & FL_ONGROUND;
	int m_nWeaponMode = Cheat.LocalPlayer->m_bIsScoped() ? 1 : 0;
	float min_jump_inaccuracy_tan = 0.f;

	if (settings.auto_stop->get(3) && !local_on_ground) { // superior "dynamic autostop"
		float flInaccuracyJumpInitial = ctx.weapon_info->_flInaccuracyUnknown;

		float fSqrtMaxJumpSpeed = Math::Q_sqrt(cvars.sv_jump_impulse->GetFloat());
		float fSqrtVerticalSpeed = Math::Q_sqrt(abs(ctx.local_velocity.z) * 0.3f);

		float flAirSpeedInaccuracy = Math::RemapVal(fSqrtVerticalSpeed,
			fSqrtMaxJumpSpeed * 0.25f,
			fSqrtMaxJumpSpeed,
			0.0f,
			flInaccuracyJumpInitial);

		if (flAirSpeedInaccuracy < 0)
			flAirSpeedInaccuracy = 0;
		else if (flAirSpeedInaccuracy > (2.f * flInaccuracyJumpInitial))
			flAirSpeedInaccuracy = 2.f * flInaccuracyJumpInitial;

		min_jump_inaccuracy_tan = std::tan(ctx.weapon_info->flInaccuracyStand[m_nWeaponMode] + ctx.weapon_info->flInaccuracyJump[m_nWeaponMode] + flAirSpeedInaccuracy);
	}

	for (const auto& target : scanned_targets) {
		if (target.best_point.damage > target.minimum_damage && ctx.cmd->command_number - last_target_shot < 150 && target.player == last_target) {
			if (target.hitchance > settings.hitchance->get() * 0.01f) {
				best_target = target;
				break;
			}
			else {
				if (local_on_ground || (settings.auto_stop->get(3) && FastHitchance(target.best_point.record, min_jump_inaccuracy_tan) >= settings.hitchance->get() * 0.009f)) {
					should_autostop = true;
				}
			}
		}

		if (target.hitchance > settings.hitchance->get() * 0.01f && target.best_point.damage > max(best_target.best_point.damage, target.minimum_damage))
			best_target = target;

		if (target.best_point.damage > target.minimum_damage) {
			if (local_on_ground || (settings.auto_stop->get(3) && FastHitchance(target.best_point.record, min_jump_inaccuracy_tan) >= settings.hitchance->get() * 0.009f)) {
				should_autostop = true;
			}

			if (settings.auto_scope->get() && !Cheat.LocalPlayer->m_bIsScoped() && !Cheat.LocalPlayer->m_bResumeZoom() && ctx.weapon_info->nWeaponType == WEAPONTYPE_SNIPER)
				ctx.cmd->buttons |= IN_ATTACK2;
		}

		if (target.best_point.damage > 15) {
			if (settings.auto_stop->get(1) && (local_on_ground || (settings.auto_stop->get(3) && FastHitchance(target.best_point.record, min_jump_inaccuracy_tan) >= settings.hitchance->get() * 0.01f))) {
				should_autostop = true;
			}
			Exploits->block_charge = true;
		}
	}

	if (settings.auto_stop->get(2) && !ctx.active_weapon->CanShoot() && ctx.active_weapon->m_iItemDefinitionIndex() != Revolver)
		return;

	if (should_autostop)
		AutoStop();

	if (!best_target.player || !ctx.active_weapon->CanShoot()) {
		if (best_target.player && ctx.active_weapon->m_iItemDefinitionIndex() == Revolver)
			ctx.cmd->buttons |= IN_ATTACK;
		return;
	}

	ctx.cmd->tick_count = TIME_TO_TICKS(best_target.best_point.record->m_flSimulationTime + LagCompensation->GetLerpTime());
	ctx.cmd->viewangles = best_target.angle - Cheat.LocalPlayer->m_aimPunchAngle() * cvars.weapon_recoil_scale->GetFloat();
	ctx.cmd->buttons |= IN_ATTACK;

	if (!settings.auto_stop->get(2) && ctx.tickbase_shift > 0) {
		doubletap_stop = true;
		doubletap_stop_speed = ctx.active_weapon->MaxSpeed() * 0.25f;
	}

	if (Exploits->GetExploitType() == CExploits::E_DoubleTap)
		Exploits->ForceTeleport();
	else if (Exploits->GetExploitType() == CExploits::E_HideShots)
		Exploits->HideShot();
	if (!config.antiaim.misc.fake_duck->get())
		ctx.send_packet = true;

	AutoPeek->returning = true;
	last_target = best_target.player;
	last_target_shot = ctx.cmd->command_number;

	if (config.visuals.effects.client_impacts->get()) {
		for (const auto& impact : best_target.best_point.impacts)
			DebugOverlay->AddBoxOverlay(impact, Vector(-1, -1, -1), Vector(1, 1, 1), QAngle(),
				config.visuals.effects.client_impacts_color->get().r,
				config.visuals.effects.client_impacts_color->get().g,
				config.visuals.effects.client_impacts_color->get().b,
				config.visuals.effects.client_impacts_color->get().a,
				config.visuals.effects.impacts_duration->get());
	}

	LagRecord* record = best_target.best_point.record;

	if (config.misc.miscellaneous.logs->get(1)) {
		Console->Log(std::format("shot at {}'s {} [dmg: {:d}] [hc: {}] [bt: {}] [res: {:.1f}deg safe: {}]", 
			best_target.player->GetName(), 
			GetHitboxName(best_target.best_point.hitbox), 
			static_cast<int>(best_target.best_point.damage), 
			static_cast<int>(best_target.hitchance * 100), 
			TIME_TO_TICKS(best_target.player->m_flSimulationTime() - record->m_flSimulationTime),
			record->resolver_data.side * best_target.player->GetMaxDesyncDelta(),
			best_target.best_point.jitter_safe
		));
	}

	ShotManager->AddShot(ctx.shoot_position, best_target.best_point.point, best_target.best_point.damage, HitboxToDamagegroup(best_target.best_point.hitbox), best_target.hitchance, best_target.best_point.record);
	if (config.visuals.chams.shot_chams->get()) {
		Chams->AddShotChams(best_target.best_point.record);
	}
}

void CRagebot::Zeusbot() {
	const float inaccuracy_tan = std::tan(EnginePrediction->WeaponInaccuracy());

	if (!ctx.active_weapon->CanShoot())
		return;

	for (int i = 0; i < ClientState->m_nMaxClients; i++) {
		CBasePlayer* player = reinterpret_cast<CBasePlayer*>(EntityList->GetClientEntity(i));
		auto& records = LagCompensation->records(i);

		if (!player || player->IsTeammate() || !player->IsAlive() || player->m_bDormant())
			continue;

		LagRecord* backup_record = LagCompensation->BackupData(player);

		for (auto i = records.rbegin(); i != records.rend(); i = std::next(i)) {
			const auto record = &*i;
			if (!LagCompensation->ValidRecord(record)) {
				if (record->breaking_lag_comp)
					break;

				continue;
			}

			float distance = ((record->m_vecOrigin + (record->m_vecMaxs + record->m_vecMins) * 0.5f) - ctx.shoot_position).LengthSqr();

			if (distance > 165 * 165)
				continue;

			memcpy(record->clamped_matrix, record->bone_matrix, sizeof(matrix3x4_t) * 128);
			LagCompensation->BacktrackEntity(record, true, true);
			const Vector points[]{
				player->GetHitboxCenter(HITBOX_STOMACH),
				player->GetHitboxCenter(HITBOX_CHEST),
				player->GetHitboxCenter(HITBOX_UPPER_CHEST),
				player->GetHitboxCenter(HITBOX_LEFT_UPPER_ARM),
				player->GetHitboxCenter(HITBOX_RIGHT_UPPER_ARM)
			};

			for (const auto& point : points) {
				CGameTrace trace = EngineTrace->TraceRay(ctx.shoot_position, point, MASK_SHOT | CONTENTS_GRATE, Cheat.LocalPlayer);

				if (trace.hit_entity != player)
					continue;

				QAngle angle = Math::VectorAngles(point - ctx.shoot_position);

				float hitchance = min(7.f / ((ctx.shoot_position - point).Q_Length() * inaccuracy_tan), 1.f);

				if (hitchance < 0.5f)
					continue;

				if (config.visuals.effects.client_impacts->get()) {
					Color col = config.visuals.effects.client_impacts_color->get();
					DebugOverlay->AddBoxOverlay(point, Vector(-1, -1, -1), Vector(1, 1, 1), QAngle(), col.r, col.g, col.b, col.a, config.visuals.effects.impacts_duration->get());
				}

				ctx.cmd->viewangles = angle;
				ctx.cmd->buttons |= IN_ATTACK;
				ctx.cmd->tick_count = TIME_TO_TICKS(record->m_flSimulationTime + LagCompensation->GetLerpTime());

				Chams->AddShotChams(record);

				Console->Log(std::format("shot at {} [hc: {}] [bt: {}]", player->GetName(), (int)(hitchance * 100.f), TIME_TO_TICKS(player->m_flSimulationTime() - record->m_flSimulationTime)));

				LagCompensation->BacktrackEntity(backup_record, true, false);
				delete backup_record;
				return;
			}
		}

		LagCompensation->BacktrackEntity(backup_record, true, false);
		delete backup_record;
	}
}

void CRagebot::AutoRevolver() {
	if (ctx.cmd->buttons & IN_ATTACK)
		return;

	if (ctx.active_weapon->m_iItemDefinitionIndex() != Revolver)
		return;

	ctx.cmd->buttons &= ~IN_ATTACK2;

	static float next_cock_time = 0.f;

	if (ctx.active_weapon->m_flPostponeFireReadyTime() > GlobalVars->curtime) {
		if (GlobalVars->curtime > next_cock_time)
			ctx.cmd->buttons |= IN_ATTACK;
	}
	else {
		next_cock_time = GlobalVars->curtime + 0.25f;
	}
}

void CRagebot::DormantAimbot() {
	const float inaccuracy_tan = std::tan(EnginePrediction->WeaponInaccuracy());

	if (!ctx.active_weapon->CanShoot() && settings.auto_stop->get(2))
		return;

	for (int i = 0; i < ClientState->m_nMaxClients; i++) {
		CBasePlayer* player = reinterpret_cast<CBasePlayer*>(EntityList->GetClientEntity(i));

		if (!player || !player->m_bDormant() || !player->IsAlive() || player->IsTeammate())
			continue;

		if (EnginePrediction->curtime() - ESPInfo[i].m_flLastUpdateTime > 5.f)
			continue;

		Vector shoot_target = player->m_vecOrigin() + Vector(0, 0, 36);

		FireBulletData_t bullet;
		if (!AutoWall->FireBullet(Cheat.LocalPlayer, ctx.shoot_position, shoot_target, bullet) || bullet.damage < CalcMinDamage(player))
			continue;

		float hc = min(7.f / ((ctx.shoot_position - shoot_target).Q_Length() * inaccuracy_tan), 1.f);

		AutoStop();
		if (hc * 100.f < settings.hitchance->get() || !ctx.active_weapon->CanShoot())
			continue;

		ctx.cmd->viewangles = Math::VectorAngles_p(shoot_target - ctx.shoot_position) - Cheat.LocalPlayer->m_aimPunchAngle() * cvars.weapon_recoil_scale->GetFloat();
		ctx.cmd->buttons |= IN_ATTACK;

		if (config.visuals.effects.client_impacts->get()) {
			for (const auto& impact : bullet.impacts)
				DebugOverlay->AddBoxOverlay(impact, Vector(-1, -1, -1), Vector(1, 1, 1), QAngle(),
					config.visuals.effects.client_impacts_color->get().r,
					config.visuals.effects.client_impacts_color->get().g,
					config.visuals.effects.client_impacts_color->get().b,
					config.visuals.effects.client_impacts_color->get().a,
					config.visuals.effects.impacts_duration->get());
		}

		if (Exploits->GetExploitType() == CExploits::E_DoubleTap)
			Exploits->ForceTeleport();
		else if (Exploits->GetExploitType() == CExploits::E_HideShots)
			Exploits->HideShot();
		if (!config.antiaim.misc.fake_duck->get())
			ctx.send_packet = true;

		AutoPeek->returning = true;

		Console->Log(std::format("shot at {} [dmg: {}] [hc: {}%] [da]", player->GetName(), bullet.damage, int(hc * 100)));
		break;
	}
}

CRagebot* Ragebot = new CRagebot;