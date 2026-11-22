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
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::Drop)),
    PV(PC(1, 0), 0.0833), PV(PC(1, 1), 0.55), PV(PC(1, 2), 1.0),
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

inline constexpr PresetValue kAutoFunk[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::AutoWah)),
    PV(PC(0, 0), 0.72), PV(PC(0, 1), 0.55), PV(PC(0, 2), 0.60),
    PV(PC(0, 3), 0.88),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::OptoComp)),
    PV(PC(1, 0), 0.50), PV(PC(1, 1), 0.55), PV(PC(1, 2), 1.00),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::Drive808)),
    PV(PC(2, 0), 0.32), PV(PC(2, 1), 0.55), PV(PC(2, 2), 0.55),
    PV(PS(3, kSlotOffsetType), static_cast<int>(ModuleId::Tremolo)),
    PV(PC(3, 0), 0.45), PV(PC(3, 1), 0.35), PV(PC(3, 2), 0.10),
    PV(PC(3, 3), 0.85),
    PV(PS(4, kSlotOffsetType), 0), PV(PS(5, kSlotOffsetType), 0),
    PV(PS(6, kSlotOffsetType), 0), PV(PS(7, kSlotOffsetType), 0),
};

inline constexpr PresetValue kPitchLead[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::Whammy)),
    PV(PC(0, 0), 0.6458), PV(PC(0, 1), 0.15),
    PV(PC(0, 2), 0.55), PV(PC(0, 3), 0.80),
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
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::PrecisionDrive)),
    PV(PC(1, 0), 0.50), PV(PC(1, 1), 0.35), PV(PC(1, 2), 0.80),
    PV(PC(1, 3), 0.50),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::Distortion)),
    PV(PC(2, 0), 0.85), PV(PC(2, 1), 0.55), PV(PC(2, 2), 0.40),
    PV(PS(3, kSlotOffsetType), static_cast<int>(ModuleId::Eq3)),
    PV(PC(3, 0), 0.45), PV(PC(3, 1), 0.35), PV(PC(3, 2), 0.62),
    PV(PC(3, 3), 0.50),
    PV(PS(4, kSlotOffsetType), 0), PV(PS(5, kSlotOffsetType), 0),
    PV(PS(6, kSlotOffsetType), 0), PV(PS(7, kSlotOffsetType), 0),
};

inline constexpr PresetValue kPrecisionMetal[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::TightGate)),
    PV(PC(0, 0), 0.60), PV(PC(0, 1), 0.06), PV(PC(0, 2), 0.95),
    PV(PC(0, 3), 1.00),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::PrecisionDrive)),
    PV(PC(1, 0), 0.55), PV(PC(1, 1), 0.45), PV(PC(1, 2), 0.78),
    PV(PC(1, 3), 0.50),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::Distortion)),
    PV(PC(2, 0), 0.82), PV(PC(2, 1), 0.55), PV(PC(2, 2), 0.42),
    PV(PS(3, kSlotOffsetType), static_cast<int>(ModuleId::Eq3)),
    PV(PC(3, 0), 0.45), PV(PC(3, 1), 0.38), PV(PC(3, 2), 0.62),
    PV(PC(3, 3), 0.50),
    PV(PS(4, kSlotOffsetType), static_cast<int>(ModuleId::NoiseGate)),
    PV(PC(4, 0), 0.55), PV(PC(4, 1), 0.20), PV(PC(4, 2), 0.75),
    PV(PS(5, kSlotOffsetType), 0), PV(PS(6, kSlotOffsetType), 0),
    PV(PS(7, kSlotOffsetType), 0),
};

inline constexpr PresetValue kLaserLead[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::Laser)),
    PV(PC(0, 0), 0.6458), PV(PC(0, 1), 0.35), PV(PC(0, 2), 0.40),
    PV(PC(0, 3), 0.50), PV(PC(0, 4), 0.80),
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

