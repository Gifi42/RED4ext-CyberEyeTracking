#include <stdafx.hpp>

#include <chrono>
#include <set>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <Workers/BaseInkWidgetController.hpp>
#include <Workers/RadialWheelWorker.hpp>
#include <Workers/CameraPitchWorker.hpp>
#include <Workers/DialogWorker.hpp>


#include <EyeTracker.hpp>
#include "Utils.hpp"

#define CAMERA_PITCH_LOOK_START 0.5 // 0.338 Screen border where we start pitching camera

#define CAMERA_PITCH_PARABOLA_A 1
#define CAMERA_PITCH_PARABOLA_B -1
#define CAMERA_PITCH_PARABOLA_C 0.25

#define CAMERA_PITCH_RESET_START 0.25

#define RED4EXT_EXPORT extern "C" __declspec(dllexport)

CyberEyeTracking::Workers::BaseInkWidgetController _healthBarWorker = CyberEyeTracking::Workers::BaseInkWidgetController("healthbarWidgetGameController");
CyberEyeTracking::Workers::BaseInkWidgetController _minimapWorker = CyberEyeTracking::Workers::BaseInkWidgetController("gameuiMinimapContainerController");
CyberEyeTracking::Workers::BaseInkWidgetController _wantedBarWorker = CyberEyeTracking::Workers::BaseInkWidgetController("WantedBarGameController");
CyberEyeTracking::Workers::BaseInkWidgetController _questTrackerWidgetWorker = CyberEyeTracking::Workers::BaseInkWidgetController("QuestTrackerGameController");
CyberEyeTracking::Workers::BaseInkWidgetController _hotkeysWidgetWorker =  CyberEyeTracking::Workers::BaseInkWidgetController("HotkeysWidgetController");

CyberEyeTracking::Workers::RadialWheelWorker _radialWheelWorker =  CyberEyeTracking::Workers::RadialWheelWorker();

CyberEyeTracking::Workers::CameraPitchWorker _cameraPitchWorker = CyberEyeTracking::Workers::CameraPitchWorker();

CyberEyeTracking::Workers::DialogWorker _dialogWorker = CyberEyeTracking::Workers::DialogWorker();


CyberEyeTracking::EyeTracker _eyeTracker;

RED4ext::WeakHandle<RED4ext::IScriptable> sysHandlers;
RED4ext::CClass* scriptGameInstanceCls;
RED4ext::CClass* inkMenuScenarioCls;

void InitializeLogger(std::filesystem::path aRoot)
{
    auto console = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
    auto file = std::make_shared<spdlog::sinks::basic_file_sink_st>(aRoot / L"cetrack.log", true);

    spdlog::sinks_init_list sinks = { console, file };

    auto logger = std::make_shared<spdlog::logger>("", sinks);
    spdlog::set_default_logger(logger);

#ifdef _DEBUG
    logger->flush_on(spdlog::level::trace);
    spdlog::set_level(spdlog::level::trace);
#else
    logger->flush_on(spdlog::level::info);
    spdlog::set_level(spdlog::level::info);
#endif
}


RED4EXT_EXPORT void OnBaseInitialization()
{
    InitializeLogger(L"");
    
    _eyeTracker = CyberEyeTracking::EyeTracker();
}

RED4EXT_EXPORT void OnInitialization()
{
    spdlog::info("Looking for a connected eye tracking device");
}


RED4EXT_EXPORT void OnShutdown()
{
    _eyeTracker.Finalize();
}

void LogClassFlags(RED4ext::CClass* cc)
{
    const auto& flgs = cc->flags;
    spdlog::debug("Flags: {:08X}", *(uint32_t*)&flgs);
    spdlog::debug("Flags.isAbstract:    {}", flgs.isAbstract);
    spdlog::debug("Flags.isNative:      {}", flgs.isNative);
    spdlog::debug("Flags.b2:            {}", flgs.b2);
    spdlog::debug("Flags.b3:            {}", flgs.b3);
    spdlog::debug("Flags.isStruct:      {}", flgs.isStruct);
    spdlog::debug("Flags.b5:            {}", flgs.b5);
    spdlog::debug("Flags.isImportOnly:  {}", flgs.isImportOnly);
    spdlog::debug("Flags.isPrivate:     {}", flgs.isPrivate);
    spdlog::debug("Flags.isProtected:   {}", flgs.isProtected);
    spdlog::debug("Flags.isTestOnly:    {}", flgs.isTestOnly);
}

