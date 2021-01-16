#pragma once

namespace CyberEyeTracking::Workers
{
    class BaseInkWidgetController
    {
    protected:
        RED4ext::CClass* GetInkWidgetControllerCls();
        std::set<uint64_t> GetScriptObjects();
        RED4ext::CClass* _inkWidgetCls = nullptr;
    public:
        BaseInkWidgetController(char* ctrlrRTTIname);
        ~BaseInkWidgetController() = default;
        void InitBase();
    };
}