inline constexpr PresetValue kWahRock[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::Wah)),
    PV(PC(0, 0), 0.45), PV(PC(0, 1), 0.65), PV(PC(0, 2), 0.55),
    PV(PC(0, 3), 1.00),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::Drive808)),
    PV(PC(1, 0), 0.40), PV(PC(1, 1), 0.55), PV(PC(1, 2), 0.50),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::AnalogDelay)),
    PV(PC(2, 0), 0.35), PV(PC(2, 1), 0.35), PV(PC(2, 2), 0.50),
    PV(PC(2, 3), 0.25),
    PV(PS(3, kSlotOffsetType), static_cast<int>(ModuleId::RoomReverb)),
    PV(PC(3, 0), 0.30), PV(PC(3, 1), 0.50), PV(PC(3, 2), 0.25),
    PV(PS(4, kSlotOffsetType), 0), PV(PS(5, kSlotOffsetType), 0),
    PV(PS(6, kSlotOffsetType), 0), PV(PS(7, kSlotOffsetType), 0),
};


inline constexpr PresetValue kClassicRock[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::Overdrive)),
    PV(PC(0, 0), 0.55), PV(PC(0, 1), 0.55), PV(PC(0, 2), 0.50),
    PV(PC(0, 3), 0.90),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::TrebleBoost)),
    PV(PC(1, 0), 0.55), PV(PC(1, 1), 0.55), PV(PC(1, 2), 0.45),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::AnalogDelay)),
    PV(PC(2, 0), 0.35), PV(PC(2, 1), 0.30), PV(PC(2, 2), 0.45),
    PV(PC(2, 3), 0.22),
    PV(PS(3, kSlotOffsetType), static_cast<int>(ModuleId::SpringReverb)),
    PV(PC(3, 0), 0.35), PV(PC(3, 1), 0.55), PV(PC(3, 2), 0.22),
    PV(PS(4, kSlotOffsetType), 0), PV(PS(5, kSlotOffsetType), 0),
    PV(PS(6, kSlotOffsetType), 0), PV(PS(7, kSlotOffsetType), 0),
};

inline constexpr PresetValue kBluesbreaker[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::BluesDrive)),
    PV(PC(0, 0), 0.50), PV(PC(0, 1), 0.55), PV(PC(0, 2), 0.50),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::Boost)),
    PV(PC(1, 0), 0.15), PV(PC(1, 1), 0.55), PV(PC(1, 2), 0.55),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::SpringReverb)),
    PV(PC(2, 0), 0.30), PV(PC(2, 1), 0.60), PV(PC(2, 2), 0.25),
    PV(PS(3, kSlotOffsetType), 0), PV(PS(4, kSlotOffsetType), 0),
    PV(PS(5, kSlotOffsetType), 0), PV(PS(6, kSlotOffsetType), 0),
    PV(PS(7, kSlotOffsetType), 0),
};

inline constexpr PresetValue kCountryTwang[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::TrebleBoost)),
    PV(PC(0, 0), 0.62), PV(PC(0, 1), 0.65), PV(PC(0, 2), 0.50),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::OptoComp)),
    PV(PC(1, 0), 0.45), PV(PC(1, 1), 0.55), PV(PC(1, 2), 0.72),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::AnalogDelay)),
    PV(PC(2, 0), 0.18), PV(PC(2, 1), 0.20), PV(PC(2, 2), 0.58),
    PV(PC(2, 3), 0.18),
    PV(PS(3, kSlotOffsetType), static_cast<int>(ModuleId::SpringReverb)),
    PV(PC(3, 0), 0.28), PV(PC(3, 1), 0.60), PV(PC(3, 2), 0.30),
    PV(PS(4, kSlotOffsetType), 0), PV(PS(5, kSlotOffsetType), 0),
    PV(PS(6, kSlotOffsetType), 0), PV(PS(7, kSlotOffsetType), 0),
};