void LogHandle(RED4ext::HandleBase* handle)
{
    if (!handle)
    {
        spdlog::debug("handle == nullptr");
        return;
    }

    if (handle->instance)
    {
        spdlog::debug("handle.instance: {:X}", handle->instance);
    }
    else
    {
        spdlog::debug("handle.instance: nullptr");
    }

    if (handle->refCount)
    {
        spdlog::debug("handle.refCount: {} {}", handle->refCount->strongRefs, handle->refCount->weakRefs);
    }
    else
    {
        spdlog::debug("handle.refCount: nullptr");
    }
}

float GetCamPitch(float pos, bool negative)
{
    return negative ?
        -CyberEyeTracking::Math::GetParametrizedParabola(CAMERA_PITCH_PARABOLA_A, CAMERA_PITCH_PARABOLA_B, CAMERA_PITCH_PARABOLA_C, 1 - pos) :
        CyberEyeTracking::Math::GetParametrizedParabola(CAMERA_PITCH_PARABOLA_A, CAMERA_PITCH_PARABOLA_B,CAMERA_PITCH_PARABOLA_C, pos);
}

static bool resetPitch = false;
static float prevX = 0;
static float prevY = 0;

void StartResetPitch(float x, float y)
{
    if (resetPitch)
        return;

    resetPitch = true;
    prevX = x;
    prevY = y;
}

