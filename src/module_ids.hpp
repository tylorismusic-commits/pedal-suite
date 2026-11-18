// Pedal Suite — module identifiers
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <algorithm>

namespace pedal {

enum ModuleId : int {
  Off = 0,
  Boost,
  Drive808,
  Count
};

constexpr int kModuleCount = static_cast<int>(ModuleId::Count);

inline constexpr const char* kModuleLabels[] = {"Off", "Boost", "808 Drive"};

static_assert(sizeof(kModuleLabels) / sizeof(kModuleLabels[0]) == kModuleCount,
              "kModuleLabels must cover every module id");

constexpr int kRackControlCount = 12;

// Shared control indices used by the first pedal controls.
constexpr int kCtlGain = 0;
constexpr int kCtlTone = 1;
constexpr int kCtlLevel = 2;
constexpr int kCtlDrive = 0;
constexpr int kCtlMix = 2;

inline constexpr bool validModule(int id) noexcept {
  return id >= 0 && id < kModuleCount;
}

inline constexpr bool moduleIsEq(int) noexcept { return false; }
inline constexpr bool moduleIsComp(int) noexcept { return false; }
inline constexpr bool moduleHasGr(int) noexcept { return false; }

// All controls are normalised to 0..1.
constexpr double defaultControlValue(ModuleId id, int control) noexcept {
  switch (id) {
    case ModuleId::Boost:
      switch (control) {
        case 0: return 0.00;  // Gain
        case 1: return 0.50;  // Tone
        case 2: return 0.50;  // Level
        default: return 0.50;
      }
    case ModuleId::Drive808:
      switch (control) {
        case 0: return 0.28;  // Drive
        case 1: return 0.50;  // Tone
        case 2: return 0.55;  // Level
        default: return 0.50;
      }
    default:
      return 0.50;
  }
}

} // namespace pedal
