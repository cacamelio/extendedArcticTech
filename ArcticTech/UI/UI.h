#pragma once

#include <Windows.h>

class CMenu {
private:
	bool m_bMenuOpened = true;
	bool m_bIsInitialized = false;
public:
	bool IsOpened() { return m_bMenuOpened; };
	bool IsInitialized() { return m_bIsInitialized; };

	void Setup();
	void Release();
	void Render();
	void WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

extern CMenu* Menu;