inline constexpr PresetValue kShoegaze[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::BigFuzz)),
    PV(PC(0, 0), 0.55), PV(PC(0, 1), 0.45), PV(PC(0, 2), 0.45),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::Chorus)),
    PV(PC(1, 0), 0.28), PV(PC(1, 1), 0.60), PV(PC(1, 2), 0.88),
    PV(PC(1, 3), 0.55),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::TapeEcho)),
    PV(PC(2, 0), 0.55), PV(PC(2, 1), 0.55), PV(PC(2, 2), 0.30),
    PV(PC(2, 3), 0.38), PV(PC(2, 4), 0.38),
    PV(PS(3, kSlotOffsetType), static_cast<int>(ModuleId::PlateReverb)),
    PV(PC(3, 0), 0.80), PV(PC(3, 1), 0.60), PV(PC(3, 2), 0.45),
    PV(PS(4, kSlotOffsetType), 0), PV(PS(5, kSlotOffsetType), 0),
    PV(PS(6, kSlotOffsetType), 0), PV(PS(7, kSlotOffsetType), 0),
};

inline constexpr PresetValue kFunkMachine[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::EnvFilter)),
    PV(PC(0, 0), 0.45), PV(PC(0, 1), 0.55), PV(PC(0, 2), 0.50),
    PV(PC(0, 3), 0.55),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::OptoComp)),
    PV(PC(1, 0), 0.55), PV(PC(1, 1), 0.50), PV(PC(1, 2), 0.78),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::Boost)),
    PV(PC(2, 0), 0.20), PV(PC(2, 1), 0.55), PV(PC(2, 2), 0.50),
    PV(PS(3, kSlotOffsetType), 0), PV(PS(4, kSlotOffsetType), 0),
    PV(PS(5, kSlotOffsetType), 0), PV(PS(6, kSlotOffsetType), 0),
    PV(PS(7, kSlotOffsetType), 0),
};

inline constexpr PresetValue kBassDI[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::BassEq)),
    PV(PC(0, 0), 0.58), PV(PC(0, 1), 0.50), PV(PC(0, 2), 0.55),
    PV(PC(0, 3), 0.50),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::OptoComp)),
    PV(PC(1, 0), 0.50), PV(PC(1, 1), 0.50), PV(PC(1, 2), 0.85),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::Cab)),
    PV(PC(2, 0), 0.30), PV(PC(2, 1), 0.80), PV(PC(2, 2), 0.50),
    PV(PC(2, 3), 0.40),
    PV(PS(3, kSlotOffsetType), 0), PV(PS(4, kSlotOffsetType), 0),
    PV(PS(5, kSlotOffsetType), 0), PV(PS(6, kSlotOffsetType), 0),
    PV(PS(7, kSlotOffsetType), 0),
};

inline constexpr PresetValue kSynthBass[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::Octave)),
    PV(PC(0, 0), 0.55), PV(PC(0, 1), 0.35), PV(PC(0, 2), 0.50),
    PV(PC(0, 3), 0.45),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::BassFuzz)),
    PV(PC(1, 0), 0.50), PV(PC(1, 1), 0.60), PV(PC(1, 2), 0.45),
    PV(PC(1, 3), 0.45),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::BassChorus)),
    PV(PC(2, 0), 0.25), PV(PC(2, 1), 0.55), PV(PC(2, 2), 0.30),
    PV(PC(2, 3), 0.35),
    PV(PS(3, kSlotOffsetType), static_cast<int>(ModuleId::BassEq)),
    PV(PC(3, 0), 0.55), PV(PC(3, 1), 0.50), PV(PC(3, 2), 0.55),
    PV(PC(3, 3), 0.50),
    PV(PS(4, kSlotOffsetType), 0), PV(PS(5, kSlotOffsetType), 0),
    PV(PS(6, kSlotOffsetType), 0), PV(PS(7, kSlotOffsetType), 0),
};

