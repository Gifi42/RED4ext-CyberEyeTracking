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
    _selectedIdxProp = _inkWidgetControllerCls->GetProperty("selectedIndex");
    _activeHubIdProp = _inkWidgetControllerCls->GetProperty("activeHubID");
    _hubControllersProp = _inkWidgetControllerCls->GetProperty("hubControllers");
    /*_onDialogsSelectF = _inkWidgetControllerCls->GetFunction("OnDialogsSelectIndex");

    _blackboardCls = _rtti->GetClass("gameIBlackboard");
    _blackboardProp = _inkWidgetControllerCls->GetProperty("InteractionsBlackboard");
    _interactionsBBDefinitionProp = _inkWidgetControllerCls->GetProperty("InteractionsBBDefinition");
    _UIInteractionsDefCls = _rtti->GetClass("UIInteractionsDef");
    _UIInteractionsSelectedIdxProp = _UIInteractionsDefCls->GetProperty("SelectedIndex");*/

    _dialogHubLogicControllerCls = _rtti->GetClass("DialogHubLogicController");
    _dhlcIdProp = _dialogHubLogicControllerCls->GetProperty("id");
    _itemControllersProp = _dialogHubLogicControllerCls->GetProperty("itemControllers");
    _hubRootWidgetProp = _dialogHubLogicControllerCls->GetProperty("rootWidget");

    _dialogChoiceLogicControllerCls = _rtti->GetClass("DialogChoiceLogicController");
    _updateViewF = _dialogChoiceLogicControllerCls->GetFunction("UpdateView");
    //_setSelectedF = _dialogChoiceLogicControllerCls->GetFunction("SetSelected");
    //selectionF = _dialogChoiceLogicControllerCls->GetFunction("AnimateSelection");
    _choiceRootWidgetProp =_dialogChoiceLogicControllerCls->GetProperty("RootWidget");

    _inkWidgetSetScaleF = _inkWidgetCls->GetFunction("SetScale");
    _inkWidgetUpdateMarginF = _inkWidgetCls->GetFunction("SetMargin");
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

bool CyberEyeTracking::Workers::DialogWorker::Reset()
{
    selected = 0;
    widgetsScaled = false;
    return false;
}

