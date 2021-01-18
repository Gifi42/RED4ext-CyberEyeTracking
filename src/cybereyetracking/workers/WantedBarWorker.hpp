#pragma once

#include "BaseInkWidgetController.hpp"

namespace CyberEyeTracking::Workers
{
    class WantedBarWorker : public BaseInkWidgetController
    {
    public:
        WantedBarWorker()
            : BaseInkWidgetController("WantedBarGameController"){};
        ~WantedBarWorker() = default;

        void Init();
    };
}
