#pragma once

#include "BaseInkWidgetController.hpp"

namespace CyberEyeTracking::Workers
{
    class HealthBarWorker : public BaseInkWidgetController
    {
    public:
        HealthBarWorker()
            : BaseInkWidgetController("healthbarWidgetGameController"){};
        ~HealthBarWorker() = default;

        void Init();
    };
}
