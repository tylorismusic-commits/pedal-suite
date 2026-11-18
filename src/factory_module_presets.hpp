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
    {"Bright Lift", "Pedal Suite exclusive", ModuleId::TrebleBoost,
     {0.55f, 0.50f, 0.50f}},
    {"Full Range", "Pedal Suite exclusive", ModuleId::TrebleBoost,
     {0.78f, 0.82f, 0.45f}},
    {"Transparent", "Pedal Suite exclusive", ModuleId::Overdrive,
     {0.35f, 0.50f, 0.50f, 1.0f}},
    {"Push", "Pedal Suite exclusive", ModuleId::Overdrive,
     {0.72f, 0.58f, 0.45f, 1.0f}},
    {"Just Breaking", "Pedal Suite exclusive", ModuleId::BluesDrive,
     {0.40f, 0.50f, 0.50f}},
    {"More Drive", "Pedal Suite exclusive", ModuleId::BluesDrive,
     {0.72f, 0.60f, 0.48f}},
    {"Rhythm", "Pedal Suite exclusive", ModuleId::Distortion,
     {0.50f, 0.50f, 0.50f}},
    {"Lead", "Pedal Suite exclusive", ModuleId::Distortion,
     {0.82f, 0.60f, 0.45f}},
    {"Classic Rat", "Pedal Suite exclusive", ModuleId::Rat,
     {0.50f, 0.50f, 0.50f}},
    {"Filtered", "Pedal Suite exclusive", ModuleId::Rat,
     {0.72f, 0.30f, 0.50f}},
    {"Wall", "Pedal Suite exclusive", ModuleId::BigFuzz,
     {0.82f, 0.50f, 0.45f}},
    {"Fuzz Lead", "Pedal Suite exclusive", ModuleId::BigFuzz,
     {0.62f, 0.66f, 0.50f}},
    {"Cleanup", "Pedal Suite exclusive", ModuleId::FuzzFace,
     {0.42f, 0.50f, 0.60f}},
    {"Full Fuzz", "Pedal Suite exclusive", ModuleId::FuzzFace,
     {0.88f, 0.50f, 0.45f}},
    {"Classic Bender", "Pedal Suite exclusive", ModuleId::ToneBender,
     {0.62f, 0.50f, 0.50f}},
    {"Bender Lead", "Pedal Suite exclusive", ModuleId::ToneBender,
     {0.86f, 0.62f, 0.45f}},
    {"Bass Grit", "Pedal Suite exclusive", ModuleId::BassDrive,
     {0.45f, 0.80f, 0.50f, 0.50f}},
    {"Bass Push", "Pedal Suite exclusive", ModuleId::BassDrive,
     {0.75f, 0.65f, 0.55f, 0.45f}},
    {"Bass Wall", "Pedal Suite exclusive", ModuleId::BassFuzz,
     {0.70f, 0.85f, 0.45f, 0.48f}},
    {"Bass Fuzz Lead", "Pedal Suite exclusive", ModuleId::BassFuzz,
     {0.92f, 0.72f, 0.60f, 0.42f}},
    {"Whammy +12", "Pedal Suite exclusive", ModuleId::PitchDrop,
     {0.75f, 0.00f, 0.10f, 0.55f, 1.00f}},
    {"Whammy Fifth", "Pedal Suite exclusive", ModuleId::PitchDrop,
     {0.65f, 0.00f, 0.25f, 0.55f, 0.70f}},
    {"Whammy -12", "Pedal Suite exclusive", ModuleId::PitchDrop,
     {0.25f, 0.25f, 0.10f, 0.45f, 1.00f}},
    {"Drop D", "Pedal Suite exclusive", ModuleId::PitchDrop,
     {0.25f, 0.50f, 0.05f, 0.50f, 1.00f}},
    {"Detune Wide", "Pedal Suite exclusive", ModuleId::PitchDrop,
     {0.50f, 1.00f, 0.15f, 0.60f, 0.55f}},
    {"Half Step Down", "Pedal Suite exclusive", ModuleId::Retune,
     {0.4583f, 0.50f, 0.35f, 1.00f}},
    {"Full Step Down", "Pedal Suite exclusive", ModuleId::Retune,
     {0.4167f, 0.50f, 0.35f, 1.00f}},
    {"Tune Up", "Pedal Suite exclusive", ModuleId::Retune,
     {0.5417f, 0.50f, 0.35f, 1.00f}},
    {"Fine Flat", "Pedal Suite exclusive", ModuleId::Retune,
     {0.50f, 0.30f, 0.35f, 1.00f}},
    {"Wide Chorus", "Pedal Suite exclusive", ModuleId::Chorus,
     {0.30f, 0.55f, 0.85f, 0.55f}},
    {"Slow Shimmer", "Pedal Suite exclusive", ModuleId::Chorus,
     {0.12f, 0.40f, 0.65f, 0.45f}},
    {"Jet", "Pedal Suite exclusive", ModuleId::Flanger,
     {0.28f, 0.60f, 0.78f, 0.55f}},
    {"Slow Sweep", "Pedal Suite exclusive", ModuleId::Flanger,
     {0.10f, 0.45f, 0.40f, 0.50f}},
    {"Classic Phase", "Pedal Suite exclusive", ModuleId::Phaser,
     {0.25f, 0.65f, 0.45f, 0.50f}},
    {"Deep Phase", "Pedal Suite exclusive", ModuleId::Phaser,
     {0.45f, 0.85f, 0.62f, 0.55f}},
    {"Bias Trem", "Pedal Suite exclusive", ModuleId::Tremolo,
     {0.35f, 0.55f, 0.15f, 1.00f}},
    {"Chop", "Pedal Suite exclusive", ModuleId::Tremolo,
     {0.55f, 0.85f, 0.85f, 1.00f}},
};

inline constexpr int kFactoryModulePresetCount =
    static_cast<int>(sizeof(kFactoryModulePresets) /
                     sizeof(kFactoryModulePresets[0]));

} // namespace pedal
