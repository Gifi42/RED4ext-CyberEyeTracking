#pragma once

// clang-format off

// This file is generated from the Game's Reflection data

#include <cstdint>
#include <RED4ext/Common.hpp>
#include <RED4ext/CName.hpp>
#include <RED4ext/Scripting/Natives/Generated/anim/AnimNode_IntValue.hpp>

namespace RED4ext
{
namespace anim
{
struct AnimNode_IntInput : anim::AnimNode_IntValue
{
    static constexpr const char* NAME = "animAnimNode_IntInput";
    static constexpr const char* ALIAS = NAME;

    CName group; // 48
    CName name; // 50
    uint8_t unk58[0x68 - 0x58]; // 58
};
RED4EXT_ASSERT_SIZE(AnimNode_IntInput, 0x68);
} // namespace anim
using animAnimNode_IntInput = anim::AnimNode_IntInput;
} // namespace RED4ext

// clang-format on