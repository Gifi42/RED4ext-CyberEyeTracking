#pragma once

#include "BaseInkWidgetController.hpp"

namespace CyberEyeTracking::Workers
{
    class QuestTrackerWorker : public BaseInkWidgetController
    {
    public:
        QuestTrackerWorker()
            : BaseInkWidgetController("QuestTrackerGameController"){};
        ~QuestTrackerWorker() = default;

        void Init();
    };
}
