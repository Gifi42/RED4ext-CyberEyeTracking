#pragma once

namespace CyberEyeTracker::Workers
{
    class HealthBarWorker
    {
    public:
        HealthBarWorker() = default;
        ~HealthBarWorker() = default;

        void Init();
        void HideHPBar();
    };
}
