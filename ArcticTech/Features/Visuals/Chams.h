#pragma once
#include "../../SDK/Misc/Color.h"
#include "../../SDK/Interfaces/IMaterialSystem.h"

class IMaterial;

struct ChamsMaterial {
	bool enabled = false;
	int type = 0;
	Color primaryColor;
	bool addZ = false;
	Color invisibleColor;
	Color secondaryColor;
	float glowThickness = 1;
};

class CChams {
	bool IsLocalPlayerAttachment(CBaseEntity* entity);

	void* _ctx;
	DrawModelState_t _state;
	ModelRenderInfo_t _info;
	matrix3x4_t* _boneToWorld;

	IMaterial* baseMaterials[3];
	ChamsMaterial materials[4];
public:
	void LoadChams();
	void UpdateSettings();
	void ColorModulate(IMaterial* mat, Color color);
	void OverrideMaterial(int type, bool z, Color color, float glowThickness = 1);
	bool OnDrawModelExecute(void* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* pBoneToWorld);
	void DrawModel(ChamsMaterial& material, float alpha = 1.f, matrix3x4_t* customBoneToWorld = nullptr);
};

extern CChams* Chams;