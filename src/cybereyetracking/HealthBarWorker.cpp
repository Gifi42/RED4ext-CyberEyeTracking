#include <stdafx.hpp>
#include <set>

#include "HealthBarWorker.hpp"

using ScriptableHandle = RED4ext::Handle<RED4ext::IScriptable>;

RED4ext::CClass* _healthbarWidgetGameControllerCls = nullptr;
RED4ext::CProperty* _hpBarProp = nullptr;

RED4ext::CClass* _inkWidgetCls = nullptr;
RED4ext::CProperty* _opacityProp = nullptr;

std::mutex _syncMutex;

std::set<uint64_t> _scriptObjects;
std::unordered_map<uint64_t, ScriptableHandle> s_objhs;

uint64_t (*HealthbarWidgetControllerInitOrig)(void* aThis, RED4ext::IScriptable* aScriptable) = nullptr;

uint64_t HealthbarWidgetControllerInitHook(void* aThis, RED4ext::IScriptable* aScriptable)
{
    auto ret = HealthbarWidgetControllerInitOrig(aThis, aScriptable);

    _syncMutex.lock();

    if (aThis == _healthbarWidgetGameControllerCls && _scriptObjects.find((uint64_t)aScriptable) == _scriptObjects.end())
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

uint64_t (*HealthbarWidgetControllerDestroyOrig)(void* aThis, RED4ext::IScriptable* aMemory) = nullptr;

uint64_t HealthbarWidgetControllerDestroyHook(void* aThis, RED4ext::IScriptable* aMemory)
{
    _syncMutex.lock();
    if (aThis != _healthbarWidgetGameControllerCls)
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
    return HealthbarWidgetControllerDestroyOrig(aThis, aMemory);
}

void Hide(uint64_t aHealthbarWidgetGameController)
{
    if (!aHealthbarWidgetGameController)
        return;

    uint64_t holder = aHealthbarWidgetGameController;
    if (_hpBarProp->flags.b21)
    {
        holder = (uint64_t)((RED4ext::IScriptable*)aHealthbarWidgetGameController)->GetValueHolder();
    }

    uintptr_t addr = (uintptr_t)holder + _hpBarProp->valueOffset;

    auto pWH = reinterpret_cast<RED4ext::WeakHandle<RED4ext::IScriptable>*>(addr);

    if (pWH)
    {
        auto inkWidget = pWH->Lock();
        if (inkWidget)
        {
            static auto base = std::chrono::high_resolution_clock::now();
            auto now = std::chrono::high_resolution_clock::now();
            float opacity = _opacityProp->GetValue<float>(inkWidget.GetPtr());
            float time = std::chrono::duration<float>(now - base).count();
            float varying_0_1 = 0.5f + 0.5f * std::sinf(time);
            float new_opacity = 0.5f + 0.5f * varying_0_1;

            // Does work but the widget doesn't update its render...
            // s_opacityProp->SetValue<float>(inkWidget.GetPtr(), new_opacity);

            auto ctrler = (RED4ext::IScriptable*)aHealthbarWidgetGameController;
            
            inkWidget->ExecuteFunction("SetOpacity", 0);

            // Looks that this one does not work :/
            // ctrler->ExecuteMethod("UpdateRequired");
        }
        else
        {
            spdlog::debug("couldn't lock HPBar whandle");
        }
    }
}

void CyberEyeTracker::Workers::HealthBarWorker::Init()
{
    auto rtti = RED4ext::CRTTISystem::Get();

    _healthbarWidgetGameControllerCls = rtti->GetClass("healthbarWidgetGameController");
    
    spdlog::debug("healthbarWidgetGameController: {}", _healthbarWidgetGameControllerCls->name.ToString());
    spdlog::debug("address: {:016X}", (uint64_t)_healthbarWidgetGameControllerCls);

    _hpBarProp = _healthbarWidgetGameControllerCls->GetProperty("HPBar");
    spdlog::debug("HPBar                        : {}", _hpBarProp->name.ToString());
    spdlog::debug("address: {:016X}", (uint64_t)_hpBarProp);

    _inkWidgetCls = rtti->GetClass("inkWidget");
    spdlog::debug("inkWidget                    : {}", _inkWidgetCls->name.ToString());
    spdlog::debug("address: {:016X}", (uint64_t)_inkWidgetCls);

    _opacityProp = _inkWidgetCls->GetProperty("opacity");
    spdlog::debug("opacity                      : {}", _opacityProp->name.ToString());
    spdlog::debug("address: {:016X}", (uint64_t)_opacityProp);

    uint64_t* pvtbl = *(uint64_t**)_healthbarWidgetGameControllerCls;

    const uint64_t baseaddr = (uint64_t)GetModuleHandle(nullptr);
    spdlog::debug("baseaddr      : {:016X}", baseaddr);
    spdlog::debug("vtbladdr (rel): {:016X}", (uint64_t)pvtbl - baseaddr);

    HealthbarWidgetControllerInitOrig = (decltype(HealthbarWidgetControllerInitOrig))pvtbl[27];
    HealthbarWidgetControllerDestroyOrig = (decltype(HealthbarWidgetControllerDestroyOrig))pvtbl[28];

    DWORD oldRw;
    if (VirtualProtect((void*)pvtbl, 0x100, PAGE_EXECUTE_READWRITE, &oldRw))
    {
        pvtbl[27] = (uint64_t)HealthbarWidgetControllerInitHook;
        pvtbl[28] = (uint64_t)HealthbarWidgetControllerDestroyHook;

        spdlog::debug("vtbl hooked !");

        VirtualProtect((void*)pvtbl, 0x100, oldRw, NULL);
    }
}

void CyberEyeTracker::Workers::HealthBarWorker::HideHPBar()
{
    decltype(_scriptObjects) copy = _scriptObjects;

    for (auto& so : copy)
    {
        RED4ext::IScriptable* scriptable = (RED4ext::IScriptable*)so;
        if (*(uint64_t*)scriptable == 0)
        {
            spdlog::debug("HealtBarWorker.HideHPBar: null vtbl");
            continue;
        }

        if (scriptable->ref.instance && scriptable->ref.GetUseCount())
        {
            ScriptableHandle sh(scriptable); // auto-share_from_this
            s_objhs.emplace((uint64_t)scriptable, sh);
        }

        Hide((uint64_t)scriptable);

        /*auto typ = scriptable->GetType();
        if (typ->GetType() != RED4ext::ERTTIType::Class ||
            !static_cast<RED4ext::CClass*>(typ)->IsA(_healthbarWidgetGameControllerCls))
        {
            RED4ext::CName cn;
            typ->GetName(cn);
            ("HealtBarWorker.HideHPBar: scriptable's type is {0}", cn.ToString());
        }*/
    }
}
