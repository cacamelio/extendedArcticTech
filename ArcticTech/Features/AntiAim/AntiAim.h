#pragma once
#include <Windows.h>

class CUserCmd;
class CBasePlayer;

struct LuaAntiAim_t {
	bool should_override_pitch = false;
	bool should_override_yaw = false;
	float pitch = 0.f;
	float yaw = 0.f;

	void override_pitch(float p) {
		should_override_pitch = true;
		pitch = p;
	}

	void override_yaw(float y) {
		should_override_yaw = true;
		yaw = y;
	}
};

class CAntiAim {
	int manualAngleState = 0;
	CBasePlayer* target;
	float notModifiedYaw = 0.f;

public:
	bool jitter;
	float realAngle;
	bool desyncing = false;
	bool freezetime = false;

	void LegMovement();
	void FakeLag();
	void Angles();
	void Desync();
	void SlowWalk();
	void FakeDuck();

	bool IsPeeking();

	int DesyncFreestand();
	void OnKeyPressed(WPARAM key);
	CBasePlayer* GetNearestTarget();
	float AtTargets();
};

extern CAntiAim* AntiAim;