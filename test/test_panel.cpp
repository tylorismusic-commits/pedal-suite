// Pedal Suite — panel tests
// SPDX-License-Identifier: GPL-3.0-or-later
#include <cairo.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>

#include "module_ids.hpp"
#include "modules.hpp"
#include "params.hpp"
#include "presets.hpp"
#include "ui/panel.hpp"
#include "value_text.hpp"

using namespace pedal;

namespace {
int failures = 0;
void check(bool ok, const char* name) {
  std::printf("%s  %s\n", ok ? "PASS" : "FAIL", name);
  if (!ok) ++failures;
}
bool clickParam(Panel& panel, Shared& s, PanelState& state, uint32_t param) {
  for (const auto& r : panel.widgetRects()) {
    if (r.param != param) continue;
    panel.pointerDown(s, state, r.x + r.w * 0.5, r.y + r.h * 0.5, false);
    panel.pointerUp(s, state);
    return true;
  }
  return false;
}
bool clickLabel(Panel& panel, Shared& s, PanelState& state,
                const char* label) {
  for (const auto& r : panel.widgetRects()) {
    if (std::string(r.label) != label) continue;
    panel.pointerDown(s, state, r.x + r.w * 0.5, r.y + r.h * 0.5, false);
    panel.pointerUp(s, state);
    return true;
  }
  return false;
}
} // namespace

