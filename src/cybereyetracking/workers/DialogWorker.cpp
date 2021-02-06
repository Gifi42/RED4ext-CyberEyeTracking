#include <stdafx.hpp>
#include <set>

#include "BaseInkWidgetController.hpp"
#include "DialogWorker.hpp"

using ScriptableHandle = RED4ext::Handle<RED4ext::IScriptable>;

void CyberEyeTracking::Workers::DialogWorker::Init()
{
    CyberEyeTracking::Workers::BaseInkWidgetController::Init();
    _activeHubIdProp = _inkWidgetControllerCls->GetProperty("activeHubID");
    _hubControllersProp = _inkWidgetControllerCls->GetProperty("hubControllers");

    _dialogHubLogicControllerCls = _rtti->GetClass("DialogHubLogicController");
    _dhlcIdProp = _dialogHubLogicControllerCls->GetProperty("id");
    _itemControllersProp = _dialogHubLogicControllerCls->GetProperty("itemControllers");
    _dialogChoiceLogicControllerCls = _rtti->GetClass("DialogChoiceLogicController");
}

void CyberEyeTracking::Workers::DialogWorker::SetAngle(float angle)
{
    for (auto& so : GetScriptObjects())
    {
        activeHubId = _activeHubIdProp->GetValue<int>(so);
        if (activeHubId < 0)
            return;

        auto hubControllers = _hubControllersProp->GetValue<RED4ext::DynArray<RED4ext::WeakHandle<RED4ext::IScriptable>>>(so);

         if (hubControllers.size == 0)
            return;

         RED4ext::IScriptable* activeHubController;
         for (auto& hubControllerWH : hubControllers)
         {
             auto hubControllerH = hubControllerWH.Lock();
             if (_dhlcIdProp->GetValue<int>(hubControllerH.instance) == activeHubId)
             {
                 activeHubController = hubControllerH.GetPtr();
                 break;
             }
         }

         if (activeHubController == nullptr)
             return;

         auto itemControllersArr = _itemControllersProp->GetValue<RED4ext::DynArray<RED4ext::Handle<RED4ext::IScriptable>>>(activeHubController);

         if (itemControllersArr.size > 10)
             return;


         auto widgetCls = _rtti->GetClass("inkWidget");
         auto getHeightF = widgetCls->GetFunction("GetHeight");
         for (auto& itemH : itemControllersArr)
         {
             if (!itemH || !itemH.instance)
                 continue;

             auto rootWidgetVal = itemH->ExecuteFunction<RED4ext::WeakHandle<RED4ext::IScriptable>>("GetRootWidget", nullptr);
             if (!rootWidgetVal.has_value())
                 continue;

             RED4ext::WeakHandle<RED4ext::IScriptable> rootWidgetWH = rootWidgetVal.value();

             auto rootWidgetH = rootWidgetWH.Lock();
             if (!rootWidgetH || !rootWidgetH.instance)
                 return;

             auto height = rootWidgetH->ExecuteFunction<float>("GetHeight", nullptr);
             if (height.has_value())
                spdlog::debug(height.value());
         }
    }
}
