// Pedal Suite — rack controls to pedal controls
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <algorithm>
#include <cmath>

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

inline stomp::modules::PitchDropControls pitchDropControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::PitchDropControls out;
  out.pitch = c.c[0];
  out.mode = static_cast<int>(std::lround(std::clamp(c.c[1], 0.0f, 1.0f) * 4.0f));
  out.glide = c.c[2];
  out.tone = c.c[3];
  out.mix = c.c[4];
  return out;
}

inline stomp::modules::RetuneControls retuneControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::RetuneControls out;
  out.semitones = c.c[0];
  out.cents = c.c[1];
  out.lowCut = c.c[2];
  out.mix = c.c[3];
  return out;
}

inline stomp::modules::ChorusControls chorusControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::ChorusControls out;
  out.rate = c.c[0];
  out.depth = c.c[1];
  out.width = c.c[2];
  out.mix = c.c[3];
  return out;
}

inline stomp::modules::FlangerControls flangerControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::FlangerControls out;
  out.rate = c.c[0];
  out.depth = c.c[1];
  out.feedback = c.c[2];
  out.mix = c.c[3];
  return out;
}

inline stomp::modules::PhaserControls phaserControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::PhaserControls out;
  out.rate = c.c[0];
  out.depth = c.c[1];
  out.feedback = c.c[2];
  out.mix = c.c[3];
  return out;
}

inline stomp::modules::TremoloControls tremoloControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::TremoloControls out;
  out.rate = c.c[0];
  out.depth = c.c[1];
  out.shape = c.c[2];
  out.mix = c.c[3];
  return out;
}

inline stomp::modules::AnalogDelayControls analogDelayControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::AnalogDelayControls out;
  out.time = c.c[0];
  out.feedback = c.c[1];
  out.tone = c.c[2];
  out.mix = c.c[3];
  return out;
}

inline stomp::modules::SpringReverbControls springReverbControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::SpringReverbControls out;
  out.decay = c.c[0];
  out.tone = c.c[1];
  out.mix = c.c[2];
  return out;
}

inline stomp::modules::BassChorusControls bassChorusControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::BassChorusControls out;
  out.rate = c.c[0];
  out.depth = c.c[1];
  out.lowCut = c.c[2];
  out.mix = c.c[3];
  return out;
}

inline stomp::modules::VibratoControls vibratoControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::VibratoControls out;
  out.rate = c.c[0];
  out.depth = c.c[1];
  out.mix = c.c[2];
  return out;
}

inline stomp::modules::OctaveControls octaveControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::OctaveControls out;
  out.down = c.c[0];
  out.up = c.c[1];
  out.tone = c.c[2];
  out.mix = c.c[3];
  return out;
}

inline stomp::modules::TapeEchoControls tapeEchoControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::TapeEchoControls out;
  out.time = c.c[0];
  out.feedback = c.c[1];
  out.wow = c.c[2];
  out.tone = c.c[3];
  out.mix = c.c[4];
  return out;
}

inline stomp::modules::DigitalDelayControls digitalDelayControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::DigitalDelayControls out;
  out.time = c.c[0];
  out.feedback = c.c[1];
  out.tone = c.c[2];
  out.mix = c.c[3];
  return out;
}

inline stomp::modules::PingPongControls pingPongControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::PingPongControls out;
  out.time = c.c[0];
  out.feedback = c.c[1];
  out.tone = c.c[2];
  out.mix = c.c[3];
  return out;
}

inline stomp::modules::PlateReverbControls plateReverbControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::PlateReverbControls out;
  out.decay = c.c[0];
  out.tone = c.c[1];
  out.mix = c.c[2];
  return out;
}

inline stomp::modules::RoomReverbControls roomReverbControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::RoomReverbControls out;
  out.decay = c.c[0];
  out.tone = c.c[1];
  out.mix = c.c[2];
  return out;
}

inline stomp::modules::FetCompControls fetCompControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::FetCompControls out;
  out.input = c.c[0];
  out.ratio = c.c[1];
  out.attack = c.c[2];
  out.release = c.c[3];
  out.output = c.c[4];
  return out;
}

inline stomp::modules::OptoCompControls optoCompControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::OptoCompControls out;
  out.peak = c.c[0];
  out.gain = c.c[1];
  out.mix = c.c[2];
  return out;
}

inline stomp::modules::EnvFilterControls envFilterControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::EnvFilterControls out;
  out.sens = c.c[0];
  out.q = c.c[1];
  out.range = c.c[2];
  out.blend = c.c[3];
  return out;
}

inline stomp::modules::GatePedalControls gateControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::GatePedalControls out;
  out.threshold = c.c[0];
  out.decay = c.c[1];
  out.range = c.c[2];
  out.link = 1.0f;
  return out;
}

inline stomp::modules::GatePedalControls tightGateControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::GatePedalControls out;
  out.threshold = c.c[0];
  out.decay = c.c[1];
  out.range = c.c[2];
  out.link = c.c[3];
  return out;
}

inline stomp::modules::Eq3Controls eq3ControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::Eq3Controls out;
  out.low = c.c[0];
  out.mid = c.c[1];
  out.high = c.c[2];
  out.level = c.c[3];
  return out;
}

inline stomp::modules::LoFiControls loFiControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::LoFiControls out;
  out.bits = c.c[0];
  out.rate = c.c[1];
  out.tone = c.c[2];
  out.mix = c.c[3];
  return out;
}

inline stomp::modules::TapeSatControls tapeSatControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::TapeSatControls out;
  out.drive = c.c[0];
  out.tone = c.c[1];
  out.mix = c.c[2];
  return out;
}

inline stomp::modules::ToneBenderControls toneBenderControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::ToneBenderControls out;
  out.attack = c.c[0];
  out.tone = c.c[1];
  out.level = c.c[2];
  return out;
}

inline stomp::modules::BassDriveControls bassDriveControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::BassDriveControls out;
  out.drive = c.c[0];
  out.blend = c.c[1];
  out.tone = c.c[2];
  out.level = c.c[3];
  return out;
}

inline stomp::modules::BassFuzzControls bassFuzzControlsFrom(
    const SlotControls& c) noexcept {
  stomp::modules::BassFuzzControls out;
  out.fuzz = c.c[0];
  out.blend = c.c[1];
  out.tone = c.c[2];
  out.level = c.c[3];
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
