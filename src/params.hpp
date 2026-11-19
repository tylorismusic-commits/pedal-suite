// Pedal Suite — parameters
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>

#include "dsp/controls.hpp"
#include "module_ids.hpp"

namespace pedal {

// Appended-only. Slot blocks are pre-allocated so automation ids survive a
// module change. Each slot is sixteen ids: type, bypass, mix, gain and twelve
// generic module controls.
enum ParamId : uint32_t {
  P_POWER = 0,
  P_INPUT,
  P_OUTPUT,
#define SLOT_PARAMS(n)                                                        \
  P_SLOT##n##_TYPE, P_SLOT##n##_BYPASS, P_SLOT##n##_MIX, P_SLOT##n##_GAIN,    \
      P_SLOT##n##_C0, P_SLOT##n##_C1, P_SLOT##n##_C2, P_SLOT##n##_C3,        \
      P_SLOT##n##_C4, P_SLOT##n##_C5, P_SLOT##n##_C6, P_SLOT##n##_C7,        \
      P_SLOT##n##_C8, P_SLOT##n##_C9, P_SLOT##n##_C10, P_SLOT##n##_C11,
  SLOT_PARAMS(0)
  SLOT_PARAMS(1)
  SLOT_PARAMS(2)
  SLOT_PARAMS(3)
  SLOT_PARAMS(4)
  SLOT_PARAMS(5)
  SLOT_PARAMS(6)
  SLOT_PARAMS(7)
#undef SLOT_PARAMS
  P_CAB_IR,
  P_NAM_MODEL,
  P_MIDI_NOTE0,
  P_MIDI_NOTE1,
  P_MIDI_NOTE2,
  P_MIDI_NOTE3,
  P_MIDI_NOTE4,
  P_MIDI_NOTE5,
  P_MIDI_NOTE6,
  P_MIDI_NOTE7,
  P_MIDI_CC0,
  P_MIDI_CC1,
  P_MIDI_CC2,
  P_MIDI_CC3,
  P_MIDI_CC4,
  P_MIDI_CC5,
  P_MIDI_CC6,
  P_MIDI_CC7,
  P_MIDI_EXPR_ON,
  P_MIDI_EXPR_SLOT,
  P_MIDI_EXPR_CTL,
  P_MIDI_EXPR_CC,
  P_COUNT
};

constexpr uint32_t kSlotStride = 16;
constexpr int kSlotOffsetType = 0;
constexpr int kSlotOffsetBypass = 1;
constexpr int kSlotOffsetMix = 2;
constexpr int kSlotOffsetGain = 3;
constexpr int kSlotOffsetC0 = 4;

constexpr uint32_t slotParam(int slot, int offset) noexcept {
  return P_SLOT0_TYPE + static_cast<uint32_t>(slot) * kSlotStride +
         static_cast<uint32_t>(offset);
}

struct Labels {
  const char* const* names = nullptr;
  int count = 0;
};

template <size_t N>
constexpr Labels labels(const char* const (&arr)[N]) noexcept {
  return Labels{arr, static_cast<int>(N)};
}

struct ParamSpec {
  const char* symbol;
  const char* name;
  const char* module;
  double min;
  double max;
  double def;
  bool stepped;
  bool hidden;
  bool toggled;
  const char* unit;
  Labels labels;
};

#define SLOT_DEFAULT_TYPE(n)                                                  \
  (n == 0 ? static_cast<double>(ModuleId::Boost)                              \
          : (n == 1 ? static_cast<double>(ModuleId::Drive808) : 0.0))
#define SLOT_DEFAULT_CTL(n, c)                                                \
  (n == 0 ? defaultControlValue(ModuleId::Boost, c)                           \
          : (n == 1 ? defaultControlValue(ModuleId::Drive808, c) : 0.5))

#define SLOT_SPECS(n, display)                                               \
  {"slot" #n "_type", "Module", "Slot " #display, 0.0,                         \
   static_cast<double>(kModuleCount - 1), SLOT_DEFAULT_TYPE(n), true, false,  \
   false, nullptr, labels(kModuleLabels)},                                    \
      {"slot" #n "_bypass", "Bypass", "Slot " #display, 0.0, 1.0, 0.0, true,   \
       false, true, nullptr, {}},                                             \
      {"slot" #n "_mix", "Mix", "Slot " #display, 0.0, 100.0, 100.0, false,  \
       false, false, "%", {}},                                                \
      {"slot" #n "_gain", "Gain", "Slot " #display, -12.0, 12.0, 0.0, false,  \
       false, false, "dB", {}},                                               \
      {"slot" #n "_c0", "Module Control 1", "Slot " #display, 0.0, 1.0,       \
       SLOT_DEFAULT_CTL(n, 0), false, false, false, nullptr, {}},             \
      {"slot" #n "_c1", "Module Control 2", "Slot " #display, 0.0, 1.0,                    \
       SLOT_DEFAULT_CTL(n, 1), false, false, false, nullptr, {}},             \
      {"slot" #n "_c2", "Module Control 3", "Slot " #display, 0.0, 1.0,                    \
       SLOT_DEFAULT_CTL(n, 2), false, false, false, nullptr, {}},             \
      {"slot" #n "_c3", "Module Control 4", "Slot " #display, 0.0, 1.0,                    \
       SLOT_DEFAULT_CTL(n, 3), false, false, false, nullptr, {}},             \
      {"slot" #n "_c4", "Module Control 5", "Slot " #display, 0.0, 1.0,                    \
       SLOT_DEFAULT_CTL(n, 4), false, false, false, nullptr, {}},             \
      {"slot" #n "_c5", "Module Control 6", "Slot " #display, 0.0, 1.0,                    \
       SLOT_DEFAULT_CTL(n, 5), false, false, false, nullptr, {}},             \
      {"slot" #n "_c6", "Module Control 7", "Slot " #display, 0.0, 1.0,                    \
       SLOT_DEFAULT_CTL(n, 6), false, false, false, nullptr, {}},             \
      {"slot" #n "_c7", "Module Control 8", "Slot " #display, 0.0, 1.0,                    \
       SLOT_DEFAULT_CTL(n, 7), false, false, false, nullptr, {}},             \
      {"slot" #n "_c8", "Module Control 9", "Slot " #display, 0.0, 1.0,                    \
       SLOT_DEFAULT_CTL(n, 8), false, false, false, nullptr, {}},             \
      {"slot" #n "_c9", "Module Control 10", "Slot " #display, 0.0, 1.0,                   \
       SLOT_DEFAULT_CTL(n, 9), false, false, false, nullptr, {}},             \
      {"slot" #n "_c10", "Module Control 11", "Slot " #display, 0.0, 1.0,                  \
       SLOT_DEFAULT_CTL(n, 10), false, false, false, nullptr, {}},            \
      {"slot" #n "_c11", "Module Control 12", "Slot " #display, 0.0, 1.0,                  \
       SLOT_DEFAULT_CTL(n, 11), false, false, false, nullptr, {}}

constexpr int kMaxCabIr = 64;
constexpr int kMaxNamModels = 64;

inline constexpr ParamSpec kParamSpecs[P_COUNT] = {
    {"power", "Power", "Global", 0.0, 1.0, 1.0, true, false, true, nullptr, {}},
    {"input", "Input", "Global", -18.0, 18.0, 0.0, false, false, false, "dB", {}},
    {"output", "Output", "Global", -18.0, 18.0, 0.0, false, false, false, "dB", {}},
    SLOT_SPECS(0, 1),
    SLOT_SPECS(1, 2),
    SLOT_SPECS(2, 3),
    SLOT_SPECS(3, 4),
    SLOT_SPECS(4, 5),
    SLOT_SPECS(5, 6),
    SLOT_SPECS(6, 7),
    SLOT_SPECS(7, 8),
    {"cab_ir", "Cabinet IR", "Cabinet", 0.0,
     static_cast<double>(kMaxCabIr), 0.0, true, true, false, nullptr, {}},
    {"nam_model", "NAM Model", "Amp", 0.0,
     static_cast<double>(kMaxNamModels), 0.0, true, true, false, nullptr, {}},
    {"midi_note1", "MIDI Note 1", "MIDI", 0.0, 127.0, 36.0, true, true, false,
     nullptr, {}},
    {"midi_note2", "MIDI Note 2", "MIDI", 0.0, 127.0, 37.0, true, true, false,
     nullptr, {}},
    {"midi_note3", "MIDI Note 3", "MIDI", 0.0, 127.0, 38.0, true, true, false,
     nullptr, {}},
    {"midi_note4", "MIDI Note 4", "MIDI", 0.0, 127.0, 39.0, true, true, false,
     nullptr, {}},
    {"midi_note5", "MIDI Note 5", "MIDI", 0.0, 127.0, 40.0, true, true, false,
     nullptr, {}},
    {"midi_note6", "MIDI Note 6", "MIDI", 0.0, 127.0, 41.0, true, true, false,
     nullptr, {}},
    {"midi_note7", "MIDI Note 7", "MIDI", 0.0, 127.0, 42.0, true, true, false,
     nullptr, {}},
    {"midi_note8", "MIDI Note 8", "MIDI", 0.0, 127.0, 43.0, true, true, false,
     nullptr, {}},
    {"midi_cc1", "MIDI CC 1", "MIDI", 0.0, 127.0, 20.0, true, true, false,
     nullptr, {}},
    {"midi_cc2", "MIDI CC 2", "MIDI", 0.0, 127.0, 21.0, true, true, false,
     nullptr, {}},
    {"midi_cc3", "MIDI CC 3", "MIDI", 0.0, 127.0, 22.0, true, true, false,
     nullptr, {}},
    {"midi_cc4", "MIDI CC 4", "MIDI", 0.0, 127.0, 23.0, true, true, false,
     nullptr, {}},
    {"midi_cc5", "MIDI CC 5", "MIDI", 0.0, 127.0, 24.0, true, true, false,
     nullptr, {}},
    {"midi_cc6", "MIDI CC 6", "MIDI", 0.0, 127.0, 25.0, true, true, false,
     nullptr, {}},
    {"midi_cc7", "MIDI CC 7", "MIDI", 0.0, 127.0, 26.0, true, true, false,
     nullptr, {}},
    {"midi_cc8", "MIDI CC 8", "MIDI", 0.0, 127.0, 27.0, true, true, false,
     nullptr, {}},
    {"midi_expr_on", "MIDI Expression", "MIDI", 0.0, 1.0, 0.0, true, true,
     true, nullptr, {}},
    {"midi_expr_slot", "Expression Slot", "MIDI", 0.0, 7.0, 0.0, true, true,
     false, nullptr, {}},
    {"midi_expr_ctl", "Expression Control", "MIDI", 0.0, 11.0, 0.0, true, true,
     false, nullptr, {}},
    {"midi_expr_cc", "Expression CC", "MIDI", 0.0, 127.0, 4.0, true, true,
     false, nullptr, {}},
};

#undef SLOT_SPECS
#undef SLOT_DEFAULT_CTL
#undef SLOT_DEFAULT_TYPE

static_assert(sizeof(kParamSpecs) / sizeof(kParamSpecs[0]) == P_COUNT,
              "kParamSpecs must cover every parameter id");

class Shared {
public:
  Shared() {
    for (uint32_t p = 0; p < P_COUNT; ++p) {
      values_[p].store(kParamSpecs[p].def, std::memory_order_relaxed);
      dirty_[p].store(false, std::memory_order_relaxed);
    }
    inPeakDb.store(-120.0f);
    outPeakDb.store(-120.0f);
    inPeakLDb.store(-120.0f);
    inPeakRDb.store(-120.0f);
    outPeakLDb.store(-120.0f);
    outPeakRDb.store(-120.0f);
    for (auto& g : slotGrDb) g.store(0.0f);
    tunerHz.store(0.0f);
    tunerCents.store(0.0f);
    tunerConfidence.store(0.0f);
    tunerNote.store(-1);
  }

