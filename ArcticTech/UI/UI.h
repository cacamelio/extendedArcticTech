#pragma once

#include <Windows.h>

class CMenu2 {
private:
	bool m_bMenuOpened;
public:
	bool Setup();
	void Render();
	void WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

extern CMenu2* Menu2;