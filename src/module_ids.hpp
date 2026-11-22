// Pedal Suite — module identifiers
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <algorithm>

namespace pedal {

enum ModuleId : int {
  Off = 0,
  Boost,
  Drive808,
  TrebleBoost,
  Overdrive,
  BluesDrive,
  Distortion,
  Rat,
  BigFuzz,
  FuzzFace,
  ToneBender,
  BassDrive,
  BassFuzz,
  Whammy,
  Retune,
  Chorus,
  Flanger,
  Phaser,
  Tremolo,
  AnalogDelay,
  SpringReverb,
  BassChorus,
  Vibrato,
  Octave,
  TapeEcho,
  DigitalDelay,
  PingPongDelay,
  PlateReverb,
  RoomReverb,
  FetComp,
  OptoComp,
  EnvFilter,
  NoiseGate,
  TightGate,
  Eq3,
  BassEq,
  LoFi,
  TapeSat,
  PrecisionDrive,
  Wah,
  Laser,
  Cab,
  Nam,
  AutoWah,
  Drop,
  Count
};

constexpr int kModuleCount = static_cast<int>(ModuleId::Count);

inline constexpr const char* kModuleLabels[] = {
    "Off",       "Boost",     "808 Drive",  "Treble Boost", "Overdrive",
    "Blues Drive", "Distortion", "Rat",     "Big Fuzz",   "Fuzz Face",
    "Tone Bender", "Bass Drive", "Bass Fuzz", "Whammy", "Retune",
    "Chorus", "Flanger", "Phaser", "Tremolo", "Analog Delay",
    "Spring Reverb", "Bass Chorus", "Vibrato", "Octave", "Tape Echo",
    "Digital Delay", "Ping-Pong", "Plate Reverb", "Room Reverb",
    "FET Comp", "Opto Comp", "Envelope Filter", "Noise Gate", "Tight Gate",
    "3-Band EQ", "Bass EQ", "Lo-Fi", "Tape Sat", "Precision Drive", "Wah",
    "Laser", "Cab", "NAM Amp", "Auto-Wah", "Drop"};

static_assert(sizeof(kModuleLabels) / sizeof(kModuleLabels[0]) == kModuleCount,
              "kModuleLabels must cover every module id");

constexpr int kRackControlCount = 12;

// NAM models are expensive (memory and CPU). Keep at most two live in a
// session; further NAM slots pass dry and show a limit warning.
inline constexpr int kMaxNamSlots = 2;
inline constexpr int kMaxCabSlots = 2;

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
inline constexpr bool moduleHasTails(int type) noexcept {
  switch (static_cast<ModuleId>(type)) {
    case ModuleId::AnalogDelay:
    case ModuleId::TapeEcho:
    case ModuleId::DigitalDelay:
    case ModuleId::PingPongDelay:
    case ModuleId::SpringReverb:
    case ModuleId::PlateReverb:
    case ModuleId::RoomReverb:
      return true;
    default:
      return false;
  }
}

inline constexpr bool moduleHasGr(int type) noexcept {
  switch (static_cast<ModuleId>(type)) {
    case ModuleId::FetComp:
    case ModuleId::OptoComp:
    case ModuleId::NoiseGate:
    case ModuleId::TightGate:
    case ModuleId::PrecisionDrive:
      return true;
    default:
      return false;
  }
}

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
    case ModuleId::TrebleBoost:
      switch (control) {
        case 0: return 0.50;  // Boost
        case 1: return 0.50;  // Range
        case 2: return 0.50;  // Level
        default: return 0.50;
      }
    case ModuleId::Overdrive:
      switch (control) {
        case 0: return 0.40;  // Drive
        case 1: return 0.50;  // Tone
        case 2: return 0.50;  // Level
        case 3: return 1.00;  // Mix
        default: return 0.50;
      }
    case ModuleId::BluesDrive:
      switch (control) {
        case 0: return 0.35;  // Drive
        case 1: return 0.50;  // Tone
        case 2: return 0.50;  // Level
        default: return 0.50;
      }
    case ModuleId::Distortion:
      switch (control) {
        case 0: return 0.50;  // Dist
        case 1: return 0.50;  // Filter
        case 2: return 0.50;  // Level
        default: return 0.50;
      }
    case ModuleId::Rat:
      switch (control) {
        case 0: return 0.50;  // Dist
        case 1: return 0.50;  // Filter
        case 2: return 0.50;  // Level
        default: return 0.50;
      }
    case ModuleId::BigFuzz:
      switch (control) {
        case 0: return 0.50;  // Sustain
        case 1: return 0.50;  // Tone
        case 2: return 0.50;  // Level
        default: return 0.50;
      }
    case ModuleId::FuzzFace:
      switch (control) {
        case 0: return 0.60;  // Fuzz
        case 1: return 0.50;  // Bias
        case 2: return 0.50;  // Volume
        default: return 0.50;
      }
    case ModuleId::ToneBender:
      switch (control) {
        case 0: return 0.60;  // Attack
        case 1: return 0.50;  // Tone
        case 2: return 0.50;  // Level
        default: return 0.50;
      }
    case ModuleId::BassDrive:
      switch (control) {
        case 0: return 0.45;  // Drive
        case 1: return 0.80;  // Blend
        case 2: return 0.50;  // Tone
        case 3: return 0.50;  // Level
        default: return 0.50;
      }
    case ModuleId::BassFuzz:
      switch (control) {
        case 0: return 0.60;  // Fuzz
        case 1: return 0.80;  // Blend
        case 2: return 0.50;  // Tone
        case 3: return 0.50;  // Level
        default: return 0.50;
      }
    case ModuleId::Whammy:
      switch (control) {
        case 0: return 0.50;  // Pitch (unison)
        case 1: return 0.20;  // Glide
        case 2: return 0.60;  // Tone
        case 3: return 1.00;  // Mix
        default: return 0.50;
      }
    case ModuleId::Drop:
      switch (control) {
        case 0: return 0.00;  // Drop (unison)
        case 1: return 0.50;  // Tone
        case 2: return 1.00;  // Mix
        default: return 0.50;
      }
    case ModuleId::Retune:
      switch (control) {
        case 0: return 0.50;  // 0 semitones
        case 1: return 0.50;  // 0 cents
        case 2: return 0.50;  // 170 Hz low cut
        case 3: return 1.00;  // Mix
        default: return 0.50;
      }
    case ModuleId::Chorus:
      switch (control) {
        case 0: return 0.35;  // Rate
        case 1: return 0.50;  // Depth
        case 2: return 0.60;  // Width
        case 3: return 0.50;  // Mix
        default: return 0.50;
      }
    case ModuleId::Flanger:
      switch (control) {
        case 0: return 0.30;  // Rate
        case 1: return 0.55;  // Depth
        case 2: return 0.60;  // Feedback
        case 3: return 0.55;  // Mix
        default: return 0.50;
      }
    case ModuleId::Phaser:
      switch (control) {
        case 0: return 0.30;  // Rate
        case 1: return 0.60;  // Depth
        case 2: return 0.50;  // Feedback
        case 3: return 0.50;  // Mix
        default: return 0.50;
      }
    case ModuleId::Tremolo:
      switch (control) {
        case 0: return 0.40;  // Rate
        case 1: return 0.60;  // Depth
        case 2: return 0.20;  // Shape
        case 3: return 1.00;  // Mix
        default: return 0.50;
      }
    case ModuleId::AnalogDelay:
      switch (control) {
        case 0: return 0.40;  // 400 ms
        case 1: return 0.40;  // Feedback
        case 2: return 0.50;  // Tone
        case 3: return 0.30;  // Mix
        default: return 0.50;
      }
    case ModuleId::SpringReverb:
      switch (control) {
        case 0: return 0.40;  // Decay
        case 1: return 0.50;  // Tone
        case 2: return 0.30;  // Mix
        default: return 0.50;
      }
    case ModuleId::BassChorus:
      switch (control) {
        case 0: return 0.30;  // Rate
        case 1: return 0.50;  // Depth
        case 2: return 0.30;  // Low cut
        case 3: return 0.45;  // Mix
        default: return 0.50;
      }
    case ModuleId::Vibrato:
      switch (control) {
        case 0: return 0.45;  // Rate
        case 1: return 0.50;  // Depth
        case 2: return 1.00;  // Mix
        default: return 0.50;
      }
    case ModuleId::Octave:
      switch (control) {
        case 0: return 0.55;  // Down
        case 1: return 0.00;  // Up
        case 2: return 0.50;  // Tone
        case 3: return 1.00;  // Mix
        default: return 0.50;
      }
    case ModuleId::TapeEcho:
      switch (control) {
        case 0: return 0.38;  // Time
        case 1: return 0.42;  // Feedback
        case 2: return 0.20;  // Wow
        case 3: return 0.40;  // Tone
        case 4: return 0.30;  // Mix
        default: return 0.50;
      }
    case ModuleId::DigitalDelay:
      switch (control) {
        case 0: return 0.40;  // Time
        case 1: return 0.40;  // Feedback
        case 2: return 0.60;  // Tone
        case 3: return 0.30;  // Mix
        default: return 0.50;
      }
    case ModuleId::PingPongDelay:
      switch (control) {
        case 0: return 0.35;  // Time
        case 1: return 0.40;  // Feedback
        case 2: return 0.60;  // Tone
        case 3: return 0.30;  // Mix
        default: return 0.50;
      }
    case ModuleId::PlateReverb:
      switch (control) {
        case 0: return 0.40;  // Decay
        case 1: return 0.55;  // Tone
        case 2: return 0.30;  // Mix
        default: return 0.50;
      }
    case ModuleId::RoomReverb:
      switch (control) {
        case 0: return 0.30;  // Decay
        case 1: return 0.50;  // Tone
        case 2: return 0.30;  // Mix
        default: return 0.50;
      }
    case ModuleId::FetComp:
      switch (control) {
        case 0: return 0.35;  // Input
        case 1: return 0.45;  // Ratio
        case 2: return 0.15;  // Attack
        case 3: return 0.30;  // Release
        case 4: return 0.50;  // Output
        default: return 0.50;
      }
    case ModuleId::OptoComp:
      switch (control) {
        case 0: return 0.50;  // Peak
        case 1: return 0.50;  // Gain
        case 2: return 1.00;  // Mix
        default: return 0.50;
      }
    case ModuleId::EnvFilter:
      switch (control) {
        case 0: return 0.55;  // Sens
        case 1: return 0.40;  // Q
        case 2: return 0.55;  // Range
        case 3: return 0.60;  // Blend
        default: return 0.50;
      }
    case ModuleId::NoiseGate:
      switch (control) {
        case 0: return 0.55;  // Threshold
        case 1: return 0.35;  // Decay
        case 2: return 0.60;  // Range
        default: return 0.50;
      }
    case ModuleId::TightGate:
      switch (control) {
        case 0: return 0.55;  // Threshold
        case 1: return 0.20;  // Decay
        case 2: return 0.75;  // Range
        case 3: return 1.00;  // Link
        default: return 0.50;
      }
    case ModuleId::Eq3:
    case ModuleId::BassEq:
      switch (control) {
        case 0: return 0.50;  // Low
        case 1: return 0.50;  // Mid
        case 2: return 0.50;  // High
        case 3: return 0.50;  // Level
        default: return 0.50;
      }
    case ModuleId::LoFi:
      switch (control) {
        case 0: return 0.65;  // Bits
        case 1: return 0.70;  // Rate
        case 2: return 0.60;  // Tone
        case 3: return 1.00;  // Mix
        default: return 0.50;
      }
    case ModuleId::TapeSat:
      switch (control) {
        case 0: return 0.50;  // Drive
        case 1: return 0.50;  // Tone
        case 2: return 0.70;  // Mix
        default: return 0.50;
      }
    case ModuleId::PrecisionDrive:
      switch (control) {
        case 0: return 0.35;  // Gate
        case 1: return 0.45;  // Drive
        case 2: return 0.70;  // Attack
        case 3: return 0.50;  // Level
        default: return 0.50;
      }
    case ModuleId::AutoWah:
      switch (control) {
        case 0: return 0.55;  // Sens
        case 1: return 0.60;  // Q
        case 2: return 0.65;  // Range
        case 3: return 0.85;  // Mix
        default: return 0.50;
      }
    case ModuleId::Wah:
      switch (control) {
        case 0: return 0.35;  // Pedal
        case 1: return 0.55;  // Q
        case 2: return 0.60;  // Range
        case 3: return 1.00;  // Mix
        default: return 0.50;
      }
    case ModuleId::Laser:
      switch (control) {
        case 0: return 0.50;  // Pitch
        case 1: return 0.35;  // Ring
        case 2: return 0.45;  // Sens
        case 3: return 0.50;  // Pedal
        case 4: return 1.00;  // Mix
        default: return 0.50;
      }
    case ModuleId::Cab:
      switch (control) {
        case 0: return 0.18;  // Low cut
        case 1: return 0.55;  // High cut
        case 2: return 0.50;  // Level
        case 3: return 1.00;  // Mix
        default: return 0.50;
      }
    case ModuleId::Nam:
      switch (control) {
        case 0: return 0.50;  // Gain
        case 1: return 0.50;  // Bass
        case 2: return 0.50;  // Mid
        case 3: return 0.50;  // Treble
        case 4: return 0.50;  // Level
        case 5: return 0.35;  // Gate
        default: return 0.50;
      }
    default:
      return 0.50;
  }
}

} // namespace pedal
