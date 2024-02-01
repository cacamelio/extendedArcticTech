#include <Windows.h>
#include <locale.h>

#include "SDK/Interfaces.h"
#include "SDK/Hooks.h"
#include "Features/Visuals/SkinChanger.h"
#include "Features/Lua/Bridge/Bridge.h"


void Initialize(HMODULE hModule) {
    setlocale(LC_ALL, "ru_RI.UTF-8");

    Interfaces::Initialize();
    Hooks::Initialize();
    Lua->Setup();
    SkinChanger->FixViewModelSequence();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Initialize, hModule, 0, 0);
    return TRUE;
}