#pragma once

#include "BaseInkWidgetController.hpp"

namespace CyberEyeTracking::Workers
{
    class MinimapWorker : public BaseInkWidgetController
    {
    public:
        MinimapWorker()
            : BaseInkWidgetController("gameuiMinimapContainerController"){};
        ~MinimapWorker() = default;

        void Init();
    };
}
