#pragma once

struct IDirect3DTexture9;

class CWeaponIcon {
	IDirect3DTexture9* weapon_icons[];
public:
	void Setup();
}