// Pedal Suite — serial pedalboard engine
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <algorithm>
#include <cmath>

#include <hdsp/db.hpp>
#include <stomp/modules/modules.hpp>

#include "dsp/controls.hpp"
#include "dsp/module_map.hpp"
#include "module_ids.hpp"

namespace pedal {

class Engine {
public:
  void prepare(float sampleRate) noexcept {
    sr_ = sampleRate > 0.0f ? sampleRate : 48000.0f;
    const float smooth = 1.0f - std::exp(-1.0f / (0.01f * sr_));
    smooth_ = smooth;
    for (auto& boost : boost_) boost.prepare(sr_);
    for (auto& drive : drive_) drive.prepare(sr_);
    for (auto& p : treble_) p.prepare(sr_);
    for (auto& p : overdrive_) p.prepare(sr_);
    for (auto& p : blues_) p.prepare(sr_);
    for (auto& p : distortion_) p.prepare(sr_);
    for (auto& p : rat_) p.prepare(sr_);
    for (auto& p : bigFuzz_) p.prepare(sr_);
    for (auto& p : fuzzFace_) p.prepare(sr_);
    for (auto& p : toneBender_) p.prepare(sr_);
    for (auto& p : bassDrive_) p.prepare(sr_);
    for (auto& p : bassFuzz_) p.prepare(sr_);
    reset();
  }

  void setControls(const Controls& c) noexcept {
    powerTarget_ = c.power ? 1.0f : 0.0f;
    inputTarget_ = hdsp::dbToGain(c.inputDb);
    outputTarget_ = hdsp::dbToGain(c.outputDb);
    for (int i = 0; i < kSlotCount; ++i) {
      slot_[i] = c.slot[i];
      boost_[i].setControls(boostControlsFrom(c.slot[i]));
      drive_[i].setControls(drive808ControlsFrom(c.slot[i]));
      treble_[i].setControls(trebleBoostControlsFrom(c.slot[i]));
      overdrive_[i].setControls(overdriveControlsFrom(c.slot[i]));
      blues_[i].setControls(bluesDriveControlsFrom(c.slot[i]));
      distortion_[i].setControls(distortionControlsFrom(c.slot[i]));
      rat_[i].setControls(ratControlsFrom(c.slot[i]));
      bigFuzz_[i].setControls(bigFuzzControlsFrom(c.slot[i]));
      fuzzFace_[i].setControls(fuzzFaceControlsFrom(c.slot[i]));
      toneBender_[i].setControls(toneBenderControlsFrom(c.slot[i]));
      bassDrive_[i].setControls(bassDriveControlsFrom(c.slot[i]));
      bassFuzz_[i].setControls(bassFuzzControlsFrom(c.slot[i]));
    }
  }

