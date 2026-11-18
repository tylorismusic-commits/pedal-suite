// Pedal Suite — DSP tests
// SPDX-License-Identifier: GPL-3.0-or-later
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

#include "dsp/engine.hpp"
#include "module_ids.hpp"
#include "modules.hpp"
#include "params.hpp"

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
    const int n = 12000;
    std::vector<float> x(n);
    for (int i = 0; i < n; ++i) {
      x[i] = 0.2f * std::sin(2.0f * 3.14159265f * 330.0f * i / 48000.0f);
    }
    bool allFinite = true;
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
    }
    check(allFinite, "every pedal module stays finite");
  }

  std::printf("%d failure(s)\n", failures);
  return failures == 0 ? 0 : 1;
}
