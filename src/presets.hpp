// Pedal Suite — factory board presets
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <cstddef>

#include "module_ids.hpp"
#include "params.hpp"

namespace pedal {

struct PresetValue { int param; double value; };
struct Preset {
  const char* category;
  const char* name;
  const PresetValue* values;
  int count;
};

#define PV(p, v) {static_cast<int>(p), (v)}
#define PS(slot, offset) slotParam((slot), (offset))
#define PC(slot, control) slotParam((slot), kSlotOffsetC0 + (control))

inline constexpr PresetValue kInit[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::Boost)),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::Drive808)),
    PV(PS(2, kSlotOffsetType), 0), PV(PS(3, kSlotOffsetType), 0),
    PV(PS(4, kSlotOffsetType), 0), PV(PS(5, kSlotOffsetType), 0),
    PV(PS(6, kSlotOffsetType), 0), PV(PS(7, kSlotOffsetType), 0),
    PV(PC(0, 0), 0.00), PV(PC(0, 1), 0.50), PV(PC(0, 2), 0.50),
    PV(PC(1, 0), 0.28), PV(PC(1, 1), 0.50), PV(PC(1, 2), 0.55),
};

inline constexpr PresetValue kCleanPlatform[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::Boost)),
    PV(PC(0, 0), 0.10), PV(PC(0, 1), 0.55), PV(PC(0, 2), 0.55),
    PV(PS(1, kSlotOffsetType), 0), PV(PS(2, kSlotOffsetType), 0),
    PV(PS(3, kSlotOffsetType), 0), PV(PS(4, kSlotOffsetType), 0),
    PV(PS(5, kSlotOffsetType), 0), PV(PS(6, kSlotOffsetType), 0),
    PV(PS(7, kSlotOffsetType), 0),
};

inline constexpr PresetValue k808Stack[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::Drive808)),
    PV(PC(0, 0), 0.42), PV(PC(0, 1), 0.45), PV(PC(0, 2), 0.55),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::Boost)),
    PV(PC(1, 0), 0.30), PV(PC(1, 1), 0.60), PV(PC(1, 2), 0.45),
    PV(PS(2, kSlotOffsetType), 0), PV(PS(3, kSlotOffsetType), 0),
    PV(PS(4, kSlotOffsetType), 0), PV(PS(5, kSlotOffsetType), 0),
    PV(PS(6, kSlotOffsetType), 0), PV(PS(7, kSlotOffsetType), 0),
};

inline constexpr PresetValue kLeadLift[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::Boost)),
    PV(PC(0, 0), 0.35), PV(PC(0, 1), 0.65), PV(PC(0, 2), 0.55),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::Drive808)),
    PV(PC(1, 0), 0.65), PV(PC(1, 1), 0.55), PV(PC(1, 2), 0.50),
    PV(PS(2, kSlotOffsetType), 0), PV(PS(3, kSlotOffsetType), 0),
    PV(PS(4, kSlotOffsetType), 0), PV(PS(5, kSlotOffsetType), 0),
    PV(PS(6, kSlotOffsetType), 0), PV(PS(7, kSlotOffsetType), 0),
};

#undef PC
#undef PS
#undef PV

inline constexpr Preset kPresets[] = {
    {"Init", "Init", kInit, static_cast<int>(sizeof(kInit) / sizeof(kInit[0]))},
    {"Clean", "Clean Platform", kCleanPlatform,
     static_cast<int>(sizeof(kCleanPlatform) / sizeof(kCleanPlatform[0]))},
    {"Drive", "808 Stack", k808Stack,
     static_cast<int>(sizeof(k808Stack) / sizeof(k808Stack[0]))},
    {"Lead", "Lead Lift", kLeadLift,
     static_cast<int>(sizeof(kLeadLift) / sizeof(kLeadLift[0]))},
};

inline constexpr int kPresetCount =
    static_cast<int>(sizeof(kPresets) / sizeof(kPresets[0]));

} // namespace pedal
