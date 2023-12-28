#pragma once
#include "../../Utils/Animation.h"
#include "../../SDK/Misc/Vector.h"

class CUserCmd;

class CAutoPeek {
	int block_buttons = 0;
	bool returning;
	Vector position;
	Animation circleAnimation;

public:

	void CreateMove();
	void Draw();
	void Return();
	void Disable() { returning = false; };
	bool IsReturning() { return returning; };
};

extern CAutoPeek* AutoPeek;