int main() {
  const std::filesystem::path config = "/tmp/opencode/pedal-suite-test-config";
  std::filesystem::remove_all(config);
  setenv("XDG_CONFIG_HOME", config.string().c_str(), 1);

  cairo_surface_t* surf =
      cairo_image_surface_create(CAIRO_FORMAT_RGB24, kPanelWidth, kPanelHeight);
  cairo_t* cr = cairo_create(surf);
  Shared s;
  initializeRack(s);
  Panel panel;
  PanelState state;
  panel.draw(cr, s, state);

  check(s.get(P_SLOT0_TYPE) == static_cast<double>(ModuleId::Boost),
        "page 1 starts with Boost");
  check(s.get(P_SLOT1_TYPE) == static_cast<double>(ModuleId::Drive808),
        "page 1 starts with 808 Drive");
  check(state.boardPage == 0, "board starts on page 1");

  // Page switch.
  panel.pointerDown(s, state, 678, 34, false);
  panel.pointerUp(s, state);
  check(state.boardPage == 1 && state.selectedSlot == 4,
        "page 2 button switches to slots 5-8");
  panel.draw(cr, s, state);

  // Add a Boost to slot 5.
  panel.pointerDown(s, state, 158, 180, false);
  panel.pointerUp(s, state);
  panel.draw(cr, s, state);
  check(clickLabel(panel, s, state, "DRIVE"), "browser offers DRIVE");
  panel.draw(cr, s, state);
  check(clickLabel(panel, s, state, "Boost"), "browser offers Boost");
  check(s.get(P_SLOT4_TYPE) == static_cast<double>(ModuleId::Boost),
        "browser loads a pedal into page 2");

  // Bypass.
  panel.draw(cr, s, state);
  check(clickParam(panel, s, state, P_SLOT4_BYPASS),
        "footswitch is clickable");
  check(s.get(P_SLOT4_BYPASS) == 1.0, "footswitch bypasses the pedal");
  // A click offset from the exact centre must still hit the round switch.
  s.set(P_SLOT4_BYPASS, 0.0);
  bool offsetHit = false;
  for (const auto& r : panel.widgetRects()) {
    if (r.param != P_SLOT4_BYPASS) continue;
    panel.pointerDown(s, state, r.x + 7.0, r.y + 3.0, false);
    panel.pointerUp(s, state);
    offsetHit = true;
    break;
  }
  check(offsetHit && s.get(P_SLOT4_BYPASS) == 1.0,
        "footswitch hit area covers the whole switch");
  check(state.browserSlot < 0, "footswitch does not open the picker");

  // Typed value.
  s.set(slotParam(0, kSlotOffsetC0), 0.40);
  double value = 0.0;
  check(parseParamValue(s, slotParam(0, kSlotOffsetC0), "18", &value),
        "typed value parses");
  check(std::fabs(value - 0.75) < 1e-9, "typed value updates the control");

  // Preset browser opens.
  panel.pointerDown(s, state, 237, 34, false);
  panel.pointerUp(s, state);
  check(state.presetOpen, "preset bar opens the preset browser");
  panel.draw(cr, s, state);
  panel.pointerDown(s, state, 200, 100, false);
  panel.pointerUp(s, state);
  check(!state.presetOpen && state.presetIndex == 0,
        "preset browser loads a factory board");

  // The gap between the last preset row and the footer must not select the
  // next (hidden) preset when the list is scrollable.
  panel.pointerDown(s, state, 150, 34, false);
  panel.pointerUp(s, state);
  panel.draw(cr, s, state);
  const int beforeGap = state.presetIndex;
  panel.pointerDown(s, state, 150, 342, false);
  panel.pointerUp(s, state);
  check(state.presetIndex == beforeGap,
        "preset list gap does not select a hidden preset");

  // Bank filtering and setlist management.
  panel.pointerDown(s, state, 237, 34, false);
  panel.pointerUp(s, state);
  check(state.presetOpen, "preset browser reopens for banks");
  panel.pointerDown(s, state, 318, 72, false);
  panel.pointerUp(s, state);
  check(state.presetBank == 1, "bank button filters the preset list");
  panel.pointerDown(s, state, 411, 72, false);
  panel.pointerUp(s, state);
  check(state.setlistOpen && !state.presetOpen,
        "SETLIST button opens the setlist manager");
  panel.pointerDown(s, state, 177, 631, false);
  panel.pointerUp(s, state);
  check(std::filesystem::exists(setlistDir() / "Setlist 1.setlist"),
        "NEW creates a setlist file");
  state.presetIndex = -1;
  panel.pointerDown(s, state, 417, 587, false);
  panel.pointerUp(s, state);
  auto created = loadSetlists(setlistDir());
  check(created.size() == 1 && created[0].entries.size() == 1 &&
            created[0].entries[0].snapshot.size() == P_COUNT &&
            created[0].entries[0].name == "Current Board",
        "ADD CURRENT snapshots the live board without a selected preset");
  panel.pointerDown(s, state, 540, 621, false);
  panel.pointerUp(s, state);
  check(state.setlistEntry == 0, "NEXT selects the first entry");
  panel.pointerDown(s, state, 311, 631, false);
  panel.pointerUp(s, state);
  panel.pointerDown(s, state, 311, 631, false);
  panel.pointerUp(s, state);
  check(loadSetlists(setlistDir()).empty(), "DELETE removes the setlist");
  panel.pointerDown(s, state, 417, 587, false);
  panel.pointerUp(s, state);
  auto autoCreated = loadSetlists(setlistDir());
  check(autoCreated.size() == 1 && autoCreated[0].entries.size() == 1,
        "ADD CURRENT creates a setlist when none exists");
  deleteSetlist(setlistDir(), autoCreated[0].name);
  panel.reloadSetlists();
  panel.pointerDown(s, state, 786, 621, false);
  panel.pointerUp(s, state);
  check(!state.setlistOpen, "setlist manager closes");
  state.presetBank = 0;

  // MIDI bank and setlist control.
  s.midiBankNextSeq.store(1);
  panel.tick(s, state);
  check(state.presetBank == 1, "MIDI bank next advances the bank filter");
  s.midiBankPrevSeq.store(1);
  panel.tick(s, state);
  check(state.presetBank == 0, "MIDI bank previous returns to ALL");

  Setlist midiSet;
  midiSet.name = "MIDI Set";
  midiSet.entries = {{"Clean Platform", true}, {"808 Stack", true}};
  writeSetlist(setlistDir(), midiSet);
  panel.reloadSetlists();
  state.setlistIndex = 0;
  state.setlistEntry = 0;
  s.midiSetlistNextSeq.store(1);
  panel.tick(s, state);
  check(state.presetIndex == 2 && state.setlistEntry == 1,
        "MIDI setlist next loads the next entry");
  s.midiSetlistPrevSeq.store(1);
  panel.tick(s, state);
  check(state.presetIndex == 1 && state.setlistEntry == 0,
        "MIDI setlist previous loads the previous entry");
  deleteSetlist(setlistDir(), "MIDI Set");
  panel.reloadSetlists();

  s.midiSetlistPcValue.store(2);
  s.midiSetlistPcSeq.store(1);
  panel.tick(s, state);
  check(state.presetIndex == 2,
        "MIDI Program Change selects a preset in the current bank");

  // Browser categories: UTIL, AMP and PRESETS.
  state.browserSlot = 4;
  state.browserCategory = 1;
  panel.draw(cr, s, state);
  check(clickLabel(panel, s, state, "UTIL"), "browser offers UTIL");
  panel.draw(cr, s, state);
  check(state.browserCategory == 5, "UTIL tab selects the util page");
  bool hasCab = false;
  for (const auto& r : panel.widgetRects()) {
    if (std::string(r.label) == "Cab") hasCab = true;
  }
  check(hasCab, "Cab is listed on the UTIL page");
  check(clickLabel(panel, s, state, "AMP"), "browser offers AMP");
  panel.draw(cr, s, state);
  check(state.browserCategory == 6, "AMP tab selects the amp page");
  bool hasNam = false;
  for (const auto& r : panel.widgetRects()) {
    if (std::string(r.label) == "NAM Amp") hasNam = true;
  }
  check(hasNam, "NAM Amp is listed on the AMP page");
  check(clickLabel(panel, s, state, "PRESETS"), "browser offers PRESETS");
  panel.draw(cr, s, state);
  check(state.browserCategory == 7, "PRESETS tab selects the preset page");
  check(clickLabel(panel, s, state, "CANCEL"), "browser cancels");
  state.browserSlot = -1;

  // About.
  panel.pointerDown(s, state, 966, 34, false);
  panel.pointerUp(s, state);
  check(state.aboutOpen, "ABOUT opens the reference");
  panel.pointerDown(s, state, 860, 667, false);
  panel.pointerUp(s, state);
  check(!state.aboutOpen, "ABOUT closes");

  // MIDI mapping panel.
  panel.draw(cr, s, state);
  panel.pointerDown(s, state, 958, 118, false);
  panel.pointerUp(s, state);
  check(state.midiOpen, "MIDI button opens the mapping panel");
  panel.draw(cr, s, state);
  panel.pointerDown(s, state, 688, 206, false);
  panel.pointerUp(s, state);
  check(s.midiLearnSlot.load() == 0, "pedal 1 learn button arms note/CC learn");
  panel.pointerDown(s, state, 688, 206, false);
  panel.pointerUp(s, state);
  check(s.midiLearnSlot.load() == -1, "learn button cancels learn mode");
  panel.pointerDown(s, state, 343, 510, false);
  panel.pointerUp(s, state);
  check(s.get(P_MIDI_EXPR_ON) > 0.5, "expression toggle enables expression");
  panel.pointerDown(s, state, 788, 510, false);
  panel.pointerUp(s, state);
  check(s.midiLearnExpr.load(), "expression learn arms CC learn");
  panel.pointerDown(s, state, 795, 612, false);
  panel.pointerUp(s, state);
  check(!state.midiOpen, "MIDI panel closes");

  // Descriptor sanity.
  bool valid = true;
  for (int id = 1; id < kModuleCount; ++id) {
    const ModuleDesc* d = moduleDesc(id);
    if (d->controlCount < 3 || d->controlCount > 6) valid = false;
    for (int k = 0; k < d->controlCount; ++k) {
      if (defaultControlValue(d->id, k) < 0.0 ||
          defaultControlValue(d->id, k) > 1.0) {
        valid = false;
      }
    }
  }
  check(valid, "pedal descriptors have 3-6 valid controls");

  cairo_destroy(cr);
  cairo_surface_destroy(surf);
  std::printf("%d failure(s)\n", failures);
  return failures == 0 ? 0 : 1;
}
