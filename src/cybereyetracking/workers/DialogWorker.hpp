#pragma once

#include "BaseInkWidgetController.hpp"

namespace CyberEyeTracking::Workers
{
    class DialogWorker : public BaseInkWidgetController
    {
    private:
        RED4ext::CProperty* _selectedIdxProp;
        RED4ext::CProperty* _activeHubIdProp;
        RED4ext::CProperty* _hubControllersProp;
        RED4ext::CProperty* _dhlcIdProp;
        RED4ext::CProperty* _itemControllersProp;
        RED4ext::CProperty* _hubRootWidgetProp;
        RED4ext::CProperty* _choiceRootWidgetProp;

        //RED4ext::CClass* _blackboardCls;
        //RED4ext::CProperty* _blackboardProp;
        //RED4ext::CProperty* _interactionsBBDefinitionProp;
        //RED4ext::CClass* _UIInteractionsDefCls;
        //RED4ext::CProperty* _UIInteractionsSelectedIdxProp;

        RED4ext::CClassFunction* _updateViewF;
        RED4ext::CClass* _dialogHubLogicControllerCls;
        RED4ext::CClass* _dialogChoiceLogicControllerCls;
        /*RED4ext::CClassFunction* _setSelectedF;
        RED4ext::CClassFunction* _animateSelectionF;
        RED4ext::CClassFunction* _onDialogsSelectF;*/

        RED4ext::CClassFunction* _inkWidgetSetScaleF;
        RED4ext::CClassFunction* _inkWidgetUpdateMarginF;

        int activeHubId = 0;
        int selected = 0;
        bool widgetsScaled = false;

        bool Reset();
    public:
        DialogWorker()
            : BaseInkWidgetController("dialogWidgetGameController"){};
        ~DialogWorker() = default;

        void Init();
        bool SelectAtPos(float yPos);
    };
}
