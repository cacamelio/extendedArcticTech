#pragma once

class CUserCmd;
typedef void(__cdecl* CL_Move_t)(float, bool);

class CDoubleTap {
	bool teleport_next_tick = false;

public:
	bool shifting_tickbase = false;
	float last_teleport_time = 0.f;
	int target_tickbase_shift = 0;
	bool block_charge = false;
	int charged_command = 0;
	int charged_tickbase = 0;

	inline bool& IsShifting() { return shifting_tickbase; };
	inline float LastTeleportTime() { return last_teleport_time; };
	inline int& TargetTickbaseShift() { return target_tickbase_shift; };

	bool	ShouldCharge();
	int		MaxTickbaseShift();
	void	ForceTeleport();
	void	CreateMove();
	void	Run();
	void	HandleTeleport(CL_Move_t cl_move, float extra_samples);
	void	DefensiveDoubletap();
};

extern CDoubleTap* DoubleTap;