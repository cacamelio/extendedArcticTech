#pragma once

class CMovement {
	int strafe_side = -1;
	float last_yaw = 0.f;
	int last_strafe_tick = 0;

public:
	void	QuickStop();
	void	AutoStrafe();
	void	CompensateThrowable();
	void	Rotate(float angle);
};

extern CMovement* Movement;