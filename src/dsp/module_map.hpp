// Pedal Suite — rack controls to pedal controls
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <stomp/modules/modules.hpp>

#include "dsp/controls.hpp"

namespace pedal {

inline stomp::modules::BoostControls boostControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::BoostControls out;
  out.gain = c.c[0];
  out.tone = c.c[1];
  out.level = c.c[2];
  return out;
}

inline stomp::modules::Drive808Controls drive808ControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::Drive808Controls out;
  out.drive = c.c[0];
  out.tone = c.c[1];
  out.level = c.c[2];
  return out;
}

} // namespace pedal
