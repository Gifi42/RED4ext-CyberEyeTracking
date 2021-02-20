#pragma once

namespace CyberEyeTracking::Workers
{
    class HUDManagerWorker
    {
    private:
        RED4ext::Handle<RED4ext::IScriptable> _hudManager;
        RED4ext::CProperty* _uiScannerVisibleProp;
        RED4ext::CProperty* _isHackingMinigameActive;
    public:
        HUDManagerWorker() = default;
        ~HUDManagerWorker() = default;

        void Init();
        bool IsScanning();
        bool IsHacking();
    };
}
