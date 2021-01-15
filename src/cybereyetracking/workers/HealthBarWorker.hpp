#pragma once

namespace CyberEyeTracking::Workers
{
    class HealthBarWorker
    {
    public:
        HealthBarWorker() = default;
        ~HealthBarWorker() = default;

        void Init();
        void HideHPBar();
        void ShowHPBar();
    };
}
