#include "Indicators.h"

#include "../../SDK/Render.h"
#include "../../SDK/Config.h"
#include "../../SDK/Globals.h"

#include "../RageBot/DoubleTap.h"


CIndicators* Indicators = new CIndicators;

void CIndicators::AddIndicator(const std::string& text, const Color color) {
	const Vector2 text_size = Render->CalcTextSize(text, CalibriBold);

	Vector2 start = cursor_position;
	Vector2 end = cursor_position + text_size + Vector2(26, 8);

	Render->GradientBox(start, Vector2((start.x + end.x) * 0.5f, end.y), Color(0, 0, 0, 0), Color(0, 0, 0, 70), Color(0, 0, 0, 0), Color(0, 0, 0, 70));
	Render->GradientBox(Vector2((start.x + end.x) * 0.5f, start.y), end, Color(0, 0, 0, 70), Color(0, 0, 0, 0), Color(0, 0, 0, 70), Color(0, 0, 0, 0));

	Render->Text(text, start + Vector2(13, 4), color, CalibriBold, TEXT_DROPSHADOW);

	cursor_position.y -= 44;
}


void CIndicators::Draw() {
	if (!Cheat.InGame || !Cheat.LocalPlayer || !Cheat.LocalPlayer->IsAlive())
		return;

	cursor_position = Vector2(5, Cheat.ScreenSize.y * 0.75f);

	if (config.visuals.other_esp.indicators->get(4) && config.ragebot.aimbot.roll_resolver->get()) {
		AddIndicator("RR", Color(240));
	}

	if (config.visuals.other_esp.indicators->get(3)) {
		if (ctx.tickbase_shift == 0 && !(Cheat.LocalPlayer->m_fFlags() & FL_ONGROUND)) {
			AddIndicator("LC", ctx.breaking_lag_compensation ? Color(30, 220, 30) : Color(220, 30, 30));
		}
		else if (ctx.tickbase_shift > 0 && ctx.tickbase_shift == DoubleTap->TargetTickbaseShift() && config.ragebot.aimbot.doubletap_options->get(0)) {
			AddIndicator("LC", DoubleTap->IsDefensiveActive() ? Color(30, 220, 30) : Color(240));
		}
	}

	if (config.ragebot.aimbot.force_body_aim->get() && config.visuals.other_esp.indicators->get(2))
		AddIndicator("BODY", Color(240));

	if (config.ragebot.aimbot.minimum_damage_override_key->get() && config.visuals.other_esp.indicators->get(1))
		AddIndicator("DMG", Color(240));

	if (config.ragebot.aimbot.doubletap->get() && config.visuals.other_esp.indicators->get(0))
		AddIndicator("DT", Color(220, 30, 30).lerp(Color(240), DoubleTap->TargetTickbaseShift() > 0 ? float(ctx.tickbase_shift) / float(DoubleTap->TargetTickbaseShift()) : 0));
}