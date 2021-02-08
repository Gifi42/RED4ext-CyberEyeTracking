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
        RED4ext::CClass* _inkWidgetCls;
        RED4ext::CClassFunction* _getMarginF;
        RED4ext::CClassFunction* _getPaddingF;

        RED4ext::CName _ctrlrRTTIname;
        bool _singleton;
        bool CheckScriptObject(RED4ext::IScriptable* scriptObject);
        
    public:
        BaseInkWidgetController(const char* ctrlrRTTIname, bool singleton = true);
        ~BaseInkWidgetController() = default;
        void Init();
        void HideWidget();
        void ShowWidget();
        bool Exist();
        void Erase();
        int ObjectsCount();
        std::set<RED4ext::IScriptable*> GetScriptObjects();
        void SetRootOpacity(float value);
    };

    
    struct inkMargin
    {
        float bottom;
        float left;
        float right;
        float top;
    };
}
