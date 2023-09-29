#pragma once

// clang-format off

// This file is generated from the Game's Reflection data

#include <cstdint>
#include <RED4ext/Common.hpp>
#include <RED4ext/Scripting/Natives/Generated/AI/behavior/PassiveExpressionDefinition.hpp>

namespace RED4ext
{
namespace AI::behavior
{
struct VehicleExpressionDefinition : AI::behavior::PassiveExpressionDefinition
{
    static constexpr const char* NAME = "AIbehaviorVehicleExpressionDefinition";
    static constexpr const char* ALIAS = NAME;

};
RED4EXT_ASSERT_SIZE(VehicleExpressionDefinition, 0x40);
} // namespace AI::behavior
using AIbehaviorVehicleExpressionDefinition = AI::behavior::VehicleExpressionDefinition;
} // namespace RED4ext

// clang-format on
