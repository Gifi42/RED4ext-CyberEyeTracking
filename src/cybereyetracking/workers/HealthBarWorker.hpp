#pragma once

#include "BaseInkWidgetController.h"

namespace CyberEyeTracking::Workers
{
    class HealthBarWorker : BaseInkWidgetController
    {
    public:
        HealthBarWorker();
        ~HealthBarWorker() = default;

        void Init();
        void HideHPBar();
        void ShowHPBar();
    };
}
