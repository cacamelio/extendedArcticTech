#pragma once

class CViewSetup;

class CWorld {
public:
	void Modulation();
	void Fog();
	void DisablePostProcessing( );
	void SkyBox();
	void ProcessCamera(CViewSetup* view_setup);
	void Smoke();
	void Crosshair();
	void RemoveBlood();
	void SunDirection(bool disable = false);
};

extern CWorld* World;