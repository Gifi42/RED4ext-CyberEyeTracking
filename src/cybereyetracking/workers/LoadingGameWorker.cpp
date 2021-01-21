#include <stdafx.hpp>
#include <set>

#include "BaseInkWidgetController.hpp"
#include "LoadingGameWorker.hpp"

using ScriptableHandle = RED4ext::Handle<RED4ext::IScriptable>;

void CyberEyeTracking::Workers::LoadingGameWorker::Init()
{
    InitBase();
}

bool CyberEyeTracking::Workers::LoadingGameWorker::Loading()
{
    for (auto& so : GetScriptObjects())
    {
        auto widget = so->ExecuteFunction<RED4ext::WeakHandle<RED4ext::IScriptable>>("GetRootWidget", nullptr);
        if (!widget)
            return false;

        auto i = widget->Lock();
        auto val = i->ExecuteFunction<bool>("IsVisible", nullptr);
        return val.has_value() && val.value();
    }
    return false;
}
