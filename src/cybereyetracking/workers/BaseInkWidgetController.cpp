#include <stdafx.hpp>
#include <set>
#include <map>

#include "BaseInkWidgetController.hpp"

#define OPACITY_HIDE 0.07

using ScriptableHandle = RED4ext::Handle<RED4ext::IScriptable>;

std::set<RED4ext::CClass*> g_inkWidgetControllersCls = std::set<RED4ext::CClass*>();
std::set<RED4ext::IScriptable*> g_scriptObjects = std::set<RED4ext::IScriptable*>();

std::mutex _syncMutex;

std::unordered_map<uint64_t, ScriptableHandle> s_objhs;

bool g_FindScriptObject(RED4ext::IScriptable* aScriptable)
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

bool g_FindInkWidgetControllersCls(RED4ext::CClass* aThis)
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

typedef uint64_t (*HookFunction)(void*, RED4ext::IScriptable*);
typedef std::map<std::string, HookFunction> HooksMap;
std::map<std::string, uint64_t> WidgetControllersOrigInitRelAdr;

HooksMap WidgetControllersOrigInitHooks;

uint64_t WidgetControllerInitHook(void* aThis, RED4ext::IScriptable* aScriptable)
{
    auto cls = ((RED4ext::CClass*)aThis);
    
    HookFunction initOrig = nullptr;
    const char* name = cls->name.ToString();
    if (WidgetControllersOrigInitHooks.count(name))
        initOrig = WidgetControllersOrigInitHooks[name];

    if (initOrig == nullptr)
        initOrig = WidgetControllersOrigInitHooks["healthbarWidgetGameController"];

    auto ret = initOrig(aThis, aScriptable);
    
    _syncMutex.lock();
    if (!g_FindInkWidgetControllersCls(cls))
    {
        _syncMutex.unlock();
        return ret;
    }
   
    if (g_FindScriptObject(aScriptable))
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

HooksMap WidgetControllersOrigDestroyHooks{};

uint64_t WidgetControllerDestroyHook(void* aThis, RED4ext::IScriptable* aMemory)
{
    auto cls = ((RED4ext::CClass*)aThis);
    _syncMutex.lock();
    
    if (g_FindInkWidgetControllersCls(cls) && g_FindScriptObject(aMemory))
    {
        spdlog::debug("hooked DestroyCls aScriptable:{:016X}", (uint64_t)aMemory);
        g_scriptObjects.erase(aMemory);
    }
    
    _syncMutex.unlock();
    
    auto destroyOrig = WidgetControllersOrigDestroyHooks[cls->name.ToString()];
    
    if (destroyOrig == nullptr)
        destroyOrig = WidgetControllersOrigInitHooks["healthbarWidgetGameController"];
    return destroyOrig(aThis, aMemory);
}
uint64_t test(void* aThis, RED4ext::IScriptable* aScriptable)
{
    return 0;
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

    auto clsName = _inkWidgetControllerCls->name.ToString();
    spdlog::debug("Controller name: {}", clsName);
    spdlog::debug("address: {:016X}", (uint64_t)_inkWidgetControllerCls);

    _inkWidgetCls = _rtti->GetClass("inkWidget");
    spdlog::debug("inkWidget                    : {}", _inkWidgetCls->name.ToString());
    spdlog::debug("address: {:016X}", (uint64_t)_inkWidgetCls);
    uint64_t* pvtbl = *(uint64_t**)_inkWidgetControllerCls;

    const uint64_t baseaddr = (uint64_t)GetModuleHandle(nullptr);
    auto vtblAdr = (uint64_t)pvtbl - baseaddr;
    spdlog::debug("baseaddr      : {:016X}", baseaddr);
    spdlog::debug("vtbladdr (rel): {:016X}", vtblAdr);

    for (auto it = WidgetControllersOrigInitRelAdr.begin(); it != WidgetControllersOrigInitRelAdr.end(); it++)
    {
        spdlog::debug("orig: {:016X}", it->second);
        spdlog::debug("cur: {:016X}", vtblAdr);
        if (it->second == vtblAdr)
        {
            WidgetControllersOrigInitHooks[clsName] = WidgetControllersOrigInitHooks[it->first];
            WidgetControllersOrigDestroyHooks[clsName] = WidgetControllersOrigDestroyHooks[it->first];
            return;
        }
    }
    WidgetControllersOrigInitHooks[clsName] = (HookFunction)pvtbl[27];
    WidgetControllersOrigDestroyHooks[clsName] = (HookFunction)pvtbl[28];
    WidgetControllersOrigInitRelAdr[clsName] = vtblAdr;
    DWORD oldRw;
    if (VirtualProtect((void*)pvtbl, 0x100, PAGE_EXECUTE_READWRITE, &oldRw))
    {
        pvtbl[27] = (uint64_t)WidgetControllerInitHook;
        pvtbl[28] = (uint64_t)WidgetControllerDestroyHook;

        spdlog::debug("vtbl hooked !");

        VirtualProtect((void*)pvtbl, 0x100, oldRw, NULL);
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


void CyberEyeTracking::Workers::BaseInkWidgetController::SetRootOpacity(float value)
{
    decltype(GetScriptObjects()) copy = GetScriptObjects();

    for (auto& scriptable : copy)
    {
        if (*(uint64_t*)scriptable == 0)
        {
            spdlog::debug("{} null vtbl", scriptable->GetType()->name.ToString());
            continue;
        }

        if (scriptable->ref.instance && scriptable->ref.GetUseCount())
        {
            ScriptableHandle sh(scriptable); // auto-share_from_this
        }
        if (!scriptable)
            return;

        auto rootWidgetFunc = _inkWidgetControllerCls->GetFunction("GetRootWidget");

        auto pWH = scriptable->ExecuteFunction<RED4ext::WeakHandle<RED4ext::IScriptable>>("GetRootWidget", nullptr);

        if (pWH)
        {
            auto inkWidget = pWH->Lock();
            if (inkWidget)
            {
                inkWidget->ExecuteFunction("SetOpacity", value);
            }
            else
            {
                spdlog::debug("couldn't lock HPBar whandle");
            }
        }
    }
}


void CyberEyeTracking::Workers::BaseInkWidgetController::HideWidget()
{
    SetRootOpacity(OPACITY_HIDE);
}

void CyberEyeTracking::Workers::BaseInkWidgetController::ShowWidget()
{
    SetRootOpacity(1);
}