RED4EXT_EXPORT void OnUpdate()
{    
    static auto timeStart = std::chrono::high_resolution_clock::now();
    static auto loadCheck = std::chrono::high_resolution_clock::now();
    static bool initialized = false;
    static bool trackerFound = false;

    auto now = std::chrono::high_resolution_clock::now();
    auto static gameInstance = RED4ext::CGameEngine::Get()->framework->gameInstance;
    using namespace std::chrono_literals;
    auto rtti = RED4ext::CRTTISystem::Get();

    if (!trackerFound)
    {
        bool initRes = _eyeTracker.Init();
        if (initRes)
            spdlog::info("Eye tracker found!");

        trackerFound = true;
        return;
    }        
   
    if ((now - timeStart) >= 10s && !initialized)
    {
        _healthBarWorker.Init();
        _minimapWorker.Init();
        _wantedBarWorker.Init();
        _questTrackerWidgetWorker.Init();
        _hotkeysWidgetWorker.Init();
        _radialWheelWorker.Init();
        _cameraPitchWorker.Init(gameInstance);
        _dialogWorker.Init();

        inkMenuScenarioCls = rtti->GetClass("inkMenuScenario");
        scriptGameInstanceCls = rtti->GetClass("ScriptGameInstance");
        
        initialized = true;
    }
    if (!initialized || (now - loadCheck) < 15s)
    {
        return;
    }

    RED4ext::ExecuteFunction(gameInstance, inkMenuScenarioCls->GetFunction("GetSystemRequestsHandler"), &sysHandlers, {});

    auto instance = sysHandlers.Lock();
    if (!instance || _dialogWorker.ObjectsCount() == 0 || (now - timeStart) < 30s)
    {
        loadCheck = std::chrono::high_resolution_clock::now();
        _dialogWorker.Erase();
        return;
    }

    auto gamePaused = instance->ExecuteFunction<bool>("IsGamePaused", nullptr);
    if (!gamePaused.has_value() || gamePaused.value())
    {
        return;
    }
    

    //args.clear();
    //args.emplace_back(nullptr, &q000started);
    //int32_t q000startedRes = 0;
    //RED4ext::ExecuteFunction(questSystem, getFactStr, &q000startedRes, args);
    //if (q000startedRes == 1)
    //    return; // check if game is loading https://discord.com/channels/717692382849663036/794613856948322344/801953012146896937

    float* pos = _eyeTracker.GetPos();
    float x = pos[0];
    float y = pos[1];
    if (x > 1)
        x = 1;
    else if (x < 0)
        x = 0;
    if (y > 1)
        y = 1;
    else if (y < 0)
        y = 0;

        
        // ================ WHEEL SELECT ==============
    if (_radialWheelWorker.ObjectsCount() > 0)
    {
        float angle = CyberEyeTracking::Math::GetAngle(x, y);
        if (_radialWheelWorker.SetAngle(angle))
            return;
    }

    // ================ CLEAN UI ==============
        
    if (x >= 0 && x <= 0.25 && //(0-480)
        y >= 0 && y <= 0.165)  // (0-110)
    {
        _healthBarWorker.ShowWidget();
        StartResetPitch(x, y);
    }
    else
    {
        _healthBarWorker.HideWidget();
    }

    if (x >= 0.848958333 && x <= 0.971875 // (1630-1866)
        && y >= 0.037037 && y <= 0.3055)  // (41-330)
    {
        _minimapWorker.ShowWidget();
        StartResetPitch(x, y);
    }
    else
    {
        _minimapWorker.HideWidget();
    }

    if (x >= 0.8208333 && x <= 0.848958333 // (1575-1630)
        && y >= 0.055555555 && y <= 0.25)  // (60-270)
    {
        _wantedBarWorker.ShowWidget();
    }
    else
    {
        _wantedBarWorker.HideWidget();
    }

    if (x >= 0.786458333 && x <= 0.9442708333333333 // (1510-1813)
        && y >= 0.35185185 && y <= 0.5) // (380-540)
    {
        _questTrackerWidgetWorker.ShowWidget();
    }
    else
    {
        _questTrackerWidgetWorker.HideWidget();
    }

    if (x >= 0.03125 && x <= 0.161458333 // (60-310)
        && y >= 0.8703703 && y <= 1)     // (940-1080)
    {
        _hotkeysWidgetWorker.ShowWidget();
        StartResetPitch(x, y);
    }
    else
    {
        _hotkeysWidgetWorker.HideWidget();
    }

    // ================ DIALOGUE SELECT ==============
    if (_dialogWorker.SelectAtPos(y))
        return;

    // ================ CAMERA PITCH ==============

    bool pitchLeft = x <= CAMERA_PITCH_LOOK_START;
    bool pitchRight = x >= 1 - CAMERA_PITCH_LOOK_START;
    bool pitchUp = y <= CAMERA_PITCH_LOOK_START;
    bool pitchDown = y >= 1 - CAMERA_PITCH_LOOK_START;

    float pitchX = 0;
    float pitchY = 0;

    if (resetPitch)
    {
        if (x > CAMERA_PITCH_RESET_START && x < 1 - CAMERA_PITCH_RESET_START
            && y > CAMERA_PITCH_RESET_START && y < 1 - CAMERA_PITCH_RESET_START)
        {
            resetPitch = false;
        }
        else
        {
            x = prevX;
            y = prevY;
        }
    }
    pitchX = GetCamPitch(x, pitchRight);
    pitchY = GetCamPitch(y, pitchDown);
        
    _cameraPitchWorker.SetPitch(pitchX, pitchY);

    // ================ LOOK AT LOOT ==============
    /* RED4ext::Handle<RED4ext::IScriptable> targetSystem;
    std::vector<RED4ext::CStackType> args;
    args.emplace_back(nullptr, &gameInstance);

    auto f = rtti->GetClass("ScriptGameInstance")->GetFunction("GetTargetingSystem");
    RED4ext::ExecuteFunction(gameInstance, f, &targetSystem, args);

    if (!targetSystem || !targetSystem.instance)
        return;

    RED4ext::Handle<RED4ext::IScriptable> playerH;
    RED4ext::ExecuteGlobalFunction("GetPlayerObject;GameInstance", &playerH, gameInstance);
    RED4ext::WeakHandle<RED4ext::IScriptable> playerWH{};
    playerWH = playerH;

    auto anglesCls = rtti->GetClass("EulerAngles");
    auto fRand = anglesCls->GetFunction("Rand");
    auto angles = anglesCls->AllocInstance();

    anglesCls->GetProperty("Pitch")->SetValue<float>(angles, 0);
    anglesCls->GetProperty("Roll")->SetValue<float>(angles, 0);
    anglesCls->GetProperty("Yaw")->SetValue<float>(angles, 45 * x);

    auto gtsCls = rtti->GetClass("gametargetingTargetingSystem");
    auto fGetObjClosest = gtsCls->GetFunction("GetObjectClosestToCrosshair");

    args.clear();
    args.emplace_back(nullptr, &playerWH);
    args.emplace_back(nullptr, &angles);
    args.emplace_back(nullptr, query);
    RED4ext::Handle<RED4ext::IScriptable> gameObj;
    RED4ext::ExecuteFunction(targetSystem.instance, fGetObjClosest, &gameObj, args);

    if (!gameObj || !gameObj.instance)
        return;

    RED4ext::CName className;
    auto gameObjCls = rtti->GetClass("gameObject");
    auto getNameFunc = gameObjCls->GetFunction("GetClassName");
    RED4ext::ExecuteFunction(gameObj.instance, getNameFunc, &className, {});

    spdlog::debug(className.ToString());*/
}