bool CyberEyeTracking::Workers::DialogWorker::SelectAtPos(float yPos)
{
    for (auto& so : GetScriptObjects())
    {
        activeHubId = _activeHubIdProp->GetValue<int>(so);
        if (activeHubId < 0)
            return Reset();

        auto hubControllers = _hubControllersProp->GetValue<RED4ext::DynArray<RED4ext::WeakHandle<RED4ext::IScriptable>>>(so);

        if (hubControllers.size == 0)
            return Reset();

        RED4ext::IScriptable* activeHubController = nullptr;
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
            return Reset();

        auto itemControllersArr = _itemControllersProp->GetValue<RED4ext::DynArray<RED4ext::Handle<RED4ext::IScriptable>>>(activeHubController);

        if (itemControllersArr.size < 2 || itemControllersArr.size > 6)
            return Reset();

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
        
        int sc1 = 34 * SCALE_MUL;
        int height = ((sc1)-34) + sc1; // x1 = 34. x1.2 = 46

        int selectionOffset = 0;
        int i = 0;

        selected = _selectedIdxProp->GetValue<int32_t>(so);
        for (i = 0; i < itemControllersArr.size; ++i)
        {
            float curChoiceStart = startPixel + (i * height); // +8
            float curChoiceEnd = (curChoiceStart + height) / 1080;
            curChoiceStart /= 1080;

            if (yPos >= curChoiceStart && yPos <= curChoiceEnd)
            {
                if (selected == i)
                    return true;

                selectionOffset = i - selected;
                selected = i;
                break;
            }
        }

        if (!widgetsScaled)
        {
            Vector2 scale{};
            scale.X = SCALE_MUL;
            scale.Y = SCALE_MUL;

            std::vector<RED4ext::CStackType> args;
            args.emplace_back(nullptr, &scale);

            auto rootWidget =
              _hubRootWidgetProp->GetValue<RED4ext::WeakHandle<RED4ext::IScriptable>>(activeHubController);
            RED4ext::ExecuteFunction(rootWidget.Lock(), _inkWidgetSetScaleF, nullptr, args);

            Margin margin{};
            i = 0;
            for (auto& itemH : itemControllersArr)
            {
                rootWidget = _choiceRootWidgetProp->GetValue<RED4ext::WeakHandle<RED4ext::IScriptable>>(itemH);
                args.clear();
                args.emplace_back(nullptr, &scale);
                RED4ext::ExecuteFunction(rootWidget.Lock(), _inkWidgetSetScaleF, nullptr, args);

                args.clear();
                margin.top = margin.bottom = 12;
                args.emplace_back(nullptr, &margin);
                RED4ext::ExecuteFunction(rootWidget.Lock(), _inkWidgetUpdateMarginF, nullptr, args);

                // args.clear();
                // bool value = selected == i;
                // args.emplace_back(nullptr, &value);
                // RED4ext::ExecuteFunction(itemH, _setSelectedF, nullptr, args);
                RED4ext::ExecuteFunction(itemH, _updateViewF, nullptr, {});
                // RED4ext::ExecuteFunction(itemH, _animateSelectionF, nullptr, {});

                ++i;
            }
            widgetsScaled = true;
        }

        bool forward = true;
        if (selectionOffset < 0)
        {
            selectionOffset *= -1;
            forward = false;
        }

        INPUT* inputs = new INPUT[selectionOffset * 2];
        for (i = 0; i < (selectionOffset * 2) - 1; i += 2)
        {
            inputs[i].type = INPUT_KEYBOARD;
            inputs[i].ki.wVk = forward ? VK_DOWN : VK_UP;
            inputs[i].ki.dwFlags = 0;

            inputs[i + 1].type = INPUT_KEYBOARD;
            inputs[i + 1].ki.wVk = forward ? VK_DOWN : VK_UP;
            inputs[i + 1].ki.dwFlags = KEYEVENTF_KEYUP;
        }

        SendInput(selectionOffset * 2, inputs, sizeof(INPUT));
        
        // ffs i spent over 2 weeks to figure out why it's always the first dialog applied when press 'F'
        // even if another option is selected and highlighted and SelectedIndex > 0. Man just fuck dis shit

        //auto blackboard = _blackboardProp->GetValue<RED4ext::Handle<RED4ext::IScriptable>>(so);
        //auto interactionsBBDef = _interactionsBBDefinitionProp->GetValue<RED4ext::Handle<RED4ext::IScriptable>>(so);
        //auto uiInteractionsSelectedIdx = _UIInteractionsSelectedIdxProp->GetValue<void*>(interactionsBBDef);
        //auto bbSetIntF = _blackboardCls->GetFunction("SetInt");
        //
        //args.clear();
        //args.emplace_back(nullptr, uiInteractionsSelectedIdx);
        //args.emplace_back(nullptr, &selected);
        //RED4ext::ExecuteFunction(blackboard, bbSetIntF, nullptr, args);
        //
        //auto selectedIdxProp = _inkWidgetControllerCls->GetProperty("selectedIndex");
        //selectedIdxProp->SetValue<int32_t>(so, selected);
        //args.clear();
        //args.emplace_back(nullptr, &selected);
        //RED4ext::ExecuteFunction(so, _onDialogsSelectF, nullptr, args);

        //auto bbGetIntF = _blackboardCls->GetFunction("GetInt");
        //interactionsBBDef = _interactionsBBDefinitionProp->GetValue<RED4ext::Handle<RED4ext::IScriptable>>(so);
        //uiInteractionsSelectedIdx = _UIInteractionsSelectedIdxProp->GetValue<void*>(interactionsBBDef);
        //int32_t selectedIdx = 0;
        //args.clear();
        //args.emplace_back(nullptr, uiInteractionsSelectedIdx);
        //RED4ext::ExecuteFunction(blackboard, bbGetIntF, &selectedIdx, args);

        //auto si = selectedIdxProp->GetValue<int32_t>(so);
        //spdlog::debug("{} {}", selectedIdx, si);

        return true;
    }
    return false;
}
