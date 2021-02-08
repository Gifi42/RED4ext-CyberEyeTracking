#pragma once

#include "BaseInkWidgetController.hpp"

namespace CyberEyeTracking::Workers
{
    class DialogWorker : public BaseInkWidgetController
    {
    private:
        RED4ext::CProperty* _activeHubIdProp;
        RED4ext::CProperty* _hubControllersProp;
        RED4ext::CProperty* _dhlcIdProp;
        RED4ext::CProperty* _itemControllersProp;

        RED4ext::CClass* _dialogHubLogicControllerCls;
        RED4ext::CClass* _dialogChoiceLogicControllerCls;
        RED4ext::CClassFunction* _setSelectedF;
        RED4ext::CClassFunction* _updateViewF;
        RED4ext::CClassFunction* _updateColorsF;
        RED4ext::CClassFunction* _animateSelectionF;
        
        int activeHubId = 0;
        int selected = 0;
    public:
        DialogWorker()
            : BaseInkWidgetController("dialogWidgetGameController"){};
        ~DialogWorker() = default;

        void Init();
        bool SelectAtPos(float yPos);
    };
}
