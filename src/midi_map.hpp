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

inline void toggleSlotBypass(Shared& shared, int slot) noexcept {
  if (slot < 0 || slot >= kSlotCount) return;
  const uint32_t param = slotParam(slot, kSlotOffsetBypass);
  shared.set(param, shared.get(param) > 0.5 ? 0.0 : 1.0);
}

} // namespace pedal
