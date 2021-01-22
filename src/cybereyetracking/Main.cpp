#include <stdafx.hpp>

#include <chrono>
#include <set>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <Workers/BaseInkWidgetController.hpp>
#include <workers/LoadingGameWorker.hpp>

#include <EyeTracker.hpp>

#define RED4EXT_EXPORT extern "C" __declspec(dllexport)

CyberEyeTracking::Workers::BaseInkWidgetController _healthBarWorker = CyberEyeTracking::Workers::BaseInkWidgetController("healthbarWidgetGameController");
CyberEyeTracking::Workers::BaseInkWidgetController _minimapWorker = CyberEyeTracking::Workers::BaseInkWidgetController("gameuiMinimapContainerController");
CyberEyeTracking::Workers::BaseInkWidgetController _wantedBarWorker = CyberEyeTracking::Workers::BaseInkWidgetController("WantedBarGameController");
CyberEyeTracking::Workers::BaseInkWidgetController _questTrackerWidgetWorker = CyberEyeTracking::Workers::BaseInkWidgetController("QuestTrackerGameController");
CyberEyeTracking::Workers::BaseInkWidgetController _hotkeysWidgetWorker =  CyberEyeTracking::Workers::BaseInkWidgetController("HotkeysWidgetController");;
CyberEyeTracking::Workers::LoadingGameWorker _initialLoadingWorker = CyberEyeTracking::Workers::LoadingGameWorker("inkInitialLoadingScreenLogicController");
CyberEyeTracking::Workers::LoadingGameWorker _defaultLoadingWorker = CyberEyeTracking::Workers::LoadingGameWorker("inkDefaultLoadingScreenLogicController");
CyberEyeTracking::Workers::LoadingGameWorker _fasttravelLoadingWorker = CyberEyeTracking::Workers::LoadingGameWorker("inkFastTravelLoadingScreenLogicController");
CyberEyeTracking::Workers::LoadingGameWorker _splashscreenLoadingWorker = CyberEyeTracking::Workers::LoadingGameWorker("inkSplashScreenLoadingScreenLogicController");

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

RED4EXT_EXPORT void OnUpdate()
{    
    static auto timeStart = std::chrono::high_resolution_clock::now();
    static auto timeLoadingCheck = std::chrono::high_resolution_clock::now();
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
        _initialLoadingWorker.Init();
        _defaultLoadingWorker.Init();
        _splashscreenLoadingWorker.Init();
        _fasttravelLoadingWorker.Init();

        inkMenuScenarioCls = rtti->GetClass("inkMenuScenario");
        scriptGameInstanceCls = rtti->GetClass("ScriptGameInstance");

        initialized = true;
    }
    if (!initialized)
        return;
    
    RED4ext::ExecuteFunction(gameInstance, inkMenuScenarioCls->GetFunction("GetSystemRequestsHandler"), &sysHandlers, {});

    auto instance = sysHandlers.Lock();
    auto gamePaused = instance->ExecuteFunction<bool>("IsGamePaused", nullptr);
    if (!gamePaused.has_value() || gamePaused.value())
    {
        return;
    }

    RED4ext::Handle<RED4ext::IScriptable> handle;
    RED4ext::ExecuteGlobalFunction("GetPlayer;GameInstance", &handle, gameInstance);

    if (!handle.instance)
    {
        timeLoadingCheck = std::chrono::high_resolution_clock::now();
        return;
    }

    if ((now - timeLoadingCheck) < 10s)
        return;

    if (_initialLoadingWorker.Loading())
    {
        SHORT spacePress = GetAsyncKeyState(VK_SPACE);
        if ((1 << 15) & spacePress)
            timeLoadingCheck = std::chrono::high_resolution_clock::now();
        
        return;
    }
    if (_defaultLoadingWorker.Loading() || _splashscreenLoadingWorker.Loading() || _fasttravelLoadingWorker.Loading())
    {
        timeLoadingCheck = std::chrono::high_resolution_clock::now();
        return;
    }

    if ((now - timeStart) > 35s)
    {
        _initialLoadingWorker.Erase();
        _defaultLoadingWorker.Erase();
        _splashscreenLoadingWorker.Erase();
        _fasttravelLoadingWorker.Erase();


        float* pos = _eyeTracker.GetPos();
        float x = pos[0];
        float y = pos[1];

        // ================ CLEAR UI ==============
        if (x >= 0 && x <= 0.25 && //(0-480)
            y >=0  && y <= 0.165) // (0-110)
        {
            _healthBarWorker.ShowWidget();
        }
        else
        {
            _healthBarWorker.HideWidget();
        }

        if (x >= 0.848958333 && x <= 0.971875 // (1630-1866)
            && y >= 0.037037 && y <= 0.3055) // (41-330)
        {
            _minimapWorker.ShowWidget();
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

        if (x >= 0.786458333 && x <= 0.953125          // (1510-1830)
            && y >= 0.35185185 && y <= 0.555555555555) // (380-600)
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
        }
        else
        {
            _hotkeysWidgetWorker.HideWidget();
        }
    }
    
}