inline constexpr PresetValue kProgLead[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::Whammy)),
    PV(PC(0, 0), 0.75), PV(PC(0, 1), 0.10),
    PV(PC(0, 2), 0.55), PV(PC(0, 3), 0.45),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::Overdrive)),
    PV(PC(1, 0), 0.55), PV(PC(1, 1), 0.60), PV(PC(1, 2), 0.50),
    PV(PC(1, 3), 0.85),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::DigitalDelay)),
    PV(PC(2, 0), 0.42), PV(PC(2, 1), 0.45), PV(PC(2, 2), 0.55),
    PV(PC(2, 3), 0.30),
    PV(PS(3, kSlotOffsetType), static_cast<int>(ModuleId::PlateReverb)),
    PV(PC(3, 0), 0.55), PV(PC(3, 1), 0.65), PV(PC(3, 2), 0.30),
    PV(PS(4, kSlotOffsetType), 0), PV(PS(5, kSlotOffsetType), 0),
    PV(PS(6, kSlotOffsetType), 0), PV(PS(7, kSlotOffsetType), 0),
};

inline constexpr PresetValue kWorship[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::Boost)),
    PV(PC(0, 0), 0.15), PV(PC(0, 1), 0.50), PV(PC(0, 2), 0.55),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::Chorus)),
    PV(PC(1, 0), 0.22), PV(PC(1, 1), 0.55), PV(PC(1, 2), 0.80),
    PV(PC(1, 3), 0.50),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::TapeEcho)),
    PV(PC(2, 0), 0.50), PV(PC(2, 1), 0.50), PV(PC(2, 2), 0.35),
    PV(PC(2, 3), 0.35), PV(PC(2, 4), 0.35),
    PV(PS(3, kSlotOffsetType), static_cast<int>(ModuleId::PlateReverb)),
    PV(PC(3, 0), 0.78), PV(PC(3, 1), 0.60), PV(PC(3, 2), 0.45),
    PV(PS(4, kSlotOffsetType), 0), PV(PS(5, kSlotOffsetType), 0),
    PV(PS(6, kSlotOffsetType), 0), PV(PS(7, kSlotOffsetType), 0),
};

inline constexpr PresetValue kGarage[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::FuzzFace)),
    PV(PC(0, 0), 0.65), PV(PC(0, 1), 0.50), PV(PC(0, 2), 0.55),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::Tremolo)),
    PV(PC(1, 0), 0.45), PV(PC(1, 1), 0.60), PV(PC(1, 2), 0.20),
    PV(PC(1, 3), 1.00),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::SpringReverb)),
    PV(PC(2, 0), 0.40), PV(PC(2, 1), 0.55), PV(PC(2, 2), 0.30),
    PV(PS(3, kSlotOffsetType), 0), PV(PS(4, kSlotOffsetType), 0),
    PV(PS(5, kSlotOffsetType), 0), PV(PS(6, kSlotOffsetType), 0),
    PV(PS(7, kSlotOffsetType), 0),
};

inline constexpr PresetValue kDoom[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::BigFuzz)),
    PV(PC(0, 0), 0.85), PV(PC(0, 1), 0.40), PV(PC(0, 2), 0.45),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::Rat)),
    PV(PC(1, 0), 0.45), PV(PC(1, 1), 0.35), PV(PC(1, 2), 0.50),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::Eq3)),
    PV(PC(2, 0), 0.65), PV(PC(2, 1), 0.45), PV(PC(2, 2), 0.45),
    PV(PC(2, 3), 0.50),
    PV(PS(3, kSlotOffsetType), static_cast<int>(ModuleId::RoomReverb)),
    PV(PC(3, 0), 0.55), PV(PC(3, 1), 0.45), PV(PC(3, 2), 0.30),
    PV(PS(4, kSlotOffsetType), 0), PV(PS(5, kSlotOffsetType), 0),
    PV(PS(6, kSlotOffsetType), 0), PV(PS(7, kSlotOffsetType), 0),
};

