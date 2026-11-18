// Pedal Suite — pedal registry
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <cstring>

#include <hdsp/modules/eq.hpp>

#include "module_ids.hpp"
#include "params.hpp"

namespace pedal {

enum class ControlFormat {
  Percent,
  ThresholdDb,
  GainDb,
  Ratio,
  AttackMs,
  ReleaseMs,
  ScHpfHz,
  EqHpfHz,
  EqLpfHz,
  AirFreqHz,
  TiltDb,
  AirDb,
  GateThresholdDb,
  RangeDb,
  GateAttackMs,
  GateReleaseMs,
  HoldMs,
  BipolarPercent,
  ClipDriveDb,
  ClipCeilingDb,
  ClipMode,
  DeEssFreqHz,
  DeEssRangeDb,
  LimiterCeilingDb,
  LimiterReleaseMs,
  WidthPercent,
  MonoHz,
  CrushBits,
  CrushToneHz,
  FilterCutoffHz,
  LfoRateHz,
  FilterMode,
  MotionMode,
  SubFreqHz,
  DelayMs,
  DetuneMs,
  QValue,
  DynRangeDb,
  DynRatio,
  MakeupDb,
  RiderTargetDb,
  RiderSpeedMs,
  SpaceDecayS,
  SpacePreDelayMs,
  SpaceDelayMs,
  CompDistortion,
  BoostDb,
  ClassicThresholdDb,
  TrimDb,
  DaTubeOutputDb,
  FattenerGainDb,
  TapeColor,
  WideGainDb,
  CompAmount,
  AttackShortMs,
  AC1Mode,
  Threshold48Db,
  Threshold18Db,
  CurveBipolar,
  BloomInputDb,
  BloomOutputDb,
  RsInputDb,
  RsOutputDb,
  Ratio2To10,
  ThresholdM40P10Db,
  Unit10,
  Output24Db,
  TapeGainDb,
  Comp48Db,
  Threshold30Db,
  L2ReleaseMs,
  Threshold20Db,
  Margin6Db,
  HybridLowCut,
  HybridLowFreq,
  HybridBoost15,
  HybridMidFreq,
  HybridGain15,
  HybridHighFreq,
  HybridHighBoost,
  HybridHighAtten,
  PedalGainDb,
  PedalLevelDb,
};

struct ControlDesc {
  int param = 0;
  const char* label = "";
  const char* name = "";
  ControlFormat format = ControlFormat::Percent;
  bool stepped = false;
  Labels labels{};
  bool toggled = false;
};

using hdsp::modules::airFreqHz;
using hdsp::modules::eqHpfHz;
using hdsp::modules::eqLpfHz;

struct ModuleDesc {
  ModuleId id = ModuleId::Off;
  const char* name = "Off";
  const char* category = "Off";
  const char* description = "Empty pedal slot";
  bool isEq = false;
  bool isComp = false;
  int model = 0;
  int controlCount = 0;
  const ControlDesc* controls = nullptr;
};

namespace detail {

inline constexpr ControlDesc kBoostControls[] = {
    {0, "GAIN", "Gain", ControlFormat::PedalGainDb},
    {1, "TONE", "Tone", ControlFormat::TiltDb},
    {2, "LEVEL", "Level", ControlFormat::PedalLevelDb},
};

inline constexpr ControlDesc kDrive808Controls[] = {
    {0, "DRIVE", "Drive", ControlFormat::Percent},
    {1, "TONE", "Tone", ControlFormat::Percent},
    {2, "LEVEL", "Level", ControlFormat::Percent},
};

inline constexpr ControlDesc kTrebleBoostControls[] = {
    {0, "BOOST", "Boost", ControlFormat::Percent},
    {1, "RANGE", "Range", ControlFormat::Percent},
    {2, "LEVEL", "Level", ControlFormat::PedalLevelDb},
};
inline constexpr ControlDesc kOverdriveControls[] = {
    {0, "DRIVE", "Drive", ControlFormat::Percent},
    {1, "TONE", "Tone", ControlFormat::Percent},
    {2, "LEVEL", "Level", ControlFormat::PedalLevelDb},
    {3, "MIX", "Mix", ControlFormat::Percent},
};
inline constexpr ControlDesc kBluesDriveControls[] = {
    {0, "DRIVE", "Drive", ControlFormat::Percent},
    {1, "TONE", "Tone", ControlFormat::Percent},
    {2, "LEVEL", "Level", ControlFormat::PedalLevelDb},
};
inline constexpr ControlDesc kDistortionControls[] = {
    {0, "DIST", "Distortion", ControlFormat::Percent},
    {1, "FILTER", "Filter", ControlFormat::Percent},
    {2, "LEVEL", "Level", ControlFormat::PedalLevelDb},
};
inline constexpr ControlDesc kRatControls[] = {
    {0, "DIST", "Distortion", ControlFormat::Percent},
    {1, "FILTER", "Filter", ControlFormat::Percent},
    {2, "LEVEL", "Level", ControlFormat::PedalLevelDb},
};
inline constexpr ControlDesc kBigFuzzControls[] = {
    {0, "SUSTAIN", "Sustain", ControlFormat::Percent},
    {1, "TONE", "Tone", ControlFormat::Percent},
    {2, "LEVEL", "Level", ControlFormat::PedalLevelDb},
};
inline constexpr ControlDesc kFuzzFaceControls[] = {
    {0, "FUZZ", "Fuzz", ControlFormat::Percent},
    {1, "BIAS", "Bias", ControlFormat::Percent},
    {2, "VOLUME", "Volume", ControlFormat::Percent},
};
inline constexpr ControlDesc kToneBenderControls[] = {
    {0, "ATTACK", "Attack", ControlFormat::Percent},
    {1, "TONE", "Tone", ControlFormat::Percent},
    {2, "LEVEL", "Level", ControlFormat::PedalLevelDb},
};
inline constexpr ControlDesc kBassDriveControls[] = {
    {0, "DRIVE", "Drive", ControlFormat::Percent},
    {1, "BLEND", "Blend", ControlFormat::Percent},
    {2, "TONE", "Tone", ControlFormat::Percent},
    {3, "LEVEL", "Level", ControlFormat::PedalLevelDb},
};
inline constexpr ControlDesc kBassFuzzControls[] = {
    {0, "FUZZ", "Fuzz", ControlFormat::Percent},
    {1, "BLEND", "Blend", ControlFormat::Percent},
    {2, "TONE", "Tone", ControlFormat::Percent},
    {3, "LEVEL", "Level", ControlFormat::PedalLevelDb},
};

inline constexpr ModuleDesc kModules[] = {
    {ModuleId::Off, "Off", "Off", "Empty pedal slot", false, false, 0, 0,
     nullptr},
    {ModuleId::Boost, "Boost", "Drive", "Clean FET-style boost pedal", false,
     false, 0,
     static_cast<int>(sizeof(kBoostControls) / sizeof(kBoostControls[0])),
     kBoostControls},
    {ModuleId::Drive808, "808 Drive", "Drive", "Retuned TS9/808 overdrive",
     false, false, 0,
     static_cast<int>(sizeof(kDrive808Controls) / sizeof(kDrive808Controls[0])),
     kDrive808Controls},
    {ModuleId::TrebleBoost, "Treble Boost", "Drive",
     "Rangemaster-style treble booster", false, false, 0,
     static_cast<int>(sizeof(kTrebleBoostControls) /
                      sizeof(kTrebleBoostControls[0])),
     kTrebleBoostControls},
    {ModuleId::Overdrive, "Overdrive", "Drive", "Transparent soft-clip overdrive",
     false, false, 0,
     static_cast<int>(sizeof(kOverdriveControls) /
                      sizeof(kOverdriveControls[0])),
     kOverdriveControls},
    {ModuleId::BluesDrive, "Blues Drive", "Drive", "Low-gain blues overdrive",
     false, false, 0,
     static_cast<int>(sizeof(kBluesDriveControls) /
                      sizeof(kBluesDriveControls[0])),
     kBluesDriveControls},
    {ModuleId::Distortion, "Distortion", "Drive", "Hard-clip distortion", false,
     false, 0,
     static_cast<int>(sizeof(kDistortionControls) /
                      sizeof(kDistortionControls[0])),
     kDistortionControls},
    {ModuleId::Rat, "Rat", "Drive", "Rat-style hard-clipping distortion", false,
     false, 0,
     static_cast<int>(sizeof(kRatControls) / sizeof(kRatControls[0])),
     kRatControls},
    {ModuleId::BigFuzz, "Big Fuzz", "Drive", "Sustaining muff-style fuzz", false,
     false, 0,
     static_cast<int>(sizeof(kBigFuzzControls) / sizeof(kBigFuzzControls[0])),
     kBigFuzzControls},
    {ModuleId::FuzzFace, "Fuzz Face", "Drive", "Germanium-style fuzz", false,
     false, 0,
     static_cast<int>(sizeof(kFuzzFaceControls) / sizeof(kFuzzFaceControls[0])),
     kFuzzFaceControls},
    {ModuleId::ToneBender, "Tone Bender", "Drive", "Silicon fuzz", false, false,
     0,
     static_cast<int>(sizeof(kToneBenderControls) /
                      sizeof(kToneBenderControls[0])),
     kToneBenderControls},
    {ModuleId::BassDrive, "Bass Drive", "Drive",
     "Bass overdrive with clean low end", false, false, 0,
     static_cast<int>(sizeof(kBassDriveControls) /
                      sizeof(kBassDriveControls[0])),
     kBassDriveControls},
    {ModuleId::BassFuzz, "Bass Fuzz", "Drive",
     "Bass fuzz with clean low end", false, false, 0,
     static_cast<int>(sizeof(kBassFuzzControls) /
                      sizeof(kBassFuzzControls[0])),
     kBassFuzzControls},
};

inline constexpr const char* kModuleSources[] = {
    "\u2014",
    "Pedal Suite exclusive",
    "Amp Suite",
    "Pedal Suite exclusive",
    "Pedal Suite exclusive",
    "Pedal Suite exclusive",
    "Pedal Suite exclusive",
    "Pedal Suite exclusive",
    "Pedal Suite exclusive",
    "Pedal Suite exclusive",
    "Pedal Suite exclusive",
    "Pedal Suite exclusive",
    "Pedal Suite exclusive",
};

constexpr bool moduleSourcesComplete() noexcept {
  for (int i = 0; i < kModuleCount; ++i) {
    if (!kModuleSources[i]) return false;
  }
  return true;
}
static_assert(moduleSourcesComplete(),
              "kModuleSources must fill every module id");
static_assert(sizeof(kModuleSources) / sizeof(kModuleSources[0]) == kModuleCount,
              "kModuleSources must cover every module id");

} // namespace detail

inline const ModuleDesc* moduleDesc(int id) noexcept {
  return validModule(id) ? &detail::kModules[id] : &detail::kModules[0];
}

inline const char* moduleName(int id) noexcept { return moduleDesc(id)->name; }

inline const char* moduleSource(int id) noexcept {
  return validModule(id) ? detail::kModuleSources[id] : "\u2014";
}

inline void applyModuleDefaults(Shared& shared, int slot, int type,
                                bool markDirty = true) {
  const ModuleDesc* d = moduleDesc(type);
  const ModuleId id = d->id;
  auto set = [&](uint32_t param, double value) {
    if (markDirty) {
      shared.set(param, value);
    } else {
      shared.setFromHost(param, value);
    }
  };
  set(slotParam(slot, kSlotOffsetType), static_cast<double>(type));
  set(slotParam(slot, kSlotOffsetBypass), 0.0);
  set(slotParam(slot, kSlotOffsetMix), 100.0);
  set(slotParam(slot, kSlotOffsetGain), 0.0);
  for (int i = 0; i < kRackControlCount; ++i) {
    set(slotParam(slot, kSlotOffsetC0 + i), defaultControlValue(id, i));
  }
}

inline void initializeRack(Shared& shared) {
  for (int slot = 0; slot < kSlotCount; ++slot) {
    applyModuleDefaults(shared, slot, static_cast<int>(ModuleId::Off), false);
  }
  applyModuleDefaults(shared, 0, static_cast<int>(ModuleId::Boost), false);
  applyModuleDefaults(shared, 1, static_cast<int>(ModuleId::Drive808), false);
}

} // namespace pedal