  double get(uint32_t p) const noexcept {
    return values_[p].load(std::memory_order_relaxed);
  }

  void set(uint32_t p, double v) noexcept {
    if (!std::isfinite(v)) return;
    const ParamSpec& s = kParamSpecs[p];
    v = std::min(std::max(v, s.min), s.max);
    if (s.stepped) v = static_cast<double>(static_cast<int>(v + (v >= 0 ? 0.5 : -0.5)));
    const double previous = values_[p].load(std::memory_order_relaxed);
    values_[p].store(v, std::memory_order_relaxed);
    dirty_[p].store(true, std::memory_order_release);
    if (v != previous) {
      revision_.fetch_add(1, std::memory_order_relaxed);
    }
  }

  void setFromHost(uint32_t p, double v) noexcept {
    if (!std::isfinite(v)) return;
    const ParamSpec& s = kParamSpecs[p];
    v = std::min(std::max(v, s.min), s.max);
    if (s.stepped) v = static_cast<double>(static_cast<int>(v + (v >= 0 ? 0.5 : -0.5)));
    const double previous = values_[p].load(std::memory_order_relaxed);
    values_[p].store(v, std::memory_order_relaxed);
    if (v != previous) {
      revision_.fetch_add(1, std::memory_order_relaxed);
    }
  }

