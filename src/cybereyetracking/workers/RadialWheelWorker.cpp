#include <stdafx.hpp>
#include <set>

#include "BaseInkWidgetController.hpp"
#include "RadialWheelWorker.hpp"

using ScriptableHandle = RED4ext::Handle<RED4ext::IScriptable>;

void CyberEyeTracking::Workers::RadialWheelWorker::Init()
{
    CyberEyeTracking::Workers::BaseInkWidgetController::Init();
    _radialWeaponsProp = _inkWidgetControllerCls->GetProperty("radialWeapons");
    _isActiveProp = _inkWidgetControllerCls->GetProperty("isActive");
    _activeIndexProp = _inkWidgetControllerCls->GetProperty("activeIndex");
    _determineActiveSlot = _inkWidgetControllerCls->GetFunction("DetermineActiveSlot");
    _setActiveSlot = _inkWidgetControllerCls->GetFunction("SetActiveSlot");
    _updatePointer = _inkWidgetControllerCls->GetFunction("UpdatePointer");
}

bool CyberEyeTracking::Workers::RadialWheelWorker::SetAngle(float angle)
{
    for (auto& so : GetScriptObjects())
    {
        bool isAcitve = _isActiveProp->GetValue<bool>(so);

        if (!isAcitve)
            return false;

        std::vector<RED4ext::CStackType> args;

        args.emplace_back(nullptr, &angle);
        
        RED4ext::Handle<RED4ext::IScriptable> wepH;
        RED4ext::ExecuteFunction(so, _determineActiveSlot, &wepH, args);

        if (wepH)
        {
            args.clear();
            args.emplace_back(nullptr, &wepH);
            
            bool res = false;
            RED4ext::ExecuteFunction(so, _setActiveSlot, &res, args);

            args.clear();
            args.emplace_back(nullptr);
            args.emplace_back(nullptr, &angle);

            RED4ext::ExecuteFunction(so, _updatePointer, nullptr, args);
            return true;
        }
    }

    return false;
}
