#include <stdafx.hpp>
#include <set>

#include "BaseInkWidgetController.hpp"
#include "CameraPitchWorker.hpp"


using ScriptableHandle = RED4ext::Handle<RED4ext::IScriptable>;

void CyberEyeTracking::Workers::CameraPitchWorker::Init(RED4ext::GameInstance* gameInstance)
{
    CyberEyeTracking::Workers::BaseInkWidgetController::Init();
    _fGetLocalOrientation = _inkWidgetControllerCls->GetFunction("GetLocalOrientation");
    _fSetLocalOrientation = _inkWidgetControllerCls->GetFunction("SetLocalOrientation");

    _quanterionCls = _rtti->GetClass("Quaternion");
    _initQuan = _quanterionCls->AllocInstance();

    _fSetZRot = _quanterionCls->GetFunction("SetZRot");
    _fSetXRot = _quanterionCls->GetFunction("SetXRot");

    _gameInstance = gameInstance;
    _playerPuppetCls = _rtti->GetClass("PlayerPuppet");
}

void CyberEyeTracking::Workers::CameraPitchWorker::SetPitch(float x, float y)
{
    for (auto& so : GetScriptObjects())
    {
        RED4ext::Handle<RED4ext::IScriptable> playerPuppetH;
        RED4ext::ExecuteGlobalFunction("GetPlayer;GameInstance", &playerPuppetH, _gameInstance);

        auto res = playerPuppetH->ExecuteFunction<RED4ext::Handle<RED4ext::IScriptable>>("GetFPPCameraComponent");
        if (!res.has_value())
            return;

        auto cameraH = res.value();
        
        RED4ext::IScriptable* quan = _quanterionCls->AllocInstance();

        std::vector<RED4ext::CStackType> args;
        

        if (x != 0 || y != 0)
        {
            if (x != 0)
            {
                args.emplace_back(nullptr, quan);
                args.emplace_back(nullptr, &x);
                RED4ext::ExecuteFunction(quan, _fSetZRot, nullptr, args);
            }

            if (y != 0)
            {
                args.clear();
                args.emplace_back(nullptr, quan);
                args.emplace_back(nullptr, &y);
                RED4ext::ExecuteFunction(quan, _fSetXRot, nullptr, args);
            }

            args.clear();
            args.emplace_back(nullptr, quan);
            RED4ext::ExecuteFunction(cameraH.GetPtr(), _fSetLocalOrientation, nullptr, args);
        }
        else
        {
            args.emplace_back(nullptr, quan);
            RED4ext::ExecuteFunction(cameraH.GetPtr(), _fSetLocalOrientation, nullptr, args);
        }
        return;
    }
}
