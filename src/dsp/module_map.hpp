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

inline stomp::modules::TrebleBoostControls trebleBoostControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::TrebleBoostControls out;
  out.boost = c.c[0];
  out.range = c.c[1];
  out.level = c.c[2];
  return out;
}

inline stomp::modules::OverdriveControls overdriveControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::OverdriveControls out;
  out.drive = c.c[0];
  out.tone = c.c[1];
  out.level = c.c[2];
  out.mix = c.c[3];
  return out;
}

inline stomp::modules::BluesDriveControls bluesDriveControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::BluesDriveControls out;
  out.drive = c.c[0];
  out.tone = c.c[1];
  out.level = c.c[2];
  return out;
}

inline stomp::modules::DistortionControls distortionControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::DistortionControls out;
  out.dist = c.c[0];
  out.filter = c.c[1];
  out.level = c.c[2];
  return out;
}

inline stomp::modules::RatControls ratControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::RatControls out;
  out.dist = c.c[0];
  out.filter = c.c[1];
  out.level = c.c[2];
  return out;
}

inline stomp::modules::BigFuzzControls bigFuzzControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::BigFuzzControls out;
  out.sustain = c.c[0];
  out.tone = c.c[1];
  out.level = c.c[2];
  return out;
}

inline stomp::modules::FuzzFaceControls fuzzFaceControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::FuzzFaceControls out;
  out.fuzz = c.c[0];
  out.bias = c.c[1];
  out.volume = c.c[2];
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
