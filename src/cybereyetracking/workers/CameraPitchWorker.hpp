#pragma once

#include "BaseInkWidgetController.hpp"

namespace CyberEyeTracking::Workers
{
    class CameraPitchWorker : public BaseInkWidgetController
    {
    private:
        RED4ext::CClassFunction* _fGetLocalOrientation;
        RED4ext::CClassFunction* _fSetLocalOrientation;
        RED4ext::CClassFunction* _fSetZRot;
        RED4ext::CClassFunction* _fSetXRot;

        RED4ext::CClass* _quanterionCls;
        RED4ext::IScriptable* _initQuan;

        RED4ext::CClass* _playerPuppetCls;
        RED4ext::GameInstance* _gameInstance;

    public:
        CameraPitchWorker()
            : BaseInkWidgetController("gameFPPCameraComponent"){};
        ~CameraPitchWorker() = default;

        void Init(RED4ext::GameInstance* gameInstance);
        void SetPitch(float x, float y);
    };
}
