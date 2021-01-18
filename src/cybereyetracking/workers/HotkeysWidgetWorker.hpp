#pragma once

#include "BaseInkWidgetController.hpp"

namespace CyberEyeTracking::Workers
{
    class HotkeysWidgetWorker : public BaseInkWidgetController
    {
    public:
        HotkeysWidgetWorker()
            : BaseInkWidgetController("HotkeysWidgetController"){};
        ~HotkeysWidgetWorker() = default;

        void Init();
    };
}
