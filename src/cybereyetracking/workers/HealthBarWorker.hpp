#pragma once

#include "BaseInkWidgetController.hpp"

namespace CyberEyeTracking::Workers
{
    class HealthBarWorker : public BaseInkWidgetController
    {
    private:
        void SetOpacity(float value);
    public:
        HealthBarWorker()
            : BaseInkWidgetController("healthbarWidgetGameController"){};
        ~HealthBarWorker() = default;

        void Init();
        void HideHPBar();
        void ShowHPBar();
    };
}
