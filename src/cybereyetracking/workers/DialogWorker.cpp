#include <stdafx.hpp>
#include <set>

#include "BaseInkWidgetController.hpp"
#include "DialogWorker.hpp"

#define SCALE_MUL 1.2
#define PIXEL_OFFSET 19

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
    _setSelectedF = _dialogChoiceLogicControllerCls->GetFunction("SetSelected");
    _updateViewF = _dialogChoiceLogicControllerCls->GetFunction("UpdateView");
    _updateColorsF = _dialogChoiceLogicControllerCls->GetFunction("UpdateColors");
    _animateSelectionF = _dialogChoiceLogicControllerCls->GetFunction("AnimateSelection");
}

struct Vector2
{
    float X;
    float Y;
};

struct Margin
{
    float bottom;
    float left;
    float right;
    float top;
};

bool CyberEyeTracking::Workers::DialogWorker::SelectAtPos(float yPos)
{
    for (auto& so : GetScriptObjects())
    {
        activeHubId = _activeHubIdProp->GetValue<int>(so);
        if (activeHubId < 0)
        {
            selected = 0;
            return false;
        }

        auto hubControllers = _hubControllersProp->GetValue<RED4ext::DynArray<RED4ext::WeakHandle<RED4ext::IScriptable>>>(so);

        if (hubControllers.size == 0)
        {
            selected = 0;
            return false;
        }

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
            return false;

        auto itemControllersArr = _itemControllersProp->GetValue<RED4ext::DynArray<RED4ext::Handle<RED4ext::IScriptable>>>(activeHubController);

        if (itemControllersArr.size < 2 || itemControllersArr.size > 6)
            return false;

        // x1: 2 = 777, 3 = 759, 4 = 742. offset = 18. height = 31. next = +34
        // x1.2: 2 = 758 , 3 = 734, 4 = 710. offset = 24
        float startPixel = 777;
        if (SCALE_MUL != 1)
            startPixel -= (18 * SCALE_MUL);

        float startOffset = (itemControllersArr.size - 2) * 18 * SCALE_MUL;
        startPixel -= startOffset;
        float startPos = startPixel / 1080;
        if (yPos < startPos)
            yPos = startPos;
        //        args.emplace_back(true);
        
        auto selectedIndexProp = _inkWidgetControllerCls->GetProperty("selectedIndex");
        int sc1 = 34 * SCALE_MUL;
        int height = ((sc1)-34) + sc1; // x1 = 34. x1.2 = 46
        for (int i = 0; i < itemControllersArr.size; ++i)
        {
            float curChoiceStart = startPixel + (i * height); // +8
            float curChoiceEnd = (curChoiceStart + height) / 1080;
            curChoiceStart /= 1080;

            /*if (i == itemControllersArr.size - 1 && selected == -1)
                value = true;*/

            if (yPos >= curChoiceStart && yPos <= curChoiceEnd)
            {
                selected = i;
            }
        }

        Vector2 scale{};
        scale.X = SCALE_MUL;
        scale.Y = SCALE_MUL;
        auto rootWidgetProp = _dialogHubLogicControllerCls->GetProperty("rootWidget");
        auto setScaleF = _inkWidgetCls->GetFunction("SetScale");
        auto updateMargin = _inkWidgetCls->GetFunction("SetMargin");
        std::vector<RED4ext::CStackType> args;
        args.emplace_back(nullptr, &scale);

        auto rootWidget = rootWidgetProp->GetValue<RED4ext::WeakHandle<RED4ext::IScriptable>>(activeHubController);
        RED4ext::ExecuteFunction(rootWidget.Lock(), setScaleF, nullptr, args);

        rootWidgetProp = _dialogChoiceLogicControllerCls->GetProperty("RootWidget");

        int i = 0;
        Margin margin{};
        
        for (auto& itemH : itemControllersArr)
        {
            rootWidget = rootWidgetProp->GetValue<RED4ext::WeakHandle<RED4ext::IScriptable>>(itemH);
            bool value = selected == i; 
            args.clear();
            args.emplace_back(nullptr, &scale);
            RED4ext::ExecuteFunction(rootWidget.Lock(), setScaleF, nullptr, args);

            args.clear();
            margin.top = margin.bottom = 12;
            args.emplace_back(nullptr, &margin);
            RED4ext::ExecuteFunction(rootWidget.Lock(), updateMargin, nullptr, args);

            args.clear();
            args.emplace_back(nullptr, &value);
            RED4ext::ExecuteFunction(itemH, _setSelectedF, nullptr, args);
            RED4ext::ExecuteFunction(itemH, _updateColorsF, nullptr, {});
            RED4ext::ExecuteFunction(itemH, _updateViewF, nullptr, {});
            RED4ext::ExecuteFunction(itemH, _animateSelectionF, nullptr, {});
            ++i;
        }

        selectedIndexProp->SetValue<int>(so, selected);
        spdlog::debug("{}", selectedIndexProp->GetValue<int>(so));
        return true;
    }
    return false;
}
