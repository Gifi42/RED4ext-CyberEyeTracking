#pragma once

#include "BaseInkWidgetController.hpp"

namespace CyberEyeTracking::Workers
{
    class RadialWheelWorker : public BaseInkWidgetController
    {
    private:
        RED4ext::CProperty* _radialWeaponsProp;
        RED4ext::CProperty* _isActiveProp;
        RED4ext::CProperty* _activeIndexProp;
        RED4ext::CClassFunction* _determineActiveSlot;
        RED4ext::CClassFunction* _setActiveSlot;
        RED4ext::CClassFunction* _updatePointer;
    public:
        RadialWheelWorker()
            : BaseInkWidgetController("RadialWheelController"){};
        ~RadialWheelWorker() = default;

        void Init();
        void SetAngle(float angle);
    };
}
