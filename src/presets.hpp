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


// --- Curated boards ---------------------------------------------------------

inline constexpr PresetValue kAmbientWash[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::Chorus)),
    PV(PC(0, 0), 0.25), PV(PC(0, 1), 0.60), PV(PC(0, 2), 0.90),
    PV(PC(0, 3), 0.50),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::TapeEcho)),
    PV(PC(1, 0), 0.55), PV(PC(1, 1), 0.55), PV(PC(1, 2), 0.35),
    PV(PC(1, 3), 0.35), PV(PC(1, 4), 0.35),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::PlateReverb)),
    PV(PC(2, 0), 0.75), PV(PC(2, 1), 0.65), PV(PC(2, 2), 0.40),
    PV(PS(3, kSlotOffsetType), 0), PV(PS(4, kSlotOffsetType), 0),
    PV(PS(5, kSlotOffsetType), 0), PV(PS(6, kSlotOffsetType), 0),
    PV(PS(7, kSlotOffsetType), 0),
};

inline constexpr PresetValue kGojiraDrop[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::TightGate)),
    PV(PC(0, 0), 0.60), PV(PC(0, 1), 0.08), PV(PC(0, 2), 0.90),
    PV(PC(0, 3), 1.00),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::PitchDrop)),
    PV(PC(1, 0), 0.0833), PV(PC(1, 1), 0.50), PV(PC(1, 2), 0.02),
    PV(PC(1, 3), 0.55), PV(PC(1, 4), 1.0),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::Distortion)),
    PV(PC(2, 0), 0.75), PV(PC(2, 1), 0.50), PV(PC(2, 2), 0.45),
    PV(PS(3, kSlotOffsetType), static_cast<int>(ModuleId::Eq3)),
    PV(PC(3, 0), 0.55), PV(PC(3, 1), 0.40), PV(PC(3, 2), 0.60),
    PV(PC(3, 3), 0.50),
    PV(PS(4, kSlotOffsetType), static_cast<int>(ModuleId::NoiseGate)),
    PV(PC(4, 0), 0.55), PV(PC(4, 1), 0.20), PV(PC(4, 2), 0.75),
    PV(PS(5, kSlotOffsetType), 0), PV(PS(6, kSlotOffsetType), 0),
    PV(PS(7, kSlotOffsetType), 0),
};

inline constexpr PresetValue kBassGrowl[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::BassDrive)),
    PV(PC(0, 0), 0.45), PV(PC(0, 1), 0.80), PV(PC(0, 2), 0.50),
    PV(PC(0, 3), 0.50),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::BassEq)),
    PV(PC(1, 0), 0.62), PV(PC(1, 1), 0.48), PV(PC(1, 2), 0.55),
    PV(PC(1, 3), 0.50),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::OptoComp)),
    PV(PC(2, 0), 0.55), PV(PC(2, 1), 0.55), PV(PC(2, 2), 1.00),
    PV(PS(3, kSlotOffsetType), static_cast<int>(ModuleId::BassChorus)),
    PV(PC(3, 0), 0.20), PV(PC(3, 1), 0.40), PV(PC(3, 2), 0.35),
    PV(PC(3, 3), 0.35),
    PV(PS(4, kSlotOffsetType), 0), PV(PS(5, kSlotOffsetType), 0),
    PV(PS(6, kSlotOffsetType), 0), PV(PS(7, kSlotOffsetType), 0),
};

inline constexpr PresetValue kFunkWah[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::EnvFilter)),
    PV(PC(0, 0), 0.65), PV(PC(0, 1), 0.55), PV(PC(0, 2), 0.65),
    PV(PC(0, 3), 0.80),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::OptoComp)),
    PV(PC(1, 0), 0.50), PV(PC(1, 1), 0.55), PV(PC(1, 2), 1.00),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::Drive808)),
    PV(PC(2, 0), 0.35), PV(PC(2, 1), 0.55), PV(PC(2, 2), 0.55),
    PV(PS(3, kSlotOffsetType), static_cast<int>(ModuleId::Tremolo)),
    PV(PC(3, 0), 0.45), PV(PC(3, 1), 0.40), PV(PC(3, 2), 0.10),
    PV(PC(3, 3), 0.90),
    PV(PS(4, kSlotOffsetType), 0), PV(PS(5, kSlotOffsetType), 0),
    PV(PS(6, kSlotOffsetType), 0), PV(PS(7, kSlotOffsetType), 0),
};

