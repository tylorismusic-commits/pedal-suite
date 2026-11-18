// Pedal Suite — DSP tests
// SPDX-License-Identifier: GPL-3.0-or-later
#include <algorithm>
#include <cmath>
#include <cstdio>
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

  std::printf("%d failure(s)\n", failures);
  return failures == 0 ? 0 : 1;
}