  bool takeDirty(uint32_t p) noexcept {
    return dirty_[p].exchange(false, std::memory_order_acquire);
  }
  bool isDirty(uint32_t p) const noexcept {
    return dirty_[p].load(std::memory_order_acquire);
  }

  bool anyDirty() const noexcept {
    for (uint32_t p = 0; p < P_COUNT; ++p)
      if (dirty_[p].load(std::memory_order_acquire)) return true;
    return false;
  }

  std::atomic<float> inPeakDb, outPeakDb;
  std::atomic<float> inPeakLDb, inPeakRDb;
  std::atomic<float> outPeakLDb, outPeakRDb;
  std::atomic<float> slotGrDb[kSlotCount];
  std::atomic<float> tunerHz, tunerCents, tunerConfidence;
  std::atomic<int> tunerNote;
  // Learn requests written by the editor and consumed by the audio thread,
  // plus feedback so an out-of-process LV2 UI can see when learn completes.
  std::atomic<int> midiLearnSlot{-1};
  std::atomic<bool> midiLearnExpr{false};
  std::atomic<int> midiLearnSlotState{-1};
  std::atomic<bool> midiLearnExprState{false};
  // Performance requests raised by MIDI and consumed by the editor.
  std::atomic<uint32_t> midiSetlistNextSeq{0};
  std::atomic<uint32_t> midiSetlistPrevSeq{0};
  std::atomic<uint32_t> midiSetlistPcSeq{0};
  std::atomic<int> midiSetlistPcValue{-1};
  std::atomic<uint32_t> midiBankNextSeq{0};
  std::atomic<uint32_t> midiBankPrevSeq{0};
  std::atomic<int> irBankCount{0};
  std::atomic<int> namBankCount{0};

