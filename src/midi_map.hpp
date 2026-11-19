// Pedal Suite — MIDI footswitch mapping
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <algorithm>
#include <cmath>

#include "module_ids.hpp"
#include "params.hpp"

namespace pedal {

// Foot controllers commonly send either low notes (C2..G2) or middle C
// octave notes (C4..G4). Both ranges toggle pedals 1..8.
inline constexpr int footswitchSlotForNote(int note) noexcept {
  if (note >= 36 && note < 36 + kSlotCount) return note - 36;
  if (note >= 60 && note < 60 + kSlotCount) return note - 60;
  return -1;
}

// Performance controls. Program Change selects a setlist entry (or a preset
// within the current bank when no setlist is active).
inline constexpr int kSetlistPrevCc = 28;
inline constexpr int kSetlistNextCc = 29;
inline constexpr int kBankPrevCc = 30;
inline constexpr int kBankNextCc = 31;

// CC 20..27 and 80..87 are the common footswitch assignments.
inline constexpr int footswitchSlotForCc(int cc) noexcept {
  if (cc >= 20 && cc < 20 + kSlotCount) return cc - 20;
  if (cc >= 80 && cc < 80 + kSlotCount) return cc - 80;
  return -1;
}

inline uint32_t midiNoteParam(int slot) noexcept {
  return static_cast<uint32_t>(P_MIDI_NOTE0 + slot);
}

inline uint32_t midiCcParam(int slot) noexcept {
  return static_cast<uint32_t>(P_MIDI_CC0 + slot);
}

// A learned mapping wins over the factory note/CC ranges.  A stored 0 means
// "unassigned"; note/CC 0 are outside the useful footswitch range.
inline int mappedNoteSlot(const Shared& shared, int note) noexcept {
  if (note <= 0) return -1;
  for (int slot = 0; slot < kSlotCount; ++slot) {
    const int mapped =
        static_cast<int>(std::lround(shared.get(midiNoteParam(slot))));
    if (mapped == note) return slot;
  }
  return footswitchSlotForNote(note);
}

inline int mappedCcSlot(const Shared& shared, int cc) noexcept {
  if (cc <= 0) return -1;
  for (int slot = 0; slot < kSlotCount; ++slot) {
    const int mapped =
        static_cast<int>(std::lround(shared.get(midiCcParam(slot))));
    if (mapped == cc) return slot;
  }
  return footswitchSlotForCc(cc);
}

inline void assignNoteMapping(Shared& shared, int slot, int note) noexcept {
  if (slot < 0 || slot >= kSlotCount || note <= 0) return;
  for (int other = 0; other < kSlotCount; ++other) {
    if (other == slot) continue;
    if (static_cast<int>(std::lround(shared.get(midiNoteParam(other)))) ==
        note) {
      shared.set(midiNoteParam(other), 0.0);
    }
  }
  shared.set(midiNoteParam(slot), static_cast<double>(note));
}

inline void assignCcMapping(Shared& shared, int slot, int cc) noexcept {
  if (slot < 0 || slot >= kSlotCount || cc <= 0) return;
  for (int other = 0; other < kSlotCount; ++other) {
    if (other == slot) continue;
    if (static_cast<int>(std::lround(shared.get(midiCcParam(other)))) == cc) {
      shared.set(midiCcParam(other), 0.0);
    }
  }
  shared.set(midiCcParam(slot), static_cast<double>(cc));
}

inline void setWahPedal(Shared& shared, int slot, float position) noexcept {
  if (slot < 0 || slot >= kSlotCount) return;
  if (static_cast<int>(shared.get(slotParam(slot, kSlotOffsetType)) + 0.5) !=
      static_cast<int>(ModuleId::Wah)) {
    return;
  }
  const float v = position < 0.0f ? 0.0f : (position > 1.0f ? 1.0f : position);
  shared.set(slotParam(slot, kSlotOffsetC0 + 0), v);
}

// An expression pedal drives the first Wah in the chain.
inline void setFirstWahPedal(Shared& shared, float position) noexcept {
  for (int slot = 0; slot < kSlotCount; ++slot) {
    if (static_cast<int>(
            shared.get(slotParam(slot, kSlotOffsetType)) + 0.5) ==
        static_cast<int>(ModuleId::Wah)) {
      setWahPedal(shared, slot, position);
      return;
    }
  }
}

inline bool expressionEnabled(const Shared& shared) noexcept {
  return shared.get(P_MIDI_EXPR_ON) > 0.5;
}

inline int expressionCc(const Shared& shared) noexcept {
  return static_cast<int>(std::lround(shared.get(P_MIDI_EXPR_CC)));
}

inline int expressionSlot(const Shared& shared) noexcept {
  return std::clamp(
      static_cast<int>(std::lround(shared.get(P_MIDI_EXPR_SLOT))), 0,
      kSlotCount - 1);
}

inline int expressionControl(const Shared& shared) noexcept {
  return std::clamp(
      static_cast<int>(std::lround(shared.get(P_MIDI_EXPR_CTL))), 0,
      kRackControlCount - 1);
}

// When expression is enabled the learned CC drives the chosen control on the
// chosen slot.  With it off, CC 4/11 keep the original first-Wah behaviour.
inline void applyExpression(Shared& shared, float position) noexcept {
  const float v = position < 0.0f ? 0.0f : (position > 1.0f ? 1.0f : position);
  if (!expressionEnabled(shared)) {
    setFirstWahPedal(shared, v);
    return;
  }
  const int slot = expressionSlot(shared);
  const int control = expressionControl(shared);
  shared.set(slotParam(slot, kSlotOffsetC0 + control), v);
}

inline void toggleSlotBypass(Shared& shared, int slot) noexcept {
  if (slot < 0 || slot >= kSlotCount) return;
  const uint32_t param = slotParam(slot, kSlotOffsetBypass);
  shared.set(param, shared.get(param) > 0.5 ? 0.0 : 1.0);
}

} // namespace pedal
