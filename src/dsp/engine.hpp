// Pedal Suite — serial pedalboard engine
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

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
    smooth_ = 1.0f - std::exp(-1.0f / (0.01f * sr_));
    for (int s = 0; s < kSlotCount; ++s) preparedType_[s] = -1;
    revision_ = 0;
    tuner_.prepare(sr_);
    reset();
  }

  // Modules are prepared lazily when a slot first selects them. This keeps
  // idle/small boards cheap.
  void setControls(const Controls& c, uint64_t revision = 0) noexcept {
    if (revision != 0 && revision == revision_) return;
    revision_ = revision;
    powerTarget_ = c.power ? 1.0f : 0.0f;
    inputTarget_ = hdsp::dbToGain(c.inputDb);
    outputTarget_ = hdsp::dbToGain(c.outputDb);
    for (int i = 0; i < kSlotCount; ++i) {
      const int type = c.slot[i].type;
      if (type != preparedType_[i]) {
        prepareSlot(i, type);
        preparedType_[i] = type;
      }
      slot_[i] = c.slot[i];
      setSlotControls(i, type, c.slot[i]);
    }
  }

  void setCabIr(const float* h, int taps) noexcept {
    for (auto& c : cab_) c.setIR(h, taps);
  }

  void process(float* left, float* right, int n, Meters& meters) noexcept {
    if (!left || !right || n <= 0) return;
    float inL = 0.0f, inR = 0.0f, outL = 0.0f, outR = 0.0f;

    for (int i = 0; i < n; ++i) {
      const float inSampleL = left[i], inSampleR = right[i];
      tuner_.process(inSampleL, inSampleR);
      float l = inSampleL * input_;
      float r = inSampleR * input_;
      const float dryL = l, dryR = r;
      for (int s = 0; s < kSlotCount; ++s) {
        const SlotControls& c = slot_[s];
        if (c.type == static_cast<int>(ModuleId::Off)) continue;
        const ModuleId id = static_cast<ModuleId>(c.type);
        if (c.bypass) {
          // Let delay and reverb tails ring out with the input muted.
          if (moduleHasTails(c.type)) {
            float tailL = 0.0f, tailR = 0.0f;
            processSlot(s, id, tailL, tailR);
            l += tailL;
            r += tailR;
          }
          continue;
        }
        const float sl = l, sr = r;
        processSlot(s, id, l, r);
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
    for (int s = 0; s < kSlotCount; ++s) {
      meters.slotGrDb[s] = 0.0f;
      const SlotControls& c = slot_[s];
      if (c.bypass) continue;
      switch (static_cast<ModuleId>(c.type)) {
        case ModuleId::FetComp:
          meters.slotGrDb[s] = fetComp_[s].gainReductionDb();
          break;
        case ModuleId::OptoComp:
          meters.slotGrDb[s] = optoComp_[s].gainReductionDb();
          break;
        case ModuleId::NoiseGate:
          meters.slotGrDb[s] = noiseGate_[s].gainReductionDb();
          break;
        case ModuleId::TightGate:
          meters.slotGrDb[s] = tightGate_[s].gainReductionDb();
          break;
        case ModuleId::PrecisionDrive:
          meters.slotGrDb[s] = precisionDrive_[s].gainReductionDb();
          break;
        default:
          break;
      }
    }
  }

  const stomp::modules::Tuner& tuner() const noexcept { return tuner_; }

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
    for (auto& p : pitchDrop_) p.reset();
    for (auto& p : retune_) p.reset();
    for (auto& p : chorus_) p.reset();
    for (auto& p : flanger_) p.reset();
    for (auto& p : phaser_) p.reset();
    for (auto& p : tremolo_) p.reset();
    for (auto& p : analogDelay_) p.reset();
    for (auto& p : springReverb_) p.reset();
    for (auto& p : bassChorus_) p.reset();
    for (auto& p : vibrato_) p.reset();
    for (auto& p : octave_) p.reset();
    for (auto& p : tapeEcho_) p.reset();
    for (auto& p : digitalDelay_) p.reset();
    for (auto& p : pingPong_) p.reset();
    for (auto& p : plateReverb_) p.reset();
    for (auto& p : roomReverb_) p.reset();
    for (auto& p : fetComp_) p.reset();
    for (auto& p : optoComp_) p.reset();
    for (auto& p : envFilter_) p.reset();
    for (auto& p : noiseGate_) p.reset();
    for (auto& p : tightGate_) p.reset();
    for (auto& p : eq3_) p.reset();
    for (auto& p : bassEq_) p.reset();
    for (auto& p : loFi_) p.reset();
    for (auto& p : tapeSat_) p.reset();
    for (auto& p : precisionDrive_) p.reset();
    for (auto& p : wah_) p.reset();
    for (auto& p : laser_) p.reset();
    for (auto& p : cab_) p.reset();
    input_ = inputTarget_;
    output_ = outputTarget_;
    power_ = powerTarget_;
  }

  int latencySamples() const noexcept {
    int latency = 0;
    for (int s = 0; s < kSlotCount; ++s) {
      const SlotControls& c = slot_[s];
      if (c.bypass) continue;
      switch (static_cast<ModuleId>(c.type)) {
        case ModuleId::PitchDrop:
          latency += pitchDrop_[s].latencySamples();
          break;
        case ModuleId::Retune:
          latency += retune_[s].latencySamples();
          break;
        case ModuleId::Laser:
          latency += laser_[s].latencySamples();
          break;
        default:
          break;
      }
    }
    return latency;
  }

private:
  void prepareSlot(int s, int type) noexcept {
    switch (static_cast<ModuleId>(type)) {
      case ModuleId::Boost: boost_[s].prepare(sr_); break;
      case ModuleId::Drive808: drive_[s].prepare(sr_); break;
      case ModuleId::TrebleBoost: treble_[s].prepare(sr_); break;
      case ModuleId::Overdrive: overdrive_[s].prepare(sr_); break;
      case ModuleId::BluesDrive: blues_[s].prepare(sr_); break;
      case ModuleId::Distortion: distortion_[s].prepare(sr_); break;
      case ModuleId::Rat: rat_[s].prepare(sr_); break;
      case ModuleId::BigFuzz: bigFuzz_[s].prepare(sr_); break;
      case ModuleId::FuzzFace: fuzzFace_[s].prepare(sr_); break;
      case ModuleId::ToneBender: toneBender_[s].prepare(sr_); break;
      case ModuleId::BassDrive: bassDrive_[s].prepare(sr_); break;
      case ModuleId::BassFuzz: bassFuzz_[s].prepare(sr_); break;
      case ModuleId::PitchDrop: pitchDrop_[s].prepare(sr_); break;
      case ModuleId::Retune: retune_[s].prepare(sr_); break;
      case ModuleId::Chorus: chorus_[s].prepare(sr_); break;
      case ModuleId::Flanger: flanger_[s].prepare(sr_); break;
      case ModuleId::Phaser: phaser_[s].prepare(sr_); break;
      case ModuleId::Tremolo: tremolo_[s].prepare(sr_); break;
      case ModuleId::AnalogDelay: analogDelay_[s].prepare(sr_); break;
      case ModuleId::SpringReverb: springReverb_[s].prepare(sr_); break;
      case ModuleId::BassChorus: bassChorus_[s].prepare(sr_); break;
      case ModuleId::Vibrato: vibrato_[s].prepare(sr_); break;
      case ModuleId::Octave: octave_[s].prepare(sr_); break;
      case ModuleId::TapeEcho: tapeEcho_[s].prepare(sr_); break;
      case ModuleId::DigitalDelay: digitalDelay_[s].prepare(sr_); break;
      case ModuleId::PingPongDelay: pingPong_[s].prepare(sr_); break;
      case ModuleId::PlateReverb: plateReverb_[s].prepare(sr_); break;
      case ModuleId::RoomReverb: roomReverb_[s].prepare(sr_); break;
      case ModuleId::FetComp: fetComp_[s].prepare(sr_); break;
      case ModuleId::OptoComp: optoComp_[s].prepare(sr_); break;
      case ModuleId::EnvFilter: envFilter_[s].prepare(sr_); break;
      case ModuleId::NoiseGate: noiseGate_[s].prepare(sr_); break;
      case ModuleId::TightGate: tightGate_[s].prepare(sr_); break;
      case ModuleId::Eq3: eq3_[s].prepare(sr_, 120.0f, 800.0f, 4000.0f); break;
      case ModuleId::BassEq: bassEq_[s].prepare(sr_); break;
      case ModuleId::LoFi: loFi_[s].prepare(sr_); break;
      case ModuleId::TapeSat: tapeSat_[s].prepare(sr_); break;
      case ModuleId::PrecisionDrive: precisionDrive_[s].prepare(sr_); break;
      case ModuleId::Wah: wah_[s].prepare(sr_); break;
      case ModuleId::Laser: laser_[s].prepare(sr_); break;
      case ModuleId::Cab: cab_[s].prepare(sr_); break;
      default: break;
    }
  }

  void setSlotControls(int s, int type, const SlotControls& c) noexcept {
    switch (static_cast<ModuleId>(type)) {
      case ModuleId::Boost: boost_[s].setControls(boostControlsFrom(c)); break;
      case ModuleId::Drive808: drive_[s].setControls(drive808ControlsFrom(c)); break;
      case ModuleId::TrebleBoost: treble_[s].setControls(trebleBoostControlsFrom(c)); break;
      case ModuleId::Overdrive: overdrive_[s].setControls(overdriveControlsFrom(c)); break;
      case ModuleId::BluesDrive: blues_[s].setControls(bluesDriveControlsFrom(c)); break;
      case ModuleId::Distortion: distortion_[s].setControls(distortionControlsFrom(c)); break;
      case ModuleId::Rat: rat_[s].setControls(ratControlsFrom(c)); break;
      case ModuleId::BigFuzz: bigFuzz_[s].setControls(bigFuzzControlsFrom(c)); break;
      case ModuleId::FuzzFace: fuzzFace_[s].setControls(fuzzFaceControlsFrom(c)); break;
      case ModuleId::ToneBender: toneBender_[s].setControls(toneBenderControlsFrom(c)); break;
      case ModuleId::BassDrive: bassDrive_[s].setControls(bassDriveControlsFrom(c)); break;
      case ModuleId::BassFuzz: bassFuzz_[s].setControls(bassFuzzControlsFrom(c)); break;
      case ModuleId::PitchDrop: pitchDrop_[s].setControls(pitchDropControlsFrom(c)); break;
      case ModuleId::Retune: retune_[s].setControls(retuneControlsFrom(c)); break;
      case ModuleId::Chorus: chorus_[s].setControls(chorusControlsFrom(c)); break;
      case ModuleId::Flanger: flanger_[s].setControls(flangerControlsFrom(c)); break;
      case ModuleId::Phaser: phaser_[s].setControls(phaserControlsFrom(c)); break;
      case ModuleId::Tremolo: tremolo_[s].setControls(tremoloControlsFrom(c)); break;
      case ModuleId::AnalogDelay: analogDelay_[s].setControls(analogDelayControlsFrom(c)); break;
      case ModuleId::SpringReverb: springReverb_[s].setControls(springReverbControlsFrom(c)); break;
      case ModuleId::BassChorus: bassChorus_[s].setControls(bassChorusControlsFrom(c)); break;
      case ModuleId::Vibrato: vibrato_[s].setControls(vibratoControlsFrom(c)); break;
      case ModuleId::Octave: octave_[s].setControls(octaveControlsFrom(c)); break;
      case ModuleId::TapeEcho: tapeEcho_[s].setControls(tapeEchoControlsFrom(c)); break;
      case ModuleId::DigitalDelay: digitalDelay_[s].setControls(digitalDelayControlsFrom(c)); break;
      case ModuleId::PingPongDelay: pingPong_[s].setControls(pingPongControlsFrom(c)); break;
      case ModuleId::PlateReverb: plateReverb_[s].setControls(plateReverbControlsFrom(c)); break;
      case ModuleId::RoomReverb: roomReverb_[s].setControls(roomReverbControlsFrom(c)); break;
      case ModuleId::FetComp: fetComp_[s].setControls(fetCompControlsFrom(c)); break;
      case ModuleId::OptoComp: optoComp_[s].setControls(optoCompControlsFrom(c)); break;
      case ModuleId::EnvFilter: envFilter_[s].setControls(envFilterControlsFrom(c)); break;
      case ModuleId::NoiseGate: noiseGate_[s].setControls(gateControlsFrom(c)); break;
      case ModuleId::TightGate: tightGate_[s].setControls(tightGateControlsFrom(c)); break;
      case ModuleId::Eq3: eq3_[s].setControls(eq3ControlsFrom(c)); break;
      case ModuleId::BassEq: bassEq_[s].setControls(eq3ControlsFrom(c)); break;
      case ModuleId::LoFi: loFi_[s].setControls(loFiControlsFrom(c)); break;
      case ModuleId::TapeSat: tapeSat_[s].setControls(tapeSatControlsFrom(c)); break;
      case ModuleId::PrecisionDrive: precisionDrive_[s].setControls(precisionDriveControlsFrom(c)); break;
      case ModuleId::Wah: wah_[s].setControls(wahControlsFrom(c)); break;
      case ModuleId::Laser: laser_[s].setControls(laserControlsFrom(c)); break;
      case ModuleId::Cab: cab_[s].setControls(cabControlsFrom(c)); break;
      default: break;
    }
  }

  inline void processSlot(int s, ModuleId id, float& l, float& r) noexcept {
    switch (id) {
      case ModuleId::Boost: boost_[s].process(l, r); break;
      case ModuleId::Drive808: drive_[s].process(l, r); break;
      case ModuleId::TrebleBoost: treble_[s].process(l, r); break;
      case ModuleId::Overdrive: overdrive_[s].process(l, r); break;
      case ModuleId::BluesDrive: blues_[s].process(l, r); break;
      case ModuleId::Distortion: distortion_[s].process(l, r); break;
      case ModuleId::Rat: rat_[s].process(l, r); break;
      case ModuleId::BigFuzz: bigFuzz_[s].process(l, r); break;
      case ModuleId::FuzzFace: fuzzFace_[s].process(l, r); break;
      case ModuleId::ToneBender: toneBender_[s].process(l, r); break;
      case ModuleId::BassDrive: bassDrive_[s].process(l, r); break;
      case ModuleId::BassFuzz: bassFuzz_[s].process(l, r); break;
      case ModuleId::PitchDrop: pitchDrop_[s].process(l, r); break;
      case ModuleId::Retune: retune_[s].process(l, r); break;
      case ModuleId::Chorus: chorus_[s].process(l, r); break;
      case ModuleId::Flanger: flanger_[s].process(l, r); break;
      case ModuleId::Phaser: phaser_[s].process(l, r); break;
      case ModuleId::Tremolo: tremolo_[s].process(l, r); break;
      case ModuleId::AnalogDelay: analogDelay_[s].process(l, r); break;
      case ModuleId::SpringReverb: springReverb_[s].process(l, r); break;
      case ModuleId::BassChorus: bassChorus_[s].process(l, r); break;
      case ModuleId::Vibrato: vibrato_[s].process(l, r); break;
      case ModuleId::Octave: octave_[s].process(l, r); break;
      case ModuleId::TapeEcho: tapeEcho_[s].process(l, r); break;
      case ModuleId::DigitalDelay: digitalDelay_[s].process(l, r); break;
      case ModuleId::PingPongDelay: pingPong_[s].process(l, r); break;
      case ModuleId::PlateReverb: plateReverb_[s].process(l, r); break;
      case ModuleId::RoomReverb: roomReverb_[s].process(l, r); break;
      case ModuleId::FetComp: fetComp_[s].process(l, r); break;
      case ModuleId::OptoComp: optoComp_[s].process(l, r); break;
      case ModuleId::EnvFilter: envFilter_[s].process(l, r); break;
      case ModuleId::NoiseGate: noiseGate_[s].process(l, r); break;
      case ModuleId::TightGate: tightGate_[s].process(l, r); break;
      case ModuleId::Eq3: eq3_[s].process(l, r); break;
      case ModuleId::BassEq: bassEq_[s].process(l, r); break;
      case ModuleId::LoFi: loFi_[s].process(l, r); break;
      case ModuleId::TapeSat: tapeSat_[s].process(l, r); break;
      case ModuleId::PrecisionDrive: precisionDrive_[s].process(l, r); break;
      case ModuleId::Wah: wah_[s].process(l, r); break;
      case ModuleId::Laser: laser_[s].process(l, r); break;
      case ModuleId::Cab: cab_[s].process(l, r); break;
      default: break;
    }
  }

private:
  float sr_ = 48000.0f;
  float smooth_ = 0.01f;
  float input_ = 1.0f, output_ = 1.0f, power_ = 1.0f;
  float inputTarget_ = 1.0f, outputTarget_ = 1.0f, powerTarget_ = 1.0f;
  SlotControls slot_[kSlotCount];
  int preparedType_[kSlotCount] = {-1, -1, -1, -1, -1, -1, -1, -1};
  uint64_t revision_ = 0;
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
  stomp::modules::PitchDrop pitchDrop_[kSlotCount];
  stomp::modules::Retune retune_[kSlotCount];
  stomp::modules::Chorus chorus_[kSlotCount];
  stomp::modules::Flanger flanger_[kSlotCount];
  stomp::modules::Phaser phaser_[kSlotCount];
  stomp::modules::Tremolo tremolo_[kSlotCount];
  stomp::modules::AnalogDelay analogDelay_[kSlotCount];
  stomp::modules::SpringReverb springReverb_[kSlotCount];
  stomp::modules::BassChorus bassChorus_[kSlotCount];
  stomp::modules::Vibrato vibrato_[kSlotCount];
  stomp::modules::Octave octave_[kSlotCount];
  stomp::modules::TapeEcho tapeEcho_[kSlotCount];
  stomp::modules::DigitalDelay digitalDelay_[kSlotCount];
  stomp::modules::PingPongDelay pingPong_[kSlotCount];
  stomp::modules::PlateReverb plateReverb_[kSlotCount];
  stomp::modules::RoomReverb roomReverb_[kSlotCount];
  stomp::modules::FetComp fetComp_[kSlotCount];
  stomp::modules::OptoComp optoComp_[kSlotCount];
  stomp::modules::EnvFilter envFilter_[kSlotCount];
  stomp::modules::NoiseGate noiseGate_[kSlotCount];
  stomp::modules::TightGate tightGate_[kSlotCount];
  stomp::modules::Eq3 eq3_[kSlotCount];
  stomp::modules::BassEq bassEq_[kSlotCount];
  stomp::modules::LoFi loFi_[kSlotCount];
  stomp::modules::TapeSat tapeSat_[kSlotCount];
  stomp::modules::PrecisionDrive precisionDrive_[kSlotCount];
  stomp::modules::Wah wah_[kSlotCount];
  stomp::modules::Laser laser_[kSlotCount];
  stomp::modules::Cabinet cab_[kSlotCount];
  stomp::modules::Tuner tuner_;
};

} // namespace pedal
