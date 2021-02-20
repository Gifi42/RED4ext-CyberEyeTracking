#include <stdafx.hpp>

#include "HUDManagerWorker.hpp"

void CyberEyeTracking::Workers::HUDManagerWorker::Init()
{
    auto rtti = RED4ext::CRTTISystem::Get();
    auto getHudManagerF = rtti->GetClass("PlayerPuppet")->GetFunction("GetHudManager");
    RED4ext::Handle<RED4ext::IScriptable> playerHandle;
    RED4ext::ExecuteGlobalFunction("GetPlayer;GameInstance", &playerHandle, RED4ext::CGameEngine::Get()->framework->gameInstance);

    RED4ext::ExecuteFunction(playerHandle, getHudManagerF, &_hudManager, {});
    auto hudManagerCls = rtti->GetClass("HUDManager");
    _uiScannerVisibleProp = hudManagerCls->GetProperty("uiScannerVisible");
    _isHackingMinigameActive = hudManagerCls->GetProperty("isHackingMinigameActive");
}

bool CyberEyeTracking::Workers::HUDManagerWorker::IsScanning()
{
    if (!_hudManager)
        return false;

    return _uiScannerVisibleProp->GetValue<bool>(_hudManager);
}


bool CyberEyeTracking::Workers::HUDManagerWorker::IsHacking()
{
    if (!_hudManager)
        return false;

    return _isHackingMinigameActive->GetValue<bool>(_hudManager);
}
