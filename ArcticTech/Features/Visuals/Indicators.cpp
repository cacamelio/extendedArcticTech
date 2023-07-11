#include "Indicators.h"

#include "../../SDK/Render.h"
#include "../../SDK/Config.h"
#include "../../SDK/Globals.h"

#include "../RageBot/DoubleTap.h"


CIndicators* Indicators = new CIndicators;

void CIndicators::AddIndicator(const std::string& text, const Color color) {
	const Vector2 text_size = Render->CalcTextSize(text, CalibriBold);

	Vector2 start = cursor_position;
	Vector2 end = cursor_position + text_size + Vector2(12, 8);

	Render->GradientBox(start, Vector2((start.x + end.x) * 0.5f, end.y), Color(0, 0, 0, 0), Color(0, 0, 0, 70), Color(0, 0, 0, 0), Color(0, 0, 0, 70));
	Render->GradientBox(Vector2((start.x + end.x) * 0.5f, start.y), end, Color(0, 0, 0, 70), Color(0, 0, 0, 0), Color(0, 0, 0, 70), Color(0, 0, 0, 0));

	Render->Text(text, start + Vector2(6, 4), color, CalibriBold, TEXT_DROPSHADOW);

	cursor_position.y -= 44;
}

void CIndicators::Draw() {
	if (!Cheat.InGame || !Cheat.LocalPlayer || !Cheat.LocalPlayer->IsAlive())
		return;

	cursor_position = Vector2(12, Cheat.ScreenSize.y * 0.66f);

	if (config.ragebot.aimbot.minimum_damage_override_key->get() && config.visuals.other_esp.indicators->get(1))
		AddIndicator("DMG", Color(255, 255, 255));

	if (config.ragebot.aimbot.doubletap->get() && config.visuals.other_esp.indicators->get(0)) {
		AddIndicator("DT", Color(250, 12, 12).lerp(Color(255, 255, 255), DoubleTap->TargetTickbaseShift() > 0 ? float(ctx.tickbase_shift) / float(DoubleTap->TargetTickbaseShift()) : 0));
	}
}