inline constexpr PresetValue kAcousticDI[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::Eq3)),
    PV(PC(0, 0), 0.52), PV(PC(0, 1), 0.50), PV(PC(0, 2), 0.55),
    PV(PC(0, 3), 0.50),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::OptoComp)),
    PV(PC(1, 0), 0.40), PV(PC(1, 1), 0.45), PV(PC(1, 2), 0.70),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::RoomReverb)),
    PV(PC(2, 0), 0.45), PV(PC(2, 1), 0.55), PV(PC(2, 2), 0.25),
    PV(PS(3, kSlotOffsetType), 0), PV(PS(4, kSlotOffsetType), 0),
    PV(PS(5, kSlotOffsetType), 0), PV(PS(6, kSlotOffsetType), 0),
    PV(PS(7, kSlotOffsetType), 0),
};

#undef PC
#undef PS
#undef PV


// converted from the user preset "Clean Boost"
inline constexpr PresetValue kUserCleanBoost[] = {
    {0, 1}, {1, 0}, {2, 0}, {3, 1}, {4, 0}, {5, 100}, {6, 0}, {7, 0}, {8, 0.5}, {9, 0.61},
    {10, 0.5}, {11, 0.5}, {12, 0.5}, {13, 0.5}, {14, 0.5}, {15, 0.5}, {16, 0.5}, {17, 0.5},
    {18, 0.5}, {19, 0}, {20, 0}, {21, 100}, {22, 0}, {23, 0.5}, {24, 0.5}, {25, 0.5},
    {26, 0.5}, {27, 0.5}, {28, 0.5}, {29, 0.5}, {30, 0.5}, {31, 0.5}, {32, 0.5}, {33, 0.5},
    {34, 0.5}, {35, 0}, {36, 0}, {37, 100}, {38, 0}, {39, 0.5}, {40, 0.5}, {41, 0.5},
    {42, 0.5}, {43, 0.5}, {44, 0.5}, {45, 0.5}, {46, 0.5}, {47, 0.5}, {48, 0.5}, {49, 0.5},
    {50, 0.5}, {51, 0}, {52, 0}, {53, 100}, {54, 0}, {55, 0.5}, {56, 0.5}, {57, 0.5},
    {58, 0.5}, {59, 0.5}, {60, 0.5}, {61, 0.5}, {62, 0.5}, {63, 0.5}, {64, 0.5}, {65, 0.5},
    {66, 0.5}, {67, 0}, {68, 0}, {69, 100}, {70, 0}, {71, 0.5}, {72, 0.5}, {73, 0.5},
    {74, 0.5}, {75, 0.5}, {76, 0.5}, {77, 0.5}, {78, 0.5}, {79, 0.5}, {80, 0.5}, {81, 0.5},
    {82, 0.5}, {83, 0}, {84, 0}, {85, 100}, {86, 0}, {87, 0.5}, {88, 0.5}, {89, 0.5},
    {90, 0.5}, {91, 0.5}, {92, 0.5}, {93, 0.5}, {94, 0.5}, {95, 0.5}, {96, 0.5}, {97, 0.5},
    {98, 0.5}, {99, 0}, {100, 0}, {101, 100}, {102, 0}, {103, 0.5}, {104, 0.5}, {105, 0.5},
    {106, 0.5}, {107, 0.5}, {108, 0.5}, {109, 0.5}, {110, 0.5}, {111, 0.5}, {112, 0.5},
    {113, 0.5}, {114, 0.5}, {115, 0}, {116, 0}, {117, 100}, {118, 0}, {119, 0.5}, {120, 0.5},
    {121, 0.5}, {122, 0.5}, {123, 0.5}, {124, 0.5}, {125, 0.5}, {126, 0.5}, {127, 0.5},
    {128, 0.5}, {129, 0.5}, {130, 0.5}, {131, 0}, {132, 0}, {133, 36}, {134, 37}, {135, 38},
    {136, 39}, {137, 40}, {138, 41}, {139, 42}, {140, 43}, {141, 20}, {142, 21}, {143, 22},
    {144, 23}, {145, 24}, {146, 25}, {147, 26}, {148, 27}, {149, 0}, {150, 0}, {151, 0},
    {152, 4}, {153, 0}
};

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
    {"Funk", "Auto Funk", kAutoFunk,
     static_cast<int>(sizeof(kAutoFunk) / sizeof(kAutoFunk[0]))},
    {"Lead", "Pitch Lead", kPitchLead,
     static_cast<int>(sizeof(kPitchLead) / sizeof(kPitchLead[0]))},
    {"Surf", "Spring Surf", kSpringSurf,
     static_cast<int>(sizeof(kSpringSurf) / sizeof(kSpringSurf[0]))},
    {"Lo-Fi", "Lo-Fi Tape", kLofiTape,
     static_cast<int>(sizeof(kLofiTape) / sizeof(kLofiTape[0]))},
    {"Metal", "Modern Metal", kModernMetal,
     static_cast<int>(sizeof(kModernMetal) / sizeof(kModernMetal[0]))},
    {"Metal", "Precision Metal", kPrecisionMetal,
     static_cast<int>(sizeof(kPrecisionMetal) / sizeof(kPrecisionMetal[0]))},
    {"Lead", "Laser Lead", kLaserLead,
     static_cast<int>(sizeof(kLaserLead) / sizeof(kLaserLead[0]))},
    {"Rock", "Wah Rock", kWahRock,
     static_cast<int>(sizeof(kWahRock) / sizeof(kWahRock[0]))},
    {"Rock", "Classic Rock", kClassicRock,
     static_cast<int>(sizeof(kClassicRock) / sizeof(kClassicRock[0]))},
    {"Blues", "Bluesbreaker", kBluesbreaker,
     static_cast<int>(sizeof(kBluesbreaker) / sizeof(kBluesbreaker[0]))},
    {"Country", "Country Twang", kCountryTwang,
     static_cast<int>(sizeof(kCountryTwang) / sizeof(kCountryTwang[0]))},
    {"Shoegaze", "Shoegaze", kShoegaze,
     static_cast<int>(sizeof(kShoegaze) / sizeof(kShoegaze[0]))},
    {"Funk", "Funk Machine", kFunkMachine,
     static_cast<int>(sizeof(kFunkMachine) / sizeof(kFunkMachine[0]))},
    {"Bass", "Bass DI", kBassDI,
     static_cast<int>(sizeof(kBassDI) / sizeof(kBassDI[0]))},
    {"Bass", "Synth Bass", kSynthBass,
     static_cast<int>(sizeof(kSynthBass) / sizeof(kSynthBass[0]))},
    {"Prog", "Prog Lead", kProgLead,
     static_cast<int>(sizeof(kProgLead) / sizeof(kProgLead[0]))},
    {"Worship", "Worship", kWorship,
     static_cast<int>(sizeof(kWorship) / sizeof(kWorship[0]))},
    {"Garage", "Garage", kGarage,
     static_cast<int>(sizeof(kGarage) / sizeof(kGarage[0]))},
    {"Doom", "Doom", kDoom,
     static_cast<int>(sizeof(kDoom) / sizeof(kDoom[0]))},
    {"Acoustic", "Acoustic DI", kAcousticDI,
     static_cast<int>(sizeof(kAcousticDI) / sizeof(kAcousticDI[0]))},
    {"User", "Clean Boost", kUserCleanBoost, 154},
};

inline constexpr int kPresetCount =
    static_cast<int>(sizeof(kPresets) / sizeof(kPresets[0]));

} // namespace pedal
