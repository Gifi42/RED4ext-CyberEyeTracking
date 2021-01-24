#include <stdafx.hpp>
#include <set>
#include <map>

#include "BaseInkWidgetController.hpp"

#define OPACITY_HIDE 0.07

using ScriptableHandle = RED4ext::Handle<RED4ext::IScriptable>;

std::set<RED4ext::CClass*> g_inkWidgetControllersCls = std::set<RED4ext::CClass*>();
std::set<RED4ext::IScriptable*> g_scriptObjects = std::set<RED4ext::IScriptable*>();
std::map<std::string, bool> g_singletonClasses;
std::map<std::string, RED4ext::IScriptable*> g_signletoneScriptObjects;

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
    {
        initOrig = WidgetControllersOrigInitHooks[name];
        spdlog::debug("init {}", name);
    }

    if (initOrig == nullptr)
        initOrig = WidgetControllersOrigInitHooks["healthbarWidgetGameController"];

    auto ret = initOrig(aThis, aScriptable);
    
    _syncMutex.lock();
    if (!g_FindInkWidgetControllersCls(cls) || g_FindScriptObject(aScriptable))
    {
        _syncMutex.unlock();
        return ret;
    }

    if (g_singletonClasses[name])
    {
        g_signletoneScriptObjects[name] = aScriptable;
    }
    else if (aScriptable)
    {
        g_scriptObjects.emplace(aScriptable);
    }
    else
    {
        spdlog::debug("hooked InitCls but aScriptable is null");
    }

    _syncMutex.unlock();
    return ret;
}

HooksMap WidgetControllersOrigDestroyHooks{};

uint64_t WidgetControllerDestroyHook(void* aThis, RED4ext::IScriptable* aMemory)
{
    auto cls = ((RED4ext::CClass*)aThis);
    const char* name = cls->name.ToString();

    _syncMutex.lock();

    if (g_singletonClasses[name])
    {
        g_signletoneScriptObjects[name] = nullptr;
    }
    else if (g_FindScriptObject(aMemory))
    {
        spdlog::debug("destroy {}", cls->name.ToString());
        g_scriptObjects.erase(aMemory);
    }
    
    _syncMutex.unlock();

    HookFunction destroyOrig = nullptr;

    if (WidgetControllersOrigDestroyHooks.count(name))
        destroyOrig = WidgetControllersOrigDestroyHooks[name];
    
    if (destroyOrig == nullptr)
        destroyOrig = WidgetControllersOrigDestroyHooks["healthbarWidgetGameController"];
    return destroyOrig(aThis, aMemory);
}

bool CyberEyeTracking::Workers::BaseInkWidgetController::vtblHooked = false;

CyberEyeTracking::Workers::BaseInkWidgetController::BaseInkWidgetController(const char* ctrlrRTTIname, bool singleton)
{
    _ctrlrRTTIname = RED4ext::CName(ctrlrRTTIname);
    _singleton = singleton;
}

void CyberEyeTracking::Workers::BaseInkWidgetController::Init()
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
    spdlog::debug("===============");
    spdlog::debug("name: {}", clsName);
    spdlog::debug("address: {:016X}", (uint64_t)_inkWidgetControllerCls);

    uint64_t* pvtbl = *(uint64_t**)_inkWidgetControllerCls;

    const uint64_t baseaddr = (uint64_t)GetModuleHandle(nullptr);
    auto vtblAdr = (uint64_t)pvtbl - baseaddr;
    spdlog::debug("baseaddr      : {:016X}", baseaddr);
    spdlog::debug("vtbladdr (rel): {:016X}", vtblAdr);

    g_singletonClasses[clsName] = _singleton;
    
    for (auto it = WidgetControllersOrigInitRelAdr.begin(); it != WidgetControllersOrigInitRelAdr.end(); it++)
    {
        if (it->second == vtblAdr)
        {
            WidgetControllersOrigInitHooks[clsName] = WidgetControllersOrigInitHooks[it->first];
            WidgetControllersOrigDestroyHooks[clsName] = WidgetControllersOrigDestroyHooks[it->first];
            spdlog::debug("already hooked for {}", it->first);
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

bool CyberEyeTracking::Workers::BaseInkWidgetController::CheckScriptObject(RED4ext::IScriptable* scriptObject)
{
    return scriptObject->unk18 >= 0 && scriptObject->valueHolder != nullptr && scriptObject->ref.instance != nullptr &&
           scriptObject->ref.GetUseCount() > 0 &&
           (uint64_t)scriptObject->ref.instance != 0x00000000ffffffff && scriptObject->classType != nullptr &&
           _inkWidgetControllerCls->name == scriptObject->classType->name;
}

std::set<RED4ext::IScriptable*> CyberEyeTracking::Workers::BaseInkWidgetController::GetScriptObjects()
{
    std::set<RED4ext::IScriptable*> res;
    auto name = _ctrlrRTTIname.ToString();
    if (g_singletonClasses[name])
    {
        if (g_signletoneScriptObjects.count(name) && CheckScriptObject(g_signletoneScriptObjects[name]))
            res.emplace(g_signletoneScriptObjects[name]);

        return res;
    }
    for (auto& so : g_scriptObjects)
    {
        if (CheckScriptObject(so))
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
                spdlog::debug("couldn't lock root inkWidget whandle");
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

bool CyberEyeTracking::Workers::BaseInkWidgetController::Exist()
{
    return GetScriptObjects().size() > 0;
}

void CyberEyeTracking::Workers::BaseInkWidgetController::Erase()
{
    auto str = _ctrlrRTTIname.ToString();
    if (g_signletoneScriptObjects.count(str))
    {
        g_signletoneScriptObjects.erase(str);
    }
}


int CyberEyeTracking::Workers::BaseInkWidgetController::ObjectsCount()
{
    return GetScriptObjects().size();
}