inline constexpr PresetValue kPitchLead[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::PitchDrop)),
    PV(PC(0, 0), 0.2917), PV(PC(0, 1), 0.00), PV(PC(0, 2), 0.15),
    PV(PC(0, 3), 0.55), PV(PC(0, 4), 0.80),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::Overdrive)),
    PV(PC(1, 0), 0.50), PV(PC(1, 1), 0.55), PV(PC(1, 2), 0.50),
    PV(PC(1, 3), 1.00),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::AnalogDelay)),
    PV(PC(2, 0), 0.40), PV(PC(2, 1), 0.45), PV(PC(2, 2), 0.45),
    PV(PC(2, 3), 0.30),
    PV(PS(3, kSlotOffsetType), static_cast<int>(ModuleId::PlateReverb)),
    PV(PC(3, 0), 0.45), PV(PC(3, 1), 0.60), PV(PC(3, 2), 0.28),
    PV(PS(4, kSlotOffsetType), 0), PV(PS(5, kSlotOffsetType), 0),
    PV(PS(6, kSlotOffsetType), 0), PV(PS(7, kSlotOffsetType), 0),
};

inline constexpr PresetValue kSpringSurf[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::TrebleBoost)),
    PV(PC(0, 0), 0.55), PV(PC(0, 1), 0.50), PV(PC(0, 2), 0.50),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::Tremolo)),
    PV(PC(1, 0), 0.55), PV(PC(1, 1), 0.50), PV(PC(1, 2), 0.25),
    PV(PC(1, 3), 1.00),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::SpringReverb)),
    PV(PC(2, 0), 0.55), PV(PC(2, 1), 0.45), PV(PC(2, 2), 0.42),
    PV(PS(3, kSlotOffsetType), 0), PV(PS(4, kSlotOffsetType), 0),
    PV(PS(5, kSlotOffsetType), 0), PV(PS(6, kSlotOffsetType), 0),
    PV(PS(7, kSlotOffsetType), 0),
};

inline constexpr PresetValue kLofiTape[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::LoFi)),
    PV(PC(0, 0), 0.35), PV(PC(0, 1), 0.55), PV(PC(0, 2), 0.45),
    PV(PC(0, 3), 1.00),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::TapeSat)),
    PV(PC(1, 0), 0.50), PV(PC(1, 1), 0.40), PV(PC(1, 2), 0.70),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::TapeEcho)),
    PV(PC(2, 0), 0.50), PV(PC(2, 1), 0.50), PV(PC(2, 2), 0.60),
    PV(PC(2, 3), 0.30), PV(PC(2, 4), 0.30),
    PV(PS(3, kSlotOffsetType), static_cast<int>(ModuleId::RoomReverb)),
    PV(PC(3, 0), 0.30), PV(PC(3, 1), 0.45), PV(PC(3, 2), 0.25),
    PV(PS(4, kSlotOffsetType), 0), PV(PS(5, kSlotOffsetType), 0),
    PV(PS(6, kSlotOffsetType), 0), PV(PS(7, kSlotOffsetType), 0),
};

inline constexpr PresetValue kModernMetal[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::TightGate)),
    PV(PC(0, 0), 0.58), PV(PC(0, 1), 0.06), PV(PC(0, 2), 0.95),
    PV(PC(0, 3), 1.00),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::Drive808)),
    PV(PC(1, 0), 0.30), PV(PC(1, 1), 0.50), PV(PC(1, 2), 0.50),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::Distortion)),
    PV(PC(2, 0), 0.85), PV(PC(2, 1), 0.55), PV(PC(2, 2), 0.40),
    PV(PS(3, kSlotOffsetType), static_cast<int>(ModuleId::Eq3)),
    PV(PC(3, 0), 0.45), PV(PC(3, 1), 0.35), PV(PC(3, 2), 0.62),
    PV(PC(3, 3), 0.50),
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
    {"Ambient", "Ambient Wash", kAmbientWash,
     static_cast<int>(sizeof(kAmbientWash) / sizeof(kAmbientWash[0]))},
    {"Metal", "Gojira Drop", kGojiraDrop,
     static_cast<int>(sizeof(kGojiraDrop) / sizeof(kGojiraDrop[0]))},
    {"Bass", "Bass Growl", kBassGrowl,
     static_cast<int>(sizeof(kBassGrowl) / sizeof(kBassGrowl[0]))},
    {"Funk", "Funk Wah", kFunkWah,
     static_cast<int>(sizeof(kFunkWah) / sizeof(kFunkWah[0]))},
    {"Lead", "Pitch Lead", kPitchLead,
     static_cast<int>(sizeof(kPitchLead) / sizeof(kPitchLead[0]))},
    {"Surf", "Spring Surf", kSpringSurf,
     static_cast<int>(sizeof(kSpringSurf) / sizeof(kSpringSurf[0]))},
    {"Lo-Fi", "Lo-Fi Tape", kLofiTape,
     static_cast<int>(sizeof(kLofiTape) / sizeof(kLofiTape[0]))},
    {"Metal", "Modern Metal", kModernMetal,
     static_cast<int>(sizeof(kModernMetal) / sizeof(kModernMetal[0]))},
};

inline constexpr int kPresetCount =
    static_cast<int>(sizeof(kPresets) / sizeof(kPresets[0]));

} // namespace pedal
