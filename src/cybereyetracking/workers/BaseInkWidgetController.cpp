#include <stdafx.hpp>
#include <set>

#include "BaseInkWidgetController.h"

using ScriptableHandle = RED4ext::Handle<RED4ext::IScriptable>;

RED4ext::CName _ctrlrRTTIname;
RED4ext::CClass* _inkWidgetControllerCls = nullptr;
std::set<uint64_t> _scriptObjects = std::set<uint64_t>();

std::mutex _syncMutex;

std::unordered_map<uint64_t, ScriptableHandle> s_objhs;

uint64_t (*WidgetControllerInitOrig)(void* aThis, RED4ext::IScriptable* aScriptable) = nullptr;

uint64_t WidgetControllerInitHook(void* aThis, RED4ext::IScriptable* aScriptable)
{
    auto ret = WidgetControllerInitOrig(aThis, aScriptable);

    _syncMutex.lock();

    if (aThis == _inkWidgetControllerCls &&
        _scriptObjects.find((uint64_t)aScriptable) == _scriptObjects.end())
    {
        if (aScriptable)
        {
            // MEMORY_BASIC_INFORMATION mbi;
            // VirtualQuery(aScriptable, &mbi, sizeof(mbi));
            // spdlog::debug("aScriptable's protect flag: {:X}", mbi.Protect);

            _scriptObjects.emplace((uint64_t)aScriptable);
        }
        else
        {
            spdlog::debug("hooked InitCls but aScriptable is null");
        }
        // uint64_t addr = (uint64_t)aScriptable + 0xC0;
        // bool ok = HWBreakpoint::Set((void*)addr, HWBreakpoint::Condition::ReadWrite);
        // if (ok)
        //{
        //    spdlog::debug("HWBP set at {:016X}", addr);
        //    s_obj_addresses.emplace((uint64_t)aScriptable, (HANDLE)addr);
        //}
        // else
        //{
        //    spdlog::debug("HWBP failed at {:016X}", addr);
        //}
    }
    _syncMutex.unlock();
    return ret;
}

uint64_t (*WidgetControllerDestroyOrig)(void* aThis, RED4ext::IScriptable* aMemory) = nullptr;

uint64_t WidgetControllerDestroyHook(void* aThis, RED4ext::IScriptable* aMemory)
{
    _syncMutex.lock();
    
    if (aThis != _inkWidgetControllerCls)
    {
        // spdlog::debug("MyDestroyHook: aThis != s_healthbarWidgetGameControllerCls");
    }
    else
    {
        auto it = _scriptObjects.find((uint64_t)aMemory);
        if (it != _scriptObjects.end())
        {
            spdlog::debug("hooked DestroyCls aScriptable:{:016X}", (uint64_t)aMemory);
            _scriptObjects.erase(it);
        }
    }
    _syncMutex.unlock();
    return WidgetControllerDestroyOrig(aThis, aMemory);
}

CyberEyeTracking::Workers::BaseInkWidgetController::BaseInkWidgetController(char* ctrlrRTTIname)
{
    _ctrlrRTTIname = RED4ext::CName(ctrlrRTTIname);
}

void CyberEyeTracking::Workers::BaseInkWidgetController::InitBase()
{
    auto rtti = RED4ext::CRTTISystem::Get();
    _inkWidgetControllerCls = rtti->GetClass(_ctrlrRTTIname);

    spdlog::debug("Controller name: {}", _inkWidgetControllerCls->name.ToString());
    spdlog::debug("address: {:016X}", (uint64_t)_inkWidgetControllerCls);

    _inkWidgetCls = rtti->GetClass("inkWidget");
    spdlog::debug("inkWidget                    : {}", _inkWidgetCls->name.ToString());
    spdlog::debug("address: {:016X}", (uint64_t)_inkWidgetCls);
    uint64_t* pvtbl = *(uint64_t**)_inkWidgetControllerCls;
    
    const uint64_t baseaddr = (uint64_t)GetModuleHandle(nullptr);
    spdlog::debug("baseaddr      : {:016X}", baseaddr);
    spdlog::debug("vtbladdr (rel): {:016X}", (uint64_t)pvtbl - baseaddr);

    WidgetControllerInitOrig = (decltype(WidgetControllerInitOrig))pvtbl[27];
    WidgetControllerDestroyOrig = (decltype(WidgetControllerDestroyOrig))pvtbl[28];

    DWORD oldRw;
    if (VirtualProtect((void*)pvtbl, 0x100, PAGE_EXECUTE_READWRITE, &oldRw))
    {
        pvtbl[27] = (uint64_t)WidgetControllerInitHook;
        pvtbl[28] = (uint64_t)WidgetControllerDestroyHook;

        spdlog::debug("vtbl hooked !");

        VirtualProtect((void*)pvtbl, 0x100, oldRw, NULL);
    }
}

RED4ext::CClass* CyberEyeTracking::Workers::BaseInkWidgetController::GetInkWidgetControllerCls()
{
    return _inkWidgetControllerCls;
}

std::set<uint64_t> CyberEyeTracking::Workers::BaseInkWidgetController::GetScriptObjects()
{
    return _scriptObjects;
}
