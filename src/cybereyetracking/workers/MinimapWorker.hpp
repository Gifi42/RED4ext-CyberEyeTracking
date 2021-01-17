#pragma once

#include "BaseInkWidgetController.hpp"

namespace CyberEyeTracking::Workers
{
    class MinimapWorker : public BaseInkWidgetController
    {
    private:
        void SetOpacity(float value);
    public:
        MinimapWorker()
            : BaseInkWidgetController("gameuiMinimapContainerController"){};
        ~MinimapWorker() = default;

        void Init();
        void HideMiniMap();
        void ShowMiniMap();
    };
}
