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
#include "setlists.hpp"
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
  {
    bool ok = true;
    for (int id = 1; id < kModuleCount; ++id) {
      Shared a;
      initializeRack(a);
      applyModuleDefaults(a, 0, id, false);
      const ModuleDesc* m = moduleDesc(id);
      for (int k = 0; k < m->controlCount; ++k) {
        a.set(slotParam(0, kSlotOffsetC0 + k),
              0.05 + 0.9 * ((k + 1) / static_cast<double>(m->controlCount + 1)));
      }
      a.set(P_CAB_IR, 7.0);
      a.set(P_NAM_MODEL, 3.0);
      const auto blob = serializeState(a, 1.0);
      Shared c;
      if (!deserializeState(blob.data(), blob.size(), c)) {
        ok = false;
        continue;
      }
      if (static_cast<int>(c.get(slotParam(0, kSlotOffsetType)) + 0.5) !=
          id) {
        ok = false;
        std::printf("state type mismatch for %s\n", m->name);
      }
      for (int k = 0; k < m->controlCount; ++k) {
        const double av = a.get(slotParam(0, kSlotOffsetC0 + k));
        const double cv = c.get(slotParam(0, kSlotOffsetC0 + k));
        if (std::fabs(av - cv) > 1e-9) {
          ok = false;
          std::printf("state control mismatch %s %d\n", m->name, k);
        }
      }
      if (std::fabs(c.get(P_CAB_IR) - 7.0) > 1e-9 ||
          std::fabs(c.get(P_NAM_MODEL) - 3.0) > 1e-9) {
        ok = false;
        std::printf("state resource index mismatch\n");
      }
    }
    check(ok, "state round-trips every module and resource index");
  }
  check(static_cast<int>(b.get(slotParam(7, kSlotOffsetType)) + 0.5) ==
            static_cast<int>(ModuleId::Boost),
        "state restores page-2 pedal type");

  check(!deserializeState(nullptr, 0, b), "null state is rejected");
  unsigned char bad[8] = {0};
  check(!deserializeState(bad, sizeof(bad), b), "short state is rejected");


  {
    Shared m;
    initializeRack(m);
    check(mappedNoteSlot(m, 36) == 0, "default MIDI note 36 maps to pedal 1");
    check(mappedNoteSlot(m, 60) == 0,
          "alternate MIDI note range still maps to pedal 1");
    check(mappedCcSlot(m, 20) == 0, "default MIDI CC 20 maps to pedal 1");

    assignNoteMapping(m, 3, 64);
    check(std::lround(m.get(P_MIDI_NOTE3)) == 64,
          "MIDI note learn stores the note");
    check(mappedNoteSlot(m, 64) == 3,
          "learned MIDI note wins over the default range");
    assignNoteMapping(m, 4, 64);
    check(std::lround(m.get(P_MIDI_NOTE3)) == 0,
          "learning a duplicate note clears the old mapping");
    check(mappedNoteSlot(m, 64) == 4, "duplicate note moves to the new pedal");

    assignCcMapping(m, 5, 70);
    check(std::lround(m.get(P_MIDI_CC5)) == 70,
          "MIDI CC learn stores the CC");
    check(mappedCcSlot(m, 70) == 5, "learned MIDI CC maps to its pedal");

    applyModuleDefaults(m, 0, static_cast<int>(ModuleId::Wah), false);
    m.set(P_MIDI_EXPR_ON, 0.0);
    applyExpression(m, 0.62f);
    check(std::fabs(m.get(slotParam(0, kSlotOffsetC0)) - 0.62) < 1e-6,
          "legacy expression drives the first Wah");

    applyModuleDefaults(m, 1, static_cast<int>(ModuleId::Boost), false);
    m.set(P_MIDI_EXPR_ON, 1.0);
    m.set(P_MIDI_EXPR_SLOT, 1.0);
    m.set(P_MIDI_EXPR_CTL, 2.0);
    m.set(P_MIDI_EXPR_CC, 64.0);
    applyExpression(m, 0.27f);
    check(std::fabs(m.get(slotParam(1, kSlotOffsetC0 + 2)) - 0.27) < 1e-6,
          "mapped expression drives the chosen pedal control");

    m.set(P_MIDI_NOTE2, 55.0);
    m.set(P_MIDI_CC2, 42.0);
    const auto midiBlob = serializeState(m, 1.0);
    Shared n;
    check(deserializeState(midiBlob.data(), midiBlob.size(), n),
          "MIDI mappings survive state round-trip");
    check(std::lround(n.get(P_MIDI_NOTE2)) == 55 &&
              std::lround(n.get(P_MIDI_CC2)) == 42 &&
              n.get(P_MIDI_EXPR_ON) > 0.5 &&
              std::lround(n.get(P_MIDI_EXPR_CC)) == 64,
          "state restores MIDI mappings and expression");
  }


  {
    Setlist s;
    s.name = "Test Set";
    s.entries.push_back({"Clean Platform", true});
    s.entries.push_back({"My Board", false});
    std::filesystem::remove_all(setlistDir());
    check(writeSetlist(setlistDir(), s), "setlist file writes");
    auto loaded = loadSetlists(setlistDir());
    check(loaded.size() == 1 && loaded[0].name == "Test Set" &&
              loaded[0].entries.size() == 2 &&
              loaded[0].entries[0].factory &&
              loaded[0].entries[0].name == "Clean Platform",
          "setlist file round-trips entries");
    check(renameSetlist(setlistDir(), "Test Set", "Renamed Set", loaded[0]),
          "setlist renames");
    loaded = loadSetlists(setlistDir());
    check(loaded.size() == 1 && loaded[0].name == "Renamed Set",
          "renamed setlist is listed");
    check(deleteSetlist(setlistDir(), "Renamed Set"), "setlist deletes");
    check(loadSetlists(setlistDir()).empty(), "setlist directory is empty");
  }

  std::printf("%d failure(s)\n", failures);
  return failures == 0 ? 0 : 1;
}
