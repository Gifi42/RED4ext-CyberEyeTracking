#pragma once

#include "BaseInkWidgetController.hpp"

namespace CyberEyeTracking::Workers
{
    class CameraPitchWorker
    {
    private:
        RED4ext::CClassFunction* _fGetLocalOrientation;
        RED4ext::CClassFunction* _fSetLocalOrientation;
        RED4ext::CClassFunction* _fSetZRot;
        RED4ext::CClassFunction* _fSetXRot;

        RED4ext::CClass* _quaternionCls;
        RED4ext::IScriptable* _quaternion;
        RED4ext::CProperty* _iProp;
        RED4ext::CProperty* _jProp;
        RED4ext::CProperty* _kProp;
        RED4ext::CProperty* _rProp;
        
        float _i = 0, _j = 0, _k = 0, _r = 0;

        RED4ext::CClass* _playerPuppetCls;
        RED4ext::GameInstance* _gameInstance;
        RED4ext::IScriptable* RotateQuat(float value, bool isY);

    public:
        CameraPitchWorker() = default;
        ~CameraPitchWorker() = default;
        
        void Init(RED4ext::GameInstance* gameInstance);
        void SetPitch(float x, float y);
    };
}
