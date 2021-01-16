#pragma once

namespace CyberEyeTracking::Workers
{
    class BaseInkWidgetController
    {
    private:
        static bool vtblHooked;
    protected:
        RED4ext::CRTTISystem* _rtti;
        RED4ext::CClass* _inkWidgetControllerCls;
        std::set<RED4ext::IScriptable*> GetScriptObjects();
        RED4ext::CClass* _inkWidgetCls = nullptr;
        RED4ext::CName _ctrlrRTTIname;
    public:
        BaseInkWidgetController(char* ctrlrRTTIname);
        ~BaseInkWidgetController() = default;
        void InitBase();
    };
}
