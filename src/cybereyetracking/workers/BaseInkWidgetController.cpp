#include <stdafx.hpp>
#include <set>

#include "BaseInkWidgetController.hpp"

using ScriptableHandle = RED4ext::Handle<RED4ext::IScriptable>;

std::set<RED4ext::CClass*> g_inkWidgetControllersCls = std::set<RED4ext::CClass*>();
std::set<RED4ext::IScriptable*> g_scriptObjects = std::set<RED4ext::IScriptable*>();

std::mutex _syncMutex;

std::unordered_map<uint64_t, ScriptableHandle> s_objhs;

bool _g_FindScriptObject(RED4ext::IScriptable* aScriptable)
{
    for (auto& x : g_scriptObjects)
    {
        if (x == aScriptable)
        {
            return true;
        }
    }
    return false;
}

bool _g_FindInkWidgetControllersCls(RED4ext::CClass* aThis)
{
    for (auto& x : g_inkWidgetControllersCls)
    {
        if (aThis == x)
        {
            return true;
        }
    }
    return false;
}

uint64_t (*WidgetControllerInitOrig)(void* aThis, RED4ext::IScriptable* aScriptable) = nullptr;

uint64_t WidgetControllerInitHook(void* aThis, RED4ext::IScriptable* aScriptable)
{
    auto ret = WidgetControllerInitOrig(aThis, aScriptable);

    _syncMutex.lock();
    
    if (!_g_FindInkWidgetControllersCls((RED4ext::CClass*)aThis))
    {
        _syncMutex.unlock();
        return ret;
    }
   
    if (_g_FindScriptObject(aScriptable))
    {
        _syncMutex.unlock();
        return ret;
    }

    if (aScriptable)
    {
        // MEMORY_BASIC_INFORMATION mbi;
        // VirtualQuery(aScriptable, &mbi, sizeof(mbi));
        // spdlog::debug("aScriptable's protect flag: {:X}", mbi.Protect);

        g_scriptObjects.emplace(aScriptable);
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
    
    _syncMutex.unlock();
    return ret;
}

uint64_t (*WidgetControllerDestroyOrig)(void* aThis, RED4ext::IScriptable* aMemory) = nullptr;

uint64_t WidgetControllerDestroyHook(void* aThis, RED4ext::IScriptable* aMemory)
{
    _syncMutex.lock();
    
    if (_g_FindInkWidgetControllersCls((RED4ext::CClass*)aThis) && _g_FindScriptObject(aMemory))
    {
        spdlog::debug("hooked DestroyCls aScriptable:{:016X}", (uint64_t)aMemory);
        g_scriptObjects.erase(aMemory);
    }
    
    _syncMutex.unlock();
    return WidgetControllerDestroyOrig(aThis, aMemory);
}

bool CyberEyeTracking::Workers::BaseInkWidgetController::vtblHooked = false;

CyberEyeTracking::Workers::BaseInkWidgetController::BaseInkWidgetController(char* ctrlrRTTIname)
{
    _ctrlrRTTIname = RED4ext::CName(ctrlrRTTIname);
}

void CyberEyeTracking::Workers::BaseInkWidgetController::InitBase()
{
    _rtti = RED4ext::CRTTISystem::Get();
    _inkWidgetControllerCls = _rtti->GetClass(_ctrlrRTTIname);
    bool exists = false;
    for (auto& x : g_inkWidgetControllersCls)
    {
        if (_inkWidgetControllerCls == x)
        {
            exists = true;
            break;
        }
    }
    if (!exists)
        g_inkWidgetControllersCls.emplace(_inkWidgetControllerCls);

    spdlog::debug("Controller name: {}", _inkWidgetControllerCls->name.ToString());
    spdlog::debug("address: {:016X}", (uint64_t)_inkWidgetControllerCls);

    _inkWidgetCls = _rtti->GetClass("inkWidget");
    spdlog::debug("inkWidget                    : {}", _inkWidgetCls->name.ToString());
    spdlog::debug("address: {:016X}", (uint64_t)_inkWidgetCls);
    uint64_t* pvtbl = *(uint64_t**)_inkWidgetControllerCls;
    
    const uint64_t baseaddr = (uint64_t)GetModuleHandle(nullptr);
    spdlog::debug("baseaddr      : {:016X}", baseaddr);
    spdlog::debug("vtbladdr (rel): {:016X}", (uint64_t)pvtbl - baseaddr);

    if (vtblHooked)
        return;

    WidgetControllerInitOrig = (decltype(WidgetControllerInitOrig))pvtbl[27];
    WidgetControllerDestroyOrig = (decltype(WidgetControllerDestroyOrig))pvtbl[28];

    DWORD oldRw;
    if (VirtualProtect((void*)pvtbl, 0x100, PAGE_EXECUTE_READWRITE, &oldRw))
    {
        pvtbl[27] = (uint64_t)WidgetControllerInitHook;
        pvtbl[28] = (uint64_t)WidgetControllerDestroyHook;

        spdlog::debug("vtbl hooked !");

        VirtualProtect((void*)pvtbl, 0x100, oldRw, NULL);
        vtblHooked = true;
    }
}


std::set<RED4ext::IScriptable*> CyberEyeTracking::Workers::BaseInkWidgetController::GetScriptObjects()
{
    std::set<RED4ext::IScriptable*> res;
    for (auto& so : g_scriptObjects)
    {
        if (_inkWidgetControllerCls->name == so->classType->name)
        {
            res.emplace(so);
        }
    }
    return res;
}
