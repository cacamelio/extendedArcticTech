#pragma once

#include <vector>

#include "../../SDK/Misc/Vector.h"
#include "../../SDK/Misc/QAngle.h"


class CBasePlayer;
class IGameEvent;
struct RegisteredShot_t;
struct ClientShot_t;
struct LagRecord;

struct Impact_t {
	CBasePlayer* player;
	Vector position;
	int tick;
};

struct RegisteredShot_t {
	Vector				position;
	QAngle				angle;
	int					tick = 0;
	short				weapon_id = 0;
	std::vector<Impact_t>impacts;
	CBasePlayer*		hit_player = nullptr;
	int					damagegroup = -1;
	bool				open = false;
};

struct RagebotShot_t {
	Vector shoot_pos;
	QAngle angle;
	CBasePlayer* target;
	LagRecord* record;
	int target_damagegroup;
};

class CShotManager {
	std::vector<RegisteredShot_t>	m_RegisteredShots;
	std::vector<Impact_t>			m_UnprocessedImpacts;

public:
	void	OnCreateMove();
	void	OnNetUpdate();
	void	OnEvent(IGameEvent* event);
	void	Reset();
};