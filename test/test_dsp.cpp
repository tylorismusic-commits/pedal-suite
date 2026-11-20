// Pedal Suite — DSP tests
// SPDX-License-Identifier: GPL-3.0-or-later
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <random>
#include <utility>
#include <vector>

#include "dsp/engine.hpp"
#include "module_ids.hpp"
#include "modules.hpp"
#include "params.hpp"
#include "presets.hpp"

using namespace pedal;

namespace {
int failures = 0;
void check(bool ok, const char* name) {
  std::printf("%s  %s\n", ok ? "PASS" : "FAIL", name);
  if (!ok) ++failures;
}
double rms(const std::vector<float>& x, int a, int b) {
  double s = 0.0;
  for (int i = a; i < b; ++i) s += double(x[i]) * x[i];
  return std::sqrt(s / std::max(1, b - a));
}
} // namespace

int main() {
  {
    bool audible = true;
    const int n = 24000;
    std::vector<float> in(n);
    for (int i = 0; i < n; ++i) {
      in[i] = 0.2f * std::sin(2.0f * 3.14159265f * 220.0f * i / 48000.0f);
    }
    for (int i = 0; i < kPresetCount; ++i) {
      Shared s;
      initializeRack(s);
      for (int slot = 0; slot < kSlotCount; ++slot) {
        applyModuleDefaults(s, slot, static_cast<int>(ModuleId::Off), false);
      }
      s.set(P_INPUT, 0.0);
      s.set(P_OUTPUT, 0.0);
      const Preset& p = kPresets[i];
      for (int v = 0; v < p.count; ++v) {
        s.set(static_cast<uint32_t>(p.values[v].param), p.values[v].value);
      }
      Engine e;
      e.prepare(48000.0f);
      e.setControls(controlsFrom(s));
      auto l = in, r = in;
      Meters m;
      e.process(l.data(), r.data(), n, m);
      if (rms(l, n / 2, n) < 1.0e-5) {
        audible = false;
        std::printf("preset '%s' is silent\n", p.name);
      }
      for (float v : l) {
        if (!std::isfinite(v)) {
          audible = false;
          std::printf("preset '%s' is not finite\n", p.name);
          break;
        }
      }
    }
    check(audible, "every factory board preset makes sound");
  }
#ifdef PEDAL_NAM_TEST_MODEL
  {
    Shared s;
    initializeRack(s);
    for (int slot = 0; slot < kSlotCount; ++slot) {
      applyModuleDefaults(s, slot, static_cast<int>(ModuleId::Off), false);
      applyModuleDefaults(s, slot, static_cast<int>(ModuleId::Nam), false);
    }
    Engine e;
    e.prepare(48000.0f);
    e.setControls(controlsFrom(s));
    e.setNamModel(PEDAL_NAM_TEST_MODEL);
    check(e.activeNamSlots() == kMaxNamSlots,
          "only two NAM blocks are live");
    check(e.namSlotActive(0) && e.namSlotActive(1) &&
              !e.namSlotActive(2),
          "third NAM block is limited");
    const int n = 24000;
    std::vector<float> l(n), r(n);
    for (int i = 0; i < n; ++i) {
      l[i] = r[i] = 0.2f * std::sin(2.0f * 3.14159265f * 220.0f * i /
                                    48000.0f);
    }
    Meters m;
    e.process(l.data(), r.data(), n, m);
    bool finiteAll = true;
    for (float v : l) if (!std::isfinite(v)) finiteAll = false;
    check(finiteAll, "eight NAM blocks stay finite");
    check(e.latencySamples() >= 128, "two NAM blocks report latency");
    // Removing the first NAM promotes the third slot.
    applyModuleDefaults(s, 0, static_cast<int>(ModuleId::Off), false);
    e.setControls(controlsFrom(s));
    check(e.activeNamSlots() == kMaxNamSlots && e.namSlotActive(2),
          "NAM limit follows the remaining blocks");
  }
#endif
  {
    bool stable = true;
    for (float sr : {48000.0f, 96000.0f}) {
      const int n = static_cast<int>(sr * 0.25f);
      std::vector<float> x(n);
      for (int i = 0; i < n; ++i) {
        x[i] = 0.3f * std::sin(2.0f * 3.14159265f * 220.0f * i / sr);
      }
      for (double extreme : {0.0, 1.0}) {
        for (int id = 1; id < kModuleCount; ++id) {
          Shared s;
          initializeRack(s);
          applyModuleDefaults(s, 0, id, false);
          for (int k = 0; k < kRackControlCount; ++k) {
            s.set(slotParam(0, kSlotOffsetC0 + k), extreme);
          }
          Engine e;
          e.prepare(sr);
          e.setControls(controlsFrom(s));
          auto l = x, r = x;
          Meters m;
          e.process(l.data(), r.data(), n, m);
          for (float v : l) {
            if (!std::isfinite(v)) {
              stable = false;
              std::printf("unstable module %d at %.0f Hz extreme %.0f\n",
                          id, sr, extreme);
              break;
            }
          }
        }
      }
    }
    check(stable, "every pedal stays finite at 48/96 kHz extremes");
  }
  {
    const int n = 24000;
    Shared s;
    initializeRack(s);
    applyModuleDefaults(s, 0, static_cast<int>(ModuleId::PlateReverb), false);
    s.set(slotParam(0, kSlotOffsetC0 + 0), 0.65);
    s.set(slotParam(0, kSlotOffsetC0 + 2), 1.0);
    Engine e;
    e.prepare(48000.0f);
    e.setControls(controlsFrom(s));
    std::vector<float> l(n), r(n);
    for (int i = 0; i < n; ++i) {
      l[i] = r[i] = 0.3f * std::sin(2.0f * 3.14159265f * 330.0f * i /
                                    48000.0f);
    }
    Meters m;
    e.process(l.data(), r.data(), n, m);
    s.set(slotParam(0, kSlotOffsetBypass), 1.0);
    e.setControls(controlsFrom(s));
    std::vector<float> tl(n, 0.0f), tr(n, 0.0f);
    e.process(tl.data(), tr.data(), n, m);
    check(rms(tl, 0, n) > 1.0e-4, "bypassed reverb lets its tail ring");

    Shared b;
    initializeRack(b);
    applyModuleDefaults(b, 0, static_cast<int>(ModuleId::Boost), false);
    b.set(slotParam(0, kSlotOffsetBypass), 1.0);
    Engine be;
    be.prepare(48000.0f);
    be.setControls(controlsFrom(b));
    std::vector<float> bl(n, 0.0f), br(n, 0.0f);
    be.process(bl.data(), br.data(), n, m);
    check(rms(bl, 0, n) < 1.0e-9, "bypassed pedal without tails is silent");
  }
  {
    Shared s;
    initializeRack(s);
    applyModuleDefaults(s, 0, static_cast<int>(ModuleId::FetComp), false);
    for (int k = 0; k < kRackControlCount; ++k) {
      s.set(slotParam(0, kSlotOffsetC0 + k), 0.9);
    }
    Engine e;
    e.prepare(48000.0f);
    e.setControls(controlsFrom(s));
    const int n = 24000;
    std::vector<float> l(n), r(n);
    for (int i = 0; i < n; ++i) {
      l[i] = r[i] = 0.4f * std::sin(2.0f * 3.14159265f * 220.0f * i /
                                    48000.0f);
    }
    Meters m;
    e.process(l.data(), r.data(), n, m);
    check(m.slotGrDb[0] > 1.0f, "FET Comp reports gain reduction");
    check(e.latencySamples() == 0, "compressor adds no latency");
  }
  for (float sr : {48000.0f, 96000.0f}) {
    const int n = static_cast<int>(sr * 0.25f);
    std::vector<float> x(n);
    for (int i = 0; i < n; ++i) {
      x[i] = 0.15f * std::sin(2.0f * 3.14159265f * 220.0f * i / sr);
    }
    {
      Shared s;
      initializeRack(s);
      for (int slot = 0; slot < kSlotCount; ++slot) {
        s.set(slotParam(slot, kSlotOffsetType), 0.0);
      }
      Engine e;
      e.prepare(sr);
      e.setControls(controlsFrom(s));
      auto l = x, r = x;
      Meters m;
      e.process(l.data(), r.data(), n, m);
      bool transparent = true;
      for (int i = 0; i < n; ++i) {
        if (std::fabs(l[i] - x[i]) > 1e-6f) transparent = false;
      }
      char label[64];
      std::snprintf(label, sizeof(label), "empty board is transparent at %.0f",
                    sr);
      check(transparent, label);
    }
    {
      Shared s;
      initializeRack(s);
      s.set(slotParam(0, kSlotOffsetC0 + 0), 1.0);
      Engine e;
      e.prepare(sr);
      e.setControls(controlsFrom(s));
      auto l = x, r = x;
      Meters m;
      e.process(l.data(), r.data(), n, m);
      char label[64];
      std::snprintf(label, sizeof(label), "Boost adds gain at %.0f", sr);
      check(rms(l, 1000, n) > rms(x, 1000, n) * 2.0, label);
    }
    {
      Shared s;
      initializeRack(s);
      s.set(slotParam(0, kSlotOffsetType), 0.0);
      s.set(slotParam(1, kSlotOffsetType),
            static_cast<double>(ModuleId::Drive808));
      s.set(slotParam(1, kSlotOffsetC0 + 0), 0.8);
      Engine e;
      e.prepare(sr);
      e.setControls(controlsFrom(s));
      auto l = x, r = x;
      Meters m;
      e.process(l.data(), r.data(), n, m);
      bool finite = true;
      for (float v : l) if (!std::isfinite(v)) finite = false;
      char label[64];
      std::snprintf(label, sizeof(label), "808 stays finite at %.0f", sr);
      check(finite, label);
      check(e.latencySamples() == 0, "pedalboard reports zero latency");
    }
  }
  {
    const int n = 24000;
    std::vector<float> x(n);
    for (int i = 0; i < n; ++i) {
      x[i] = 0.2f * std::sin(2.0f * 3.14159265f * 220.0f * i / 48000.0f);
    }
    auto shifted = [&](ModuleId id, int control, double value) {
      Shared s;
      initializeRack(s);
      applyModuleDefaults(s, 0, static_cast<int>(id), false);
      s.set(slotParam(0, kSlotOffsetC0 + control), value);
      if (id == ModuleId::PitchDrop) {
        s.set(slotParam(0, kSlotOffsetC0 + 2), 0.0);
      }
      Engine e;
      e.prepare(48000.0f);
      e.setControls(controlsFrom(s));
      auto l = x, r = x;
      Meters m;
      e.process(l.data(), r.data(), n, m);
      const int latency = e.latencySamples();
      int crossings = 0;
      float prev = 0.0f;
      const int start = std::min(n - 1, latency + 2400);
      for (int i = start; i < n; ++i) {
        if ((prev <= 0.0f && l[i] > 0.0f) ||
            (prev >= 0.0f && l[i] < 0.0f)) {
          ++crossings;
        }
        prev = l[i];
      }
      const double seconds = (n - start) / 48000.0;
      return std::make_pair(crossings * 0.5 / seconds, latency);
    };
    const auto pitch = shifted(ModuleId::PitchDrop, 0, 0.5);
    check(std::fabs(pitch.first - 440.0) < 30.0,
          "Pitch Drop shifts up an octave");
    check(pitch.second > 0, "Pitch Drop reports latency");
    const auto retune = shifted(ModuleId::Retune, 0, 1.0);
    check(std::fabs(retune.first - 440.0) < 30.0,
          "Retune shifts up twelve semitones");
    check(retune.second > 0, "Retune reports latency");
  }
  {
    // A bypassed pitch pedal must keep its latency in the path, otherwise the
    // host re-aligns the track every time the footswitch is pressed.
    Shared s;
    initializeRack(s);
    applyModuleDefaults(s, 0, static_cast<int>(ModuleId::PitchDrop), false);
    Engine e;
    e.prepare(48000.0f);
    e.setControls(controlsFrom(s));
    const int activeLatency = e.latencySamples();
    s.set(slotParam(0, kSlotOffsetBypass), 1.0);
    e.setControls(controlsFrom(s));
    const int bypassLatency = e.latencySamples();
    check(activeLatency > 0 && activeLatency == bypassLatency,
          "pitch bypass keeps constant latency");
  }
  {
    // Changing a slot's gain must ramp, not step.  A one-sample jump is the
    // scratch users hear while turning a knob.
    Shared s;
    initializeRack(s);
    applyModuleDefaults(s, 0, static_cast<int>(ModuleId::Boost), false);
    s.set(slotParam(0, kSlotOffsetGain), 0.0);
    Engine e;
    e.prepare(48000.0f);
    e.setControls(controlsFrom(s));
    const int block = 4096;
    std::vector<float> l(block), r(block);
    for (int i = 0; i < block; ++i) {
      l[i] = r[i] = 0.2f * std::sin(2.0f * 3.14159265f * 1000.0f * i / 48000.0f);
    }
    Meters m;
    e.process(l.data(), r.data(), block, m);
    s.set(slotParam(0, kSlotOffsetGain), 12.0);
    e.setControls(controlsFrom(s));
    std::vector<float> l2 = l, r2 = r;
    for (int i = 0; i < block; ++i) {
      l2[i] = r2[i] = 0.2f * std::sin(2.0f * 3.14159265f * 1000.0f *
                                       (i + block) / 48000.0f);
    }
    e.process(l2.data(), r2.data(), block, m);
    float maxStep = 0.0f;
    for (int i = 1; i < 64; ++i) {
      maxStep = std::max(maxStep, std::fabs(l2[i] - l2[i - 1]));
    }
    check(maxStep < 0.12f, "slot gain changes are smoothed");
  }
  {
    // Changing pitch on a bypassed pitch pedal must not move its dry delay;
    // that was the scratch users heard with the pedal switched off.
    auto render = [&](bool changePitch, std::vector<float>& out) {
      Shared s;
      initializeRack(s);
      applyModuleDefaults(s, 0, static_cast<int>(ModuleId::PitchDrop), false);
      s.set(slotParam(0, kSlotOffsetBypass), 1.0);
      Engine e;
      e.prepare(48000.0f);
      e.setControls(controlsFrom(s));
      const int n = 8192;
      std::vector<float> l(n), r(n);
      for (int i = 0; i < n; ++i) {
        l[i] = r[i] = 0.2f * std::sin(2.0f * 3.14159265f * 220.0f * i / 48000.0f);
      }
      Meters m;
      e.process(l.data(), r.data(), 4096, m);
      if (changePitch) {
        s.set(slotParam(0, kSlotOffsetC0), 1.0);
        e.setControls(controlsFrom(s));
      }
      e.process(l.data() + 4096, r.data() + 4096, 4096, m);
      out = l;
    };
    std::vector<float> a, b;
    render(false, a);
    render(true, b);
    float diff = 0.0f;
    for (int i = 4096; i < 8192; ++i) diff = std::max(diff, std::fabs(a[i] - b[i]));
    check(diff < 1.0e-6f, "bypassed pitch controls do not scratch");
  }
  {
    const int n = 24000;
    std::vector<float> x(n);
    for (int i = 0; i < n; ++i) {
      x[i] = 0.2f * std::sin(2.0f * 3.14159265f * 220.0f * i / 48000.0f);
    }
    auto run = [&](ModuleId id, int control, double value) {
      Shared s;
      initializeRack(s);
      applyModuleDefaults(s, 0, static_cast<int>(id), false);
      s.set(slotParam(0, kSlotOffsetC0 + control), value);
      Engine e;
      e.prepare(48000.0f);
      e.setControls(controlsFrom(s));
      auto l = x, r = x;
      Meters m;
      e.process(l.data(), r.data(), n, m);
      return rms(l, n / 2, n);
    };
    check(run(ModuleId::Tremolo, 1, 1.0) <
              run(ModuleId::Tremolo, 1, 0.0) * 0.7,
          "Tremolo depth modulates level");
    const double dry = rms(x, n / 2, n);
    check(std::fabs(run(ModuleId::Chorus, 3, 0.5) - dry) > dry * 0.02,
          "Chorus is audible");
    check(std::fabs(run(ModuleId::Flanger, 3, 0.5) - dry) > dry * 0.02,
          "Flanger is audible");
    check(std::fabs(run(ModuleId::Phaser, 3, 0.5) - dry) > dry * 0.02,
          "Phaser is audible");
    check(std::fabs(run(ModuleId::BassChorus, 3, 0.5) - dry) > dry * 0.02,
          "Bass Chorus is audible");
    check(std::fabs(run(ModuleId::Vibrato, 2, 1.0) - dry) > dry * 0.02,
          "Vibrato is audible");
    check(std::fabs(run(ModuleId::Octave, 0, 0.9) - dry) > dry * 0.02,
          "Octave is audible");
    check(run(ModuleId::FetComp, 0, 1.0) < dry * 0.9,
          "FET Comp reduces level");
    check(std::fabs(run(ModuleId::Eq3, 0, 0.9) - dry) > dry * 0.02,
          "3-Band EQ is audible");
    check(std::fabs(run(ModuleId::LoFi, 3, 1.0) - dry) > dry * 0.02,
          "Lo-Fi is audible");
    check(std::fabs(run(ModuleId::TapeSat, 0, 0.8) - dry) > dry * 0.02,
          "Tape Sat is audible");
    auto gateTail = [&](ModuleId id, bool tight) {
      Shared s;
      initializeRack(s);
      applyModuleDefaults(s, 0, static_cast<int>(id), false);
      s.set(slotParam(0, kSlotOffsetC0 + 0), 0.95);
      s.set(slotParam(0, kSlotOffsetC0 + 1), 0.05);
      Engine e;
      e.prepare(48000.0f);
      e.setControls(controlsFrom(s));
      auto l = x, r = x;
      Meters m;
      e.process(l.data(), r.data(), n, m);
      (void)tight;
      return rms(l, n * 3 / 4, n);
    };
    check(gateTail(ModuleId::NoiseGate, false) < dry * 0.3,
          "Noise Gate closes");
    check(gateTail(ModuleId::TightGate, true) < dry * 0.3,
          "Tight Gate closes");
  }
  {
    const int n = 48000;
    auto tail = [&](ModuleId id, int mixControl) {
      Shared s;
      initializeRack(s);
      applyModuleDefaults(s, 0, static_cast<int>(id), false);
      s.set(slotParam(0, kSlotOffsetC0 + mixControl), 1.0);
      Engine e;
      e.prepare(48000.0f);
      e.setControls(controlsFrom(s));
      std::vector<float> l(n, 0.0f), r(n, 0.0f);
      for (int i = 0; i < 2400; ++i) {
        l[i] = r[i] = 0.3f * std::sin(2.0f * 3.14159265f * 330.0f * i /
                                      48000.0f);
      }
      Meters m;
      e.process(l.data(), r.data(), n, m);
      return rms(l, 9600, n);
    };
    check(tail(ModuleId::AnalogDelay, 3) > 1.0e-4,
          "Analog Delay produces a tail");
    check(tail(ModuleId::SpringReverb, 2) > 1.0e-4,
          "Spring Reverb produces a tail");
    check(tail(ModuleId::TapeEcho, 4) > 1.0e-4,
          "Tape Echo produces a tail");
    check(tail(ModuleId::DigitalDelay, 3) > 1.0e-4,
          "Digital Delay produces a tail");
    check(tail(ModuleId::PingPongDelay, 3) > 1.0e-4,
          "Ping-Pong Delay produces a tail");
    check(tail(ModuleId::PlateReverb, 2) > 1.0e-4,
          "Plate Reverb produces a tail");
    check(tail(ModuleId::RoomReverb, 2) > 1.0e-4,
          "Room Reverb produces a tail");
  }
  {
    const int n = 12000;
    std::vector<float> x(n);
    for (int i = 0; i < n; ++i) {
      x[i] = 0.2f * std::sin(2.0f * 3.14159265f * 330.0f * i / 48000.0f);
    }
    bool allFinite = true;
    bool allAudible = true;
    for (int id = 1; id < kModuleCount; ++id) {
      Shared s;
      initializeRack(s);
      for (int slot = 0; slot < kSlotCount; ++slot) {
        s.set(slotParam(slot, kSlotOffsetType), 0.0);
      }
      s.set(slotParam(0, kSlotOffsetType), static_cast<double>(id));
      for (int k = 0; k < kRackControlCount; ++k) {
        s.set(slotParam(0, kSlotOffsetC0 + k),
              defaultControlValue(static_cast<ModuleId>(id), k));
      }
      Engine e;
      e.prepare(48000.0f);
      e.setControls(controlsFrom(s));
      auto l = x, r = x;
      Meters m;
      e.process(l.data(), r.data(), n, m);
      for (float v : l) if (!std::isfinite(v)) allFinite = false;
      if (rms(l, 1000, n) < 1.0e-6) {
        allAudible = false;
        std::printf("module %d produced silence\n", id);
      }
    }
    check(allFinite, "every pedal module stays finite");
    check(allAudible, "every pedal module produces sound");
  }
  {
    Shared s;
    initializeRack(s);
    for (int slot = 0; slot < kSlotCount; ++slot) {
      applyModuleDefaults(s, slot, static_cast<int>(ModuleId::Off), false);
    }
    applyModuleDefaults(s, 0, static_cast<int>(ModuleId::Cab), false);
    s.set(slotParam(0, kSlotOffsetC0 + 0), 0.0);
    s.set(slotParam(0, kSlotOffsetC0 + 1), 1.0);
    s.set(slotParam(0, kSlotOffsetC0 + 2), 0.5);
    s.set(slotParam(0, kSlotOffsetC0 + 3), 1.0);
    Engine e;
    e.prepare(48000.0f);
    e.setControls(controlsFrom(s));
    std::vector<float> ir(1, 1.0f);
    e.setCabIr(ir.data(), 1);
    const int n = 24000;
    std::vector<float> in(n), r(n);
    for (int i = 0; i < n; ++i) {
      in[i] = 0.2f * std::sin(2.0f * 3.14159265f * 220.0f * i / 48000.0f);
    }
    auto out = in;
    Meters m;
    e.process(out.data(), r.data(), n, m);
    check(std::fabs(rms(out, 1000, n) / rms(in, 1000, n) - 1.0) < 0.1,
          "Cab with a unit IR is transparent");
  }
  {
    Shared s;
    initializeRack(s);
    for (int slot = 0; slot < kSlotCount; ++slot) {
      applyModuleDefaults(s, slot, static_cast<int>(ModuleId::Off), false);
    }
    Engine e;
    e.prepare(48000.0f);
    e.setControls(controlsFrom(s));
    const int n = 24000;
    std::vector<float> l(n), r(n);
    Meters m;
    auto runTone = [&] {
      for (int i = 0; i < n; ++i) {
        l[i] = r[i] = 0.2f * std::sin(2.0f * 3.14159265f * 220.0f * i /
                                      48000.0f);
      }
      e.process(l.data(), r.data(), n, m);
    };
    runTone();
    applyModuleDefaults(s, 0, static_cast<int>(ModuleId::DigitalDelay));
    s.set(slotParam(0, kSlotOffsetC0 + 3), 1.0);
    e.setControls(controlsFrom(s));
    runTone();
    const bool delayAudible = rms(l, 4800, n) > 1.0e-5;
    applyModuleDefaults(s, 0, static_cast<int>(ModuleId::Wah));
    e.setControls(controlsFrom(s));
    runTone();
    const bool wahAudible = rms(l, 1000, n) > 1.0e-5;
    check(delayAudible && wahAudible, "slot type changes prepare new modules");
  }

  {
    Shared s;
    initializeRack(s);
    for (int slot = 0; slot < kSlotCount; ++slot) {
      applyModuleDefaults(s, slot, static_cast<int>(ModuleId::Cab), false);
    }
    Engine e;
    e.prepare(48000.0f);
    e.setControls(controlsFrom(s));
    std::vector<float> ir(2, 1.0f);
    e.setCabIr(ir.data(), static_cast<int>(ir.size()));
    check(e.activeCabSlots() == kMaxCabSlots,
          "only two cab blocks are live");
    check(e.cabSlotActive(0) && e.cabSlotActive(1) && !e.cabSlotActive(2),
          "third cab block is limited");
    const int n = 4800;
    std::vector<float> l(n), r(n);
    for (int i = 0; i < n; ++i) {
      l[i] = r[i] = 0.2f * std::sin(2.0f * 3.14159265f * 220.0f * i /
                                    48000.0f);
    }
    Meters m;
    e.process(l.data(), r.data(), n, m);
    bool finiteAll = true;
    float peak = 0.0f;
    for (float v : l) {
      if (!std::isfinite(v)) finiteAll = false;
      peak = std::max(peak, std::fabs(v));
    }
    check(finiteAll && peak < 10.0f, "eight cab blocks stay bounded");
    s.set(slotParam(0, kSlotOffsetBypass), 1.0);
    e.setControls(controlsFrom(s));
    check(e.activeCabSlots() == kMaxCabSlots && e.cabSlotActive(2),
          "cab limit follows bypassed blocks");
  }
  {
    // Every ordered pair of pedal modules, short burst, finite and bounded.
    bool finitePairs = true;
    bool boundedPairs = true;
    float worst = 0.0f;
    int worstA = 0, worstB = 0;
    const int n = 1200;
    for (int a = 1; a < kModuleCount; ++a) {
      for (int b = 1; b < kModuleCount; ++b) {
        Shared s;
        initializeRack(s);
        for (int slot = 0; slot < kSlotCount; ++slot) {
          applyModuleDefaults(s, slot, static_cast<int>(ModuleId::Off), false);
        }
        applyModuleDefaults(s, 0, a, false);
        applyModuleDefaults(s, 1, b, false);
        Engine e;
        e.prepare(48000.0f);
        e.setControls(controlsFrom(s));
        std::vector<float> l(n), r(n);
        for (int i = 0; i < n; ++i) {
          const float env = i < n / 2 ? 1.0f : 0.4f;
          l[i] = r[i] = 0.25f * env *
                        std::sin(2.0f * 3.14159265f * 220.0f * i / 48000.0f);
        }
        Meters m;
        e.process(l.data(), r.data(), n, m);
        for (float v : l) {
          if (!std::isfinite(v)) finitePairs = false;
          if (std::fabs(v) > worst) {
            worst = std::fabs(v);
            worstA = a;
            worstB = b;
          }
        }
      }
    }
    if (worst >= 50.0f) boundedPairs = false;
    std::printf("pair sweep worst peak %.2f at %d/%d\n", worst, worstA,
                worstB);
    check(finitePairs, "every ordered pedal pair stays finite");
    check(boundedPairs, "every ordered pedal pair stays bounded");
  }
  {
    // Random eight-slot boards, including bypassed slots.
    std::mt19937 rng(0xC0FFEE);
    std::uniform_real_distribution<float> u01(0.0f, 1.0f);
    bool finiteBoards = true;
    bool boundedBoards = true;
    float worst = 0.0f;
    const int n = 4800;
    for (int board = 0; board < 200; ++board) {
      Shared s;
      initializeRack(s);
      for (int slot = 0; slot < kSlotCount; ++slot) {
        const int type = 1 + static_cast<int>(rng() % (kModuleCount - 1));
        applyModuleDefaults(s, slot, type, false);
        for (int k = 0; k < kRackControlCount; ++k) {
          s.setFromHost(slotParam(slot, kSlotOffsetC0 + k), u01(rng));
        }
        s.setFromHost(slotParam(slot, kSlotOffsetMix), 100.0 * u01(rng));
        s.setFromHost(slotParam(slot, kSlotOffsetGain),
                      -6.0 + 12.0 * u01(rng));
        s.setFromHost(slotParam(slot, kSlotOffsetBypass),
                      (board % 5 == 0 && (slot & 1)) ? 1.0 : 0.0);
      }
      Engine e;
      e.prepare(48000.0f);
      e.setControls(controlsFrom(s));
      std::vector<float> l(n), r(n);
      for (int i = 0; i < n; ++i) {
        const float env = i < 600 ? static_cast<float>(i) / 600.0f : 1.0f;
        l[i] = r[i] = 0.15f * env *
                      std::sin(2.0f * 3.14159265f * 220.0f * i / 48000.0f);
      }
      Meters m;
      e.process(l.data(), r.data(), n, m);
      for (float v : l) {
        if (!std::isfinite(v)) finiteBoards = false;
        if (std::fabs(v) > worst) worst = std::fabs(v);
      }
      if (e.latencySamples() < 0 || e.latencySamples() > 48000) {
        boundedBoards = false;
      }
    }
    if (worst >= 500.0f) boundedBoards = false;
    std::printf("random boards worst peak %.2f\n", worst);
    check(finiteBoards, "random eight-slot boards stay finite");
    check(boundedBoards, "random boards stay bounded with sane latency");
  }
  {
    // Tempo sync: with a host tempo the delay time must follow the musical
    // division, and the SYNC switch must be able to turn that off.
    auto echoMs = [](bool syncOn) {
      pedal::Shared s;
      s.set(slotParam(0, kSlotOffsetType),
            static_cast<double>(pedal::ModuleId::DigitalDelay));
      s.set(slotParam(0, kSlotOffsetMix), 100.0);
      s.set(slotParam(0, kSlotOffsetC0), 0.4);   // TIME
      s.set(slotParam(0, kSlotOffsetC0 + 1), 0.0);   // FEEDBACK
      s.set(slotParam(0, kSlotOffsetC0 + 2), 50.0);  // TONE
      s.set(slotParam(0, kSlotOffsetC0 + 3), 100.0); // MIX
      s.set(pedal::P_SYNC, syncOn ? 1.0 : 0.0);
      s.bpm.store(120.0f, std::memory_order_relaxed);
      s.hostTempo.store(true, std::memory_order_relaxed);
      pedal::Engine e;
      e.prepare(48000.0f);
      e.setControls(pedal::controlsFrom(s));
      const int n = 48000;
      std::vector<float> l(n, 0.0f), r(n, 0.0f);
      l[0] = r[0] = 0.5f;
      pedal::Meters m;
      e.process(l.data(), r.data(), n, m);
      float peak = 0.0f;
      int at = 0;
      for (int i = 2400; i < n; ++i) {
        if (std::fabs(l[i]) > peak) {
          peak = std::fabs(l[i]);
          at = i;
        }
      }
      return at * 1000.0 / 48000.0;
    };
    const double freeMs = echoMs(false);
    const double syncMs = echoMs(true);
    std::printf("delay echo free %.0f ms, synced %.0f ms\n", freeMs, syncMs);
    check(std::fabs(freeMs - syncMs) > 20.0, "tempo sync changes the delay time");
  }
  std::printf("%d failure(s)\n", failures);
  return failures == 0 ? 0 : 1;
}
