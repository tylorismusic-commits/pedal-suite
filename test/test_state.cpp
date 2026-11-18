// Pedal Suite — state tests
// SPDX-License-Identifier: GPL-3.0-or-later
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <vector>

#include "midi_map.hpp"
#include "module_ids.hpp"
#include "modules.hpp"
#include "params.hpp"
#include "presets.hpp"
#include "dsp/ir_bank.hpp"
#include "factory_module_presets.hpp"
#include "state.hpp"
#include "value_text.hpp"

using namespace pedal;

namespace {
int failures = 0;
void check(bool ok, const char* name) {
  std::printf("%s  %s\n", ok ? "PASS" : "FAIL", name);
  if (!ok) ++failures;
}
} // namespace

int main() {
  Shared a;
  initializeRack(a);
  a.set(slotParam(0, kSlotOffsetC0), 0.42);
  a.set(slotParam(7, kSlotOffsetType), static_cast<double>(ModuleId::Boost));
  a.set(slotParam(7, kSlotOffsetMix), 73.0);
  a.set(P_OUTPUT, -3.5);

  const auto blob = serializeState(a, 1.25);
  Shared b;
  double scale = 0.0;
  check(deserializeState(blob.data(), blob.size(), b, &scale),
        "state round-trip decodes");
  check(std::fabs(scale - 1.25) < 1e-9, "state restores UI scale");
  check(std::fabs(b.get(slotParam(0, kSlotOffsetC0)) - 0.42) < 1e-9,
        "state restores pedal control");
  check(std::fabs(b.get(P_OUTPUT) + 3.5) < 1e-9,
        "state restores global output");
  {
    Shared s;
    initializeRack(s);
    bool ok = true;
    for (int id = 1; id < kModuleCount; ++id) {
      const ModuleDesc* m = moduleDesc(id);
      applyModuleDefaults(s, 0, id, false);
      for (int k = 0; k < m->controlCount; ++k) {
        const uint32_t p =
            slotParam(0, kSlotOffsetC0 + m->controls[k].param);
        for (double v : {0.0, 0.25, 0.5, 0.75, 1.0}) {
          const std::string text = formatParamValue(s, p, v);
          double out = -1.0;
          if (!parseParamValue(s, p, text, &out)) {
            ok = false;
            std::printf("parse failed: %s / %s -> '%s'\n", m->name,
                        m->controls[k].name, text.c_str());
            continue;
          }
          if (std::fabs(out - v) > 0.04) {
            ok = false;
            std::printf("round-trip drift: %s / %s %.3f -> '%s' -> %.3f\n",
                        m->name, m->controls[k].name, v, text.c_str(), out);
          }
        }
      }
    }
    check(ok, "every typed value round-trips");
  }
  {
    bool ok = true;
    for (int i = 0; i < kFactoryModulePresetCount; ++i) {
      const auto& p = kFactoryModulePresets[i];
      const ModuleDesc* m = moduleDesc(static_cast<int>(p.type));
      if (!m || m->id == ModuleId::Off) {
        ok = false;
        std::printf("preset '%s' has no module\n", p.name);
        continue;
      }
      for (int k = 0; k < m->controlCount; ++k) {
        if (p.controls[static_cast<size_t>(k)] < -0.001f ||
            p.controls[static_cast<size_t>(k)] > 1.001f) {
          ok = false;
          std::printf("preset '%s' control %d out of range\n", p.name, k);
        }
      }
    }
    check(ok, "factory pedal presets are valid");
  }
  {
    bool ok = true;
    for (int i = 0; i < kSlotCount; ++i) {
      if (footswitchSlotForNote(36 + i) != i) ok = false;
      if (footswitchSlotForNote(60 + i) != i) ok = false;
      if (footswitchSlotForCc(20 + i) != i) ok = false;
      if (footswitchSlotForCc(80 + i) != i) ok = false;
    }
    if (footswitchSlotForNote(35) != -1) ok = false;
    if (footswitchSlotForNote(44) != -1) ok = false;
    if (footswitchSlotForCc(19) != -1) ok = false;
    if (footswitchSlotForCc(28) != -1) ok = false;
    check(ok, "MIDI footswitch mapping covers eight slots");
    Shared m;
    initializeRack(m);
    toggleSlotBypass(m, 3);
    const bool on = m.get(slotParam(3, kSlotOffsetBypass)) > 0.5;
    toggleSlotBypass(m, 3);
    const bool off = m.get(slotParam(3, kSlotOffsetBypass)) <= 0.5;
    check(on && off, "MIDI footswitch toggles bypass");
    Shared w;
    initializeRack(w);
    applyModuleDefaults(w, 5, static_cast<int>(ModuleId::Wah), false);
    setFirstWahPedal(w, 0.8f);
    check(std::fabs(w.get(slotParam(5, kSlotOffsetC0 + 0)) - 0.8) < 1e-6,
          "MIDI expression moves the Wah pedal");
  }
  {
    bool ok = true;
    for (int i = 0; i < kPresetCount; ++i) {
      const Preset& p = kPresets[i];
      int types = 0;
      for (int v = 0; v < p.count; ++v) {
        const PresetValue& value = p.values[v];
        if (value.param < 0 || value.param >= static_cast<int>(P_COUNT)) {
          ok = false;
          std::printf("preset '%s' has invalid parameter %d\n", p.name,
                      value.param);
          continue;
        }
        const ParamSpec& spec = kParamSpecs[value.param];
        if (value.value < spec.min - 1e-9 || value.value > spec.max + 1e-9) {
          ok = false;
          std::printf("preset '%s' value out of range at %d\n", p.name,
                      value.param);
        }
        if (value.param >= P_SLOT0_TYPE) {
          const int offset =
              (value.param - P_SLOT0_TYPE) % kSlotStride;
          if (offset == kSlotOffsetType) ++types;
        }
      }
      if (types != kSlotCount) {
        ok = false;
        std::printf("preset '%s' does not define all %d slots\n", p.name,
                    kSlotCount);
      }
    }
    check(ok, "factory board presets are well formed");
  }
  {
    namespace fs = std::filesystem;
    const fs::path dir = fs::temp_directory_path() / "pedal-suite-ir-test";
    std::error_code ec;
    fs::remove_all(dir, ec);
    fs::create_directories(dir, ec);
    const fs::path wav = dir / "test.wav";
    const int frames = 64;
    std::vector<unsigned char> bytes;
    auto u32 = [&](uint32_t v) {
      bytes.push_back(static_cast<unsigned char>(v & 0xFF));
      bytes.push_back(static_cast<unsigned char>((v >> 8) & 0xFF));
      bytes.push_back(static_cast<unsigned char>((v >> 16) & 0xFF));
      bytes.push_back(static_cast<unsigned char>((v >> 24) & 0xFF));
    };
    auto u16 = [&](uint16_t v) {
      bytes.push_back(static_cast<unsigned char>(v & 0xFF));
      bytes.push_back(static_cast<unsigned char>((v >> 8) & 0xFF));
    };
    const uint32_t dataBytes = static_cast<uint32_t>(frames) * 2;
    bytes.insert(bytes.end(), {'R', 'I', 'F', 'F'});
    u32(36 + dataBytes);
    bytes.insert(bytes.end(), {'W', 'A', 'V', 'E'});
    bytes.insert(bytes.end(), {'f', 'm', 't', ' '});
    u32(16);
    u16(1);
    u16(1);
    u32(48000);
    u32(96000);
    u16(2);
    u16(16);
    bytes.insert(bytes.end(), {'d', 'a', 't', 'a'});
    u32(dataBytes);
    for (int i = 0; i < frames; ++i) {
      const int16_t v = i == 0 ? 16384 : 0;
      u16(static_cast<uint16_t>(v));
    }
    {
      std::ofstream out(wav, std::ios::binary | std::ios::trunc);
      out.write(reinterpret_cast<const char*>(bytes.data()),
                static_cast<std::streamsize>(bytes.size()));
    }
    const auto bank = loadIrBank(dir.string(), 48000);
    check(bank && bank->entries.size() == 1 &&
              bank->entries[0].name == "test.wav" &&
              !bank->entries[0].h.empty(),
          "IR bank loads a WAV file");
    fs::remove_all(dir, ec);
  }
  check(static_cast<int>(b.get(slotParam(7, kSlotOffsetType)) + 0.5) ==
            static_cast<int>(ModuleId::Boost),
        "state restores page-2 pedal type");

  check(!deserializeState(nullptr, 0, b), "null state is rejected");
  unsigned char bad[8] = {0};
  check(!deserializeState(bad, sizeof(bad), b), "short state is rejected");

  std::printf("%d failure(s)\n", failures);
  return failures == 0 ? 0 : 1;
}
