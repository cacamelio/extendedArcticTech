#pragma once

#include "../../Utils/Utils.h"

class CHud {
public:
	void* FindHudElement(const char* name) {
		static auto func = reinterpret_cast<void* (__thiscall*)(void*, const char*)>(Utils::PatternScan("client.dll", "55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28"));
		return func(this, name);
	}

	static CHud* Get() {
		return *reinterpret_cast<CHud**>(Utils::PatternScan("client.dll", "B9 ? ? ? ? E8 ? ? ? ? 85 C0 0F 84 ? ? ? ? 83 C0 ? 0F 84 ? ? ? ? 80 B8", 0x1));
	}
};