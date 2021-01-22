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
        bool _singleton;
        bool CheckScriptObject(RED4ext::IScriptable* scriptObject);
        void SetRootOpacity(float value);
    public:
        BaseInkWidgetController(char* ctrlrRTTIname, bool singleton = true);
        ~BaseInkWidgetController() = default;
        void Init();
        void HideWidget();
        void ShowWidget();
        bool Exist();
        void Erase();
    };
}
