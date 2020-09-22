#include <Windows.h>
#include <node_api.h>
#include <stdexcept>
#include <cstdio>
#include "dependencies/minhook/MinHook.h"
typedef napi_status(__cdecl* ngnv)(napi_env, const napi_node_version**);
typedef napi_status(__cdecl* nrs)(napi_env, napi_value, napi_value* res);
typedef napi_status(__cdecl* ncsu8)(napi_env, const char*, size_t, napi_value*);
typedef napi_status(__cdecl* ngvi32)(napi_env, napi_value, int32_t*);
typedef napi_status(__cdecl* ngg)(napi_env, napi_value*);

auto ncs = reinterpret_cast<ncsu8>(GetProcAddress(nullptr, "napi_create_string_utf8"));
auto rs = reinterpret_cast<nrs>(GetProcAddress(nullptr, "napi_run_script"));
auto ngv = reinterpret_cast<ngvi32>(GetProcAddress(nullptr, "napi_get_value_int32"));
auto ngnov = reinterpret_cast<ngnv>(GetProcAddress(nullptr, "napi_get_node_version"));

void messagebox_ptr(void* ptr, const char* caption) {
    char buf[512] = "";
    sprintf_s(buf, 512, "%p", ptr);
    MessageBoxA(nullptr, buf, caption, MB_OK);
}

void env_callback(void* env_) {
    static bool ran = [](napi_env env) {
        messagebox_ptr(env, "node::Environment found");

        // get node version
        const napi_node_version* version; 
        ngnov(env, &version); // call napi_get_node_version

        char buf[128] = "";
        sprintf_s(buf, 128, "%i.%i.%i (%s)", version->major, version->minor, version->patch, version->release);
        MessageBoxA(nullptr, buf, "Node version", MB_OK);

        auto scr = std::string("return 1;");

        int32_t ret;
        napi_value script, result;
        napi_status status = ncs(env, scr.c_str(), NAPI_AUTO_LENGTH, &script);
        rs(env, script, &result); // runs script. this is not in a module scope.

        status = ngv(env, result, &ret); // translates JSValue into a c value (i32)

        sprintf_s(buf, 128, "%i", ret);
        MessageBoxA(nullptr, buf, "return from script", MB_OK);
        return true;
    }(static_cast<napi_env>(env_));
}


typedef void* (__thiscall* ics)(void*, void*, void*, void* const&, int);
ics ics_orig = nullptr;
void* __fastcall InternalCallbackScope_constructor(void* pThis, void* edx, void** env, void* object, void*& asyncContext, int flags) {
    env_callback(env); // captures an env
    return ics_orig(pThis, env, object, asyncContext, flags);
}


int go() {
    if (MH_Initialize() != MH_OK)
        return 1;

    void* target = GetProcAddress(nullptr, "??0InternalCallbackScope@node@@QAE@PAVEnvironment@1@V?$Local@VObject@v8@@@v8@@ABUasync_context@1@W4ResourceExpectation@01@@Z");
    if (target == nullptr) {
        MessageBoxA(nullptr, "Failed to find InternalCallbackScope constructor, check your targets PE export table and replace the export.", "Hook failure", MB_ICONEXCLAMATION | MB_OK);
        return 2;
    }
    if (MH_CreateHook(target, &InternalCallbackScope_constructor, reinterpret_cast<void**>(&ics_orig)) != MH_OK)
        return 3;
    if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
        return 4;
    return 0;
}