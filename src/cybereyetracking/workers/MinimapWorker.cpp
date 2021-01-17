#include <stdafx.hpp>
#include <set>

#include "MinimapWorker.hpp"

using ScriptableHandle = RED4ext::Handle<RED4ext::IScriptable>;


void CyberEyeTracking::Workers::MinimapWorker::SetOpacity(float value)
{
    decltype(GetScriptObjects()) copy = GetScriptObjects();

    for (auto& scriptable : copy)
    {
        if (*(uint64_t*)scriptable == 0)
        {
            spdlog::debug("MinimapWorker: null vtbl");
            continue;
        }

        if (scriptable->ref.instance && scriptable->ref.GetUseCount())
        {
            ScriptableHandle sh(scriptable); // auto-share_from_this
        }
        if (!scriptable)
            return;

        uint64_t holder = (uint64_t)scriptable;
        auto rootWidgetProp = _inkWidgetControllerCls->GetProperty("rootWidget");
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
                inkWidget->ExecuteFunction("SetOpacity", value);
            }
            else
            {
                spdlog::debug("couldn't lock HPBar whandle");
            }
        }
    }
}


void CyberEyeTracking::Workers::MinimapWorker::Init()
{
    InitBase();
}

void CyberEyeTracking::Workers::MinimapWorker::HideMiniMap()
{
    SetOpacity(0.07);
}

void CyberEyeTracking::Workers::MinimapWorker::ShowMiniMap()
{
    SetOpacity(1);
}
