#include <stdafx.hpp>

#include <chrono>
#include <set>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <Workers/HealthBarWorker.hpp>
#include <Workers/MinimapWorker.hpp>
#include <EyeTracker.hpp>

#define RED4EXT_EXPORT extern "C" __declspec(dllexport)

CyberEyeTracking::Workers::HealthBarWorker _healthBarWorker;
CyberEyeTracking::Workers::MinimapWorker _minimapWorker;

CyberEyeTracking::EyeTracker _eyeTracker;

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
    _healthBarWorker = CyberEyeTracking::Workers::HealthBarWorker();
    _minimapWorker = CyberEyeTracking::Workers::MinimapWorker();
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

RED4ext::CClass* s_gameuiHUDGameControllerCls = nullptr;

RED4EXT_EXPORT void OnUpdate()
{    
    static auto timeStart = std::chrono::high_resolution_clock::now();
    static bool hooked = false;
    static bool trackerFound = false;

    auto now = std::chrono::high_resolution_clock::now();

    using namespace std::chrono_literals;

    if (!trackerFound)
    {
        bool initRes = _eyeTracker.Init();
        if (initRes)
            spdlog::info("Eye tracker found!");

        trackerFound = true;
        return;
    }        

    if ((now - timeStart) >= 10s && !hooked)
    {
        _healthBarWorker.Init();
        _minimapWorker.Init();
        hooked = true;
    }
    if (!hooked)
        return;

    if ((now - timeStart) > 45s)
    {
        float* pos = _eyeTracker.GetPos();
        float x = pos[0];
        float y = pos[1];
        
        if (x >= 0 && x <= 0.25 && //(0-480)
            y >=0  && y <= 0.165) // (0-110)
        {
            _healthBarWorker.ShowHPBar();
        }
        else
        {
            _healthBarWorker.HideHPBar();
            _minimapWorker.HideMiniMap();
        }

        if (x >= 0.833 && x <= 1      // (1600-1920)
            && y >= 0 && y <= 0.3055) // (0-330)
        {
            _minimapWorker.ShowMiniMap();
        }
        else
        {
            _minimapWorker.HideMiniMap();
        }
    }
    
}
