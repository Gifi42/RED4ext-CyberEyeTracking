#include <stdafx.hpp>
#include <set>

#include "HealthBarWorker.hpp"

using ScriptableHandle = RED4ext::Handle<RED4ext::IScriptable>;


RED4ext::CProperty* _hpBarProp = nullptr;


RED4ext::CProperty* _opacityProp = nullptr;


void SetOpacity(std::set<uint64_t> scriptObjects, float value)
{
    decltype(scriptObjects) copy = scriptObjects;

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
            //s_objhs.emplace((uint64_t)scriptable, sh);
        }
        if (!scriptable)
            return;

        uint64_t holder = (uint64_t)scriptable;
        if (_hpBarProp->flags.b21)
        {
            holder = (uint64_t)((RED4ext::IScriptable*)scriptable)->GetValueHolder();
        }

        uintptr_t addr = (uintptr_t)holder + _hpBarProp->valueOffset;

        auto pWH = reinterpret_cast<RED4ext::WeakHandle<RED4ext::IScriptable>*>(addr);

        if (pWH)
        {
            auto inkWidget = pWH->Lock();
            if (inkWidget)
            {
                /*static auto base = std::chrono::high_resolution_clock::now();
                auto now = std::chrono::high_resolution_clock::now();
                float opacity = _opacityProp->GetValue<float>(inkWidget.GetPtr());

                float time = std::chrono::duration<float>(now - base).count();
                float varying_0_1 = 0.5f + 0.5f * std::sinf(time);
                float new_opacity = 0.5f + 0.5f * varying_0_1;*/

                // Does work but the widget doesn't update its render...
                // s_opacityProp->SetValue<float>(inkWidget.GetPtr(), new_opacity);

                inkWidget->ExecuteFunction("SetOpacity", value);

                // Looks that this one does not work :/
                // ctrler->ExecuteMethod("UpdateRequired");
            }
            else
            {
                spdlog::debug("couldn't lock HPBar whandle");
            }
        }
        
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
CyberEyeTracking::Workers::HealthBarWorker::HealthBarWorker()
    : CyberEyeTracking::Workers::BaseInkWidgetController::BaseInkWidgetController("healthbarWidgetGameController")
{

}

void CyberEyeTracking::Workers::HealthBarWorker::Init()
{
    InitBase();
    auto rtti = RED4ext::CRTTISystem::Get();
    auto cls = GetInkWidgetControllerCls();
    _hpBarProp = cls->GetProperty("HPBar");
    spdlog::debug("HPBar                        : {}", _hpBarProp->name.ToString());
    spdlog::debug("address: {:016X}", (uint64_t)_hpBarProp);

    _opacityProp = _inkWidgetCls->GetProperty("opacity");
    spdlog::debug("opacity                      : {}", _opacityProp->name.ToString());
    spdlog::debug("address: {:016X}", (uint64_t)_opacityProp);

}

void CyberEyeTracking::Workers::HealthBarWorker::HideHPBar()
{
    SetOpacity(GetScriptObjects(), 0.08);
}

void CyberEyeTracking::Workers::HealthBarWorker::ShowHPBar()
{
    SetOpacity(GetScriptObjects(), 1);
}
