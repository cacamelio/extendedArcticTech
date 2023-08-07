#pragma once
#include <vector>
#include "../../SDK/Misc/Vector.h"
#include "../../SDK/Misc/QAngle.h"
#include "../../SDK/Misc/Matrix.h"
#include "../../SDK/Misc/CBasePlayer.h"
#include "Resolver.h"

class CBasePlayer;

struct LagRecord {
	CBasePlayer* player = nullptr;

	matrix3x4_t boneMatrix[128];
	matrix3x4_t aimMatrix[128];

	AnimationLayer animlayers[13];

	Vector m_vecOrigin;
	Vector m_vecVelocity;
	Vector m_vecMins = Vector(0, 0, 0);
	Vector m_vecMaxs = Vector(0, 0, 0);
	QAngle m_vecAbsAngles;

	QAngle m_viewAngle;

	float m_flSimulationTime = 0.f;
	float m_flDuckAmout = 0.f;
	float m_flDuckSpeed = 0.f;
	float m_flCycle = 0.f;
	float roll = 0.f;

	int m_nSequence = 0;
	int m_fFlags = 0;
	int m_nChokedTicks = 0;

	bool shifting_tickbase = false;
	bool breaking_lag_comp = false;
	bool exploiting = false;

	bool boneMatrixFilled = false;
	bool aimMatrixFilled = false;

	ResolverData_t resolver_data;
	CCSGOPlayerAnimationState unupdated_animstate;

	LagRecord* prev_record;
};

class CLagCompensation {
	std::array<std::deque<LagRecord>, 64> lag_records;
	float max_simulation_time[64];
	int last_update_tick[64];
public:

	__forceinline std::deque<LagRecord>& records(int index) { return lag_records[index]; };

	LagRecord* BackupData(CBasePlayer* player);

	void RecordDataIntoTrack(CBasePlayer* player, LagRecord* record);
	void BacktrackEntity(LagRecord* record, bool use_aim_matrix = false);
	void OnNetUpdate();
	void Reset(int index = -1);

	// Record helpers
	float GetLerpTime();
	bool ValidRecord(LagRecord* record);
};

extern CLagCompensation* LagCompensation;