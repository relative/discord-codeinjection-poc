#include <Windows.h>
#include <thread>
#include "dependencies/minhook/MinHook.h"

int go();

void quit(HINSTANCE module) {
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();
    FreeLibraryAndExitThread(module, 0);
}

DWORD WINAPI MainThread(LPVOID hinstDLL) {
    const auto retval = go();
    if (retval > 0) {
        char buf[128] = "";
        sprintf_s(buf, 128, "Failed to hook. go() returned %i", retval);
        MessageBoxA(nullptr, buf, "Hook failure.", MB_ICONEXCLAMATION | MB_OK);
        quit(static_cast<HINSTANCE>(hinstDLL));
        return 0;
    }
    while (!GetAsyncKeyState(VK_DELETE))
        Sleep(50);

    MessageBoxA(nullptr, "Unhooking", "MainThread", MB_OK);

    quit(static_cast<HINSTANCE>(hinstDLL));
}

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,
    DWORD fdwReason,
    LPVOID lpReserved) {
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(MainThread), hinstDLL, 0, nullptr);
            break;
        default:
            break;
    }
    return TRUE;
}