  uint64_t revision() const noexcept {
    return revision_.load(std::memory_order_relaxed);
  }

private:
  std::atomic<double> values_[P_COUNT];
  std::atomic<bool> dirty_[P_COUNT];
  std::atomic<uint64_t> revision_{0};
};

inline Controls controlsFrom(const Shared& s) noexcept {
  Controls c;
  c.power = s.get(P_POWER) > 0.5;
  c.inputDb = static_cast<float>(s.get(P_INPUT));
  c.outputDb = static_cast<float>(s.get(P_OUTPUT));
  for (int i = 0; i < kSlotCount; ++i) {
    SlotControls& d = c.slot[i];
    d.type = static_cast<int>(s.get(slotParam(i, kSlotOffsetType)) + 0.5);
    d.bypass = s.get(slotParam(i, kSlotOffsetBypass)) > 0.5;
    d.mix = static_cast<float>(s.get(slotParam(i, kSlotOffsetMix)) / 100.0);
    d.gainDb = static_cast<float>(s.get(slotParam(i, kSlotOffsetGain)));
    for (int j = 0; j < kRackControlCount; ++j) {
      d.c[j] = static_cast<float>(s.get(slotParam(i, kSlotOffsetC0 + j)));
    }
  }
  return c;
}

} // namespace pedal
