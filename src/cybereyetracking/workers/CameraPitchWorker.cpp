#include <stdafx.hpp>
#include <set>

#include "BaseInkWidgetController.hpp"
#include "CameraPitchWorker.hpp"


using ScriptableHandle = RED4ext::Handle<RED4ext::IScriptable>;

void CyberEyeTracking::Workers::CameraPitchWorker::Init(RED4ext::GameInstance* gameInstance)
{
    _fGetLocalOrientation = RED4ext::CRTTISystem::Get()->GetClass("gameFPPCameraComponent")->GetFunction("GetLocalOrientation");
    _fSetLocalOrientation = RED4ext::CRTTISystem::Get()->GetClass("gameFPPCameraComponent")->GetFunction("SetLocalOrientation");

    _quaternionCls = RED4ext::CRTTISystem::Get()->GetClass("Quaternion");
    _iProp = _quaternionCls->GetProperty("i");
    _jProp = _quaternionCls->GetProperty("j");
    _kProp = _quaternionCls->GetProperty("k");
    _rProp = _quaternionCls->GetProperty("r");

    _fSetZRot = _quaternionCls->GetFunction("SetZRot");
    _fSetXRot = _quaternionCls->GetFunction("SetXRot");

    _gameInstance = gameInstance;
}

RED4ext::IScriptable* CyberEyeTracking::Workers::CameraPitchWorker::RotateQuat(float value, bool isY)
{
    std::vector<RED4ext::CStackType> args;
    auto quat = _quaternionCls->AllocInstance();
    args.emplace_back(nullptr, quat);
    args.emplace_back(nullptr, &value);
    RED4ext::ExecuteFunction(quat, isY ? _fSetXRot : _fSetZRot, nullptr, args);

    _i += _iProp->GetValue<float>(quat);
    _j += _jProp->GetValue<float>(quat);
    _k += _kProp->GetValue<float>(quat);
    _r += _rProp->GetValue<float>(quat);

    return quat;
}

void CyberEyeTracking::Workers::CameraPitchWorker::SetPitch(float x, float y)
{
    RED4ext::Handle<RED4ext::IScriptable> playerPuppetH;
    RED4ext::ExecuteGlobalFunction("GetPlayer;GameInstance", &playerPuppetH, _gameInstance);

    auto res = playerPuppetH->ExecuteFunction<RED4ext::Handle<RED4ext::IScriptable>>("GetFPPCameraComponent");
    if (!res.has_value())
        return;

    auto cameraH = res.value();
    
    _i = 0, _j = 0, _k = 0, _r = 0;
    _quaternion = _quaternionCls->AllocInstance();
    std::vector<RED4ext::CStackType> args;
    if (x != 0 || y != 0)
    {
        if (x != 0)
        {
            RotateQuat(x, false);
        }

        if (y != 0)
        {
            RotateQuat(y, true);
        }

        _iProp->SetValue(_quaternion, _i);
        _jProp->SetValue(_quaternion, _j);
        _kProp->SetValue(_quaternion, _k);
        _rProp->SetValue(_quaternion, _r);
        args.emplace_back(nullptr, _quaternion);
        RED4ext::ExecuteFunction(cameraH.GetPtr(), _fSetLocalOrientation, nullptr, args);
    }
    else
    {
        args.emplace_back(nullptr, _quaternion);
        RED4ext::ExecuteFunction(cameraH.GetPtr(), _fSetLocalOrientation, nullptr, args);
    }
    return;
}
