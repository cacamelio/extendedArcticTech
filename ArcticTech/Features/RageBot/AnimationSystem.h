#pragma once
#include <array>

#include "../../SDK/Globals.h"
#include "../../SDK/Interfaces.h"

struct LagRecord;

class CAnimationSystem {
	struct stored_local_anims_t {
		// animstate vars
		float flEyeYaw = 0.f;
		float flEyePitch = 0.f;
		float flGoalFeetYaw = 0.f;
		float flMoveYaw = 0.f;
		float flLeanAmount = 0.f;
		float flJumpFallVelocity = 0.f;
		bool bOnGround = true;
		float flDurationInAir = 0.f;
		float flAffectedFraction = 0.f;

		std::array<float, 24> poseparams{};

		bool filled = false;
	} stored_local_anims;

	struct interpolate_data_t {
		Vector origin;
		matrix3x4_t original_matrix[MAXSTUDIOBONES];
		bool valid = false;
	};

	AnimationLayer local_animlayers[13];
	interpolate_data_t interpolate_data[64];

	matrix3x4_t sent_matrix[128];
	QAngle local_abs_angles;
	Vector sent_abs_origin;
public:
	void	FrameStageNotify(EClientFrameStage stage);
	void	OnCreateMove();
	void	UpdateLocalAnimations();

	void	StoreLocalAnims();
	void	RestoreLocalAnims();
	matrix3x4_t* GetLocalBoneMatrix() { return sent_matrix; };
	void	CorrectLocalMatrix(matrix3x4_t* mat, int size);

	void	BuildMatrix(CBasePlayer* player, matrix3x4_t* boneToWorld, int maxBones, int mask, AnimationLayer* animlayers);
	void	DisableInterpolationFlags(CBasePlayer* player);
	void	UpdateAnimations(CBasePlayer* player, LagRecord* record, std::deque<LagRecord>& records);

	Vector	GetInterpolated(CBasePlayer* player);
	void	RunInterpolation();
	void	InterpolateModel(CBasePlayer* player, matrix3x4_t* matrix);
	void	ResetInterpolation();
	void	InvalidateInterpolation(int i);
};

extern CAnimationSystem* AnimationSystem;
