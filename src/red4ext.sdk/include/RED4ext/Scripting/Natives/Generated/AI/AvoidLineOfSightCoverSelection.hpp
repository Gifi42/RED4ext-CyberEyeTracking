#pragma once

// clang-format off

// This file is generated from the Game's Reflection data

#include <cstdint>
#include <RED4ext/Common.hpp>
#include <RED4ext/Scripting/Natives/Generated/AI/CoverSelectionParameters.hpp>

namespace RED4ext
{
namespace AI
{
struct AvoidLineOfSightCoverSelection : AI::CoverSelectionParameters
{
    static constexpr const char* NAME = "AIAvoidLineOfSightCoverSelection";
    static constexpr const char* ALIAS = NAME;

    uint8_t unk10[0x20 - 0x10]; // 10
};
RED4EXT_ASSERT_SIZE(AvoidLineOfSightCoverSelection, 0x20);
} // namespace AI
using AIAvoidLineOfSightCoverSelection = AI::AvoidLineOfSightCoverSelection;
} // namespace RED4ext

// clang-format on
