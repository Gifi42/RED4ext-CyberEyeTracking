#include <stdafx.hpp>
#include <set>

#include "HealthBarWorker.hpp"

using ScriptableHandle = RED4ext::Handle<RED4ext::IScriptable>;


void CyberEyeTracking::Workers::HealthBarWorker::SetOpacity(float value)
{
    decltype(GetScriptObjects()) copy = GetScriptObjects();

    for (auto& scriptable : copy)
    {
        if (*(uint64_t*)scriptable == 0)
        {
            spdlog::debug("HealtBarWorker: null vtbl");
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
        auto rootWidgetProp = _inkWidgetControllerCls->GetProperty("RootWidget");
        if (rootWidgetProp->flags.b21)
        {
            holder = (uint64_t)((RED4ext::IScriptable*)scriptable)->GetValueHolder();
        }

        uintptr_t addr = (uintptr_t)holder + rootWidgetProp->valueOffset;

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
    }
}


void CyberEyeTracking::Workers::HealthBarWorker::Init()
{
    InitBase();
}

void CyberEyeTracking::Workers::HealthBarWorker::HideHPBar()
{
    SetOpacity(0.07);
}

void CyberEyeTracking::Workers::HealthBarWorker::ShowHPBar()
{
    SetOpacity(1);
}
