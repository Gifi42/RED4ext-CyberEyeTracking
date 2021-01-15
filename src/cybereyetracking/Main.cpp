#include <stdafx.hpp>

#include <chrono>
#include <set>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <HealthBarWorker.hpp>

#define RED4EXT_EXPORT extern "C" __declspec(dllexport)

CyberEyeTracker::Workers::HealthBarWorker _healthBarWorker;


void InitializeLogger(std::filesystem::path aRoot)
{
    auto console = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
    auto file = std::make_shared<spdlog::sinks::basic_file_sink_st>(aRoot / L"CyberEyeTracking.log", true);

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
    InitializeLogger(L"\\");
    _healthBarWorker = CyberEyeTracker::Workers::HealthBarWorker();
}

RED4EXT_EXPORT void OnInitialization()
{
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
    static auto lastGet = std::chrono::high_resolution_clock::now();
    static auto lastSet = std::chrono::high_resolution_clock::now();
    static bool hooked = false;
    auto now = std::chrono::high_resolution_clock::now();

    using namespace std::chrono_literals;
    if ((now - lastGet) >= 10s && !hooked)
    {
        _healthBarWorker.Init();
        hooked = true;
    }
    _healthBarWorker.HideHPBar();
}

RED4EXT_EXPORT void OnShutdown()
{
    
}
