#pragma once

#include <string>
#include "../../SDK/Misc/Vector.h"
#include "../../SDK/Misc/Color.h"

class CIndicators {
	Vector2 cursor_position;

public:
	void AddIndicator(const std::string& text, const Color color);
	void Draw();
};

extern CIndicators* Indicators;