  void process(float* left, float* right, int n, Meters& meters) noexcept {
    if (!left || !right || n <= 0) return;
    float inL = 0.0f, inR = 0.0f, outL = 0.0f, outR = 0.0f;

    for (int i = 0; i < n; ++i) {
      const float inSampleL = left[i], inSampleR = right[i];
      float l = inSampleL * input_;
      float r = inSampleR * input_;
      const float dryL = l, dryR = r;
      for (int s = 0; s < kSlotCount; ++s) {
        const SlotControls& c = slot_[s];
        if (c.type == static_cast<int>(ModuleId::Off) || c.bypass) continue;
        const float sl = l, sr = r;
        switch (static_cast<ModuleId>(c.type)) {
          case ModuleId::Boost:
            boost_[s].process(l, r);
            break;
          case ModuleId::Drive808:
            drive_[s].process(l, r);
            break;
          case ModuleId::TrebleBoost:
            treble_[s].process(l, r);
            break;
          case ModuleId::Overdrive:
            overdrive_[s].process(l, r);
            break;
          case ModuleId::BluesDrive:
            blues_[s].process(l, r);
            break;
          case ModuleId::Distortion:
            distortion_[s].process(l, r);
            break;
          case ModuleId::Rat:
            rat_[s].process(l, r);
            break;
          case ModuleId::BigFuzz:
            bigFuzz_[s].process(l, r);
            break;
          case ModuleId::FuzzFace:
            fuzzFace_[s].process(l, r);
            break;
          case ModuleId::ToneBender:
            toneBender_[s].process(l, r);
            break;
          case ModuleId::BassDrive:
            bassDrive_[s].process(l, r);
            break;
          case ModuleId::BassFuzz:
            bassFuzz_[s].process(l, r);
            break;
          case ModuleId::Off:
          case ModuleId::Count:
            continue;
        }
        const float mix = std::clamp(c.mix, 0.0f, 1.0f);
        l = sl + (l - sl) * mix;
        r = sr + (r - sr) * mix;
        const float gain = hdsp::dbToGain(c.gainDb);
        l *= gain;
        r *= gain;
      }
      const float active = power_;
      l = dryL + active * (l - dryL);
      r = dryR + active * (r - dryR);
      l *= output_;
      r *= output_;
      left[i] = std::isfinite(l) ? l : 0.0f;
      right[i] = std::isfinite(r) ? r : 0.0f;
      inL = std::max(inL, std::fabs(inSampleL));
      inR = std::max(inR, std::fabs(inSampleR));
      outL = std::max(outL, std::fabs(left[i]));
      outR = std::max(outR, std::fabs(right[i]));

      input_ += (inputTarget_ - input_) * smooth_;
      output_ += (outputTarget_ - output_) * smooth_;
      power_ += (powerTarget_ - power_) * smooth_;
    }

    meters.inPeakLDb = hdsp::gainToDb(inL);
    meters.inPeakRDb = hdsp::gainToDb(inR);
    meters.inPeakDb = std::max(meters.inPeakLDb, meters.inPeakRDb);
    meters.outPeakLDb = hdsp::gainToDb(outL);
    meters.outPeakRDb = hdsp::gainToDb(outR);
    meters.outPeakDb = std::max(meters.outPeakLDb, meters.outPeakRDb);
    for (int s = 0; s < kSlotCount; ++s) meters.slotGrDb[s] = 0.0f;
  }

  void reset() noexcept {
    for (auto& boost : boost_) boost.reset();
    for (auto& drive : drive_) drive.reset();
    for (auto& p : treble_) p.reset();
    for (auto& p : overdrive_) p.reset();
    for (auto& p : blues_) p.reset();
    for (auto& p : distortion_) p.reset();
    for (auto& p : rat_) p.reset();
    for (auto& p : bigFuzz_) p.reset();
    for (auto& p : fuzzFace_) p.reset();
    for (auto& p : toneBender_) p.reset();
    for (auto& p : bassDrive_) p.reset();
    for (auto& p : bassFuzz_) p.reset();
    input_ = inputTarget_;
    output_ = outputTarget_;
    power_ = powerTarget_;
  }

  int latencySamples() const noexcept { return 0; }

private:
  float sr_ = 48000.0f;
  float smooth_ = 0.01f;
  float input_ = 1.0f, output_ = 1.0f, power_ = 1.0f;
  float inputTarget_ = 1.0f, outputTarget_ = 1.0f, powerTarget_ = 1.0f;
  SlotControls slot_[kSlotCount];
  stomp::modules::Boost boost_[kSlotCount];
  stomp::modules::Drive808 drive_[kSlotCount];
  stomp::modules::TrebleBoost treble_[kSlotCount];
  stomp::modules::Overdrive overdrive_[kSlotCount];
  stomp::modules::BluesDrive blues_[kSlotCount];
  stomp::modules::Distortion distortion_[kSlotCount];
  stomp::modules::Rat rat_[kSlotCount];
  stomp::modules::BigFuzz bigFuzz_[kSlotCount];
  stomp::modules::FuzzFace fuzzFace_[kSlotCount];
  stomp::modules::ToneBender toneBender_[kSlotCount];
  stomp::modules::BassDrive bassDrive_[kSlotCount];
  stomp::modules::BassFuzz bassFuzz_[kSlotCount];
};

} // namespace pedal
