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
  panel.pointerDown(s, state, 158, 134, false);
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

  // About.
  panel.pointerDown(s, state, 966, 34, false);
  panel.pointerUp(s, state);
  check(state.aboutOpen, "ABOUT opens the reference");
  panel.pointerDown(s, state, 860, 667, false);
  panel.pointerUp(s, state);
  check(!state.aboutOpen, "ABOUT closes");

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
