// Pedal Suite — MIDI footswitch mapping
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

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

// CC 20..27 and 80..87 are the common footswitch assignments.
inline constexpr int footswitchSlotForCc(int cc) noexcept {
  if (cc >= 20 && cc < 20 + kSlotCount) return cc - 20;
  if (cc >= 80 && cc < 80 + kSlotCount) return cc - 80;
  return -1;
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

inline void toggleSlotBypass(Shared& shared, int slot) noexcept {
  if (slot < 0 || slot >= kSlotCount) return;
  const uint32_t param = slotParam(slot, kSlotOffsetBypass);
  shared.set(param, shared.get(param) > 0.5 ? 0.0 : 1.0);
}

} // namespace pedal
