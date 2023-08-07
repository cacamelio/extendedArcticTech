#pragma once

class CUserCmd;
typedef void(__cdecl* CL_Move_t)(float, bool);

class CDoubleTap {
	bool teleport_next_tick = false;
	int defensive_ticks = 0;
	bool triggered_fast_throw = false;

public:
	bool shifting_tickbase = false;
	float last_teleport_time = 0.f;
	int target_tickbase_shift = 0;
	bool block_charge = false;
	int charged_command = 0;
	int charged_tickbase = 0;
	bool defensive_this_tick = false;
	bool allow_defensive = false;

	inline bool& IsShifting() { return shifting_tickbase; };
	inline float LastTeleportTime() { return last_teleport_time; };
	inline int& TargetTickbaseShift() { return target_tickbase_shift; };
	inline bool& DefenseiveThisTick() { return defensive_this_tick; };
	inline bool& AllowDefensive() { return allow_defensive; };

	bool	ShouldCharge();
	int		MaxTickbaseShift();
	void	ForceTeleport();
	void	Run();
	void	HandleTeleport(CL_Move_t cl_move, float extra_samples);
	void	DefensiveDoubletap();
	bool	ShouldBreakLC();
	bool	IsDefensiveActive();
};

extern CDoubleTap* DoubleTap;