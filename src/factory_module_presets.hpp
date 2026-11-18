// Pedal Suite — factory pedal presets
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <array>

#include "module_ids.hpp"

namespace pedal {

struct FactoryModulePreset {
  const char* name;
  const char* source;
  ModuleId type;
  std::array<float, 12> controls;
};

inline constexpr FactoryModulePreset kFactoryModulePresets[] = {
    {"Clean Lift", "Pedal Suite exclusive", ModuleId::Boost,
     {0.15f, 0.50f, 0.55f}},
    {"Treble Push", "Pedal Suite exclusive", ModuleId::Boost,
     {0.10f, 0.75f, 0.50f}},
    {"Full Boost", "Pedal Suite exclusive", ModuleId::Boost,
     {0.50f, 0.50f, 0.45f}},
    {"Always On", "Amp Suite", ModuleId::Drive808,
     {0.28f, 0.50f, 0.55f}},
    {"Lead", "Amp Suite", ModuleId::Drive808,
     {0.65f, 0.55f, 0.50f}},
    {"Bass Warmth", "Amp Suite", ModuleId::Drive808,
     {0.30f, 0.35f, 0.50f}},
};

inline constexpr int kFactoryModulePresetCount =
    static_cast<int>(sizeof(kFactoryModulePresets) /
                     sizeof(kFactoryModulePresets[0]));

} // namespace pedal
