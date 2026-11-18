// Pedal Suite — UI preview
// SPDX-License-Identifier: GPL-3.0-or-later
#include <cairo.h>
#include <cstdio>
#include <string>

#include "module_ids.hpp"
#include "modules.hpp"
#include "params.hpp"
#include "presets.hpp"
#include "ui/panel.hpp"
#include "user_presets.hpp"

using namespace pedal;

int main(int argc, char** argv) {
  if (argc < 3) return 2;
  const std::string font = argv[1], out = argv[2];
  Shared s;
  initializeRack(s);
  saveUserPreset(userPresetDir(), "Preview Board", s);
  Panel panel;
  panel.loadFont(font);
  PanelState state;
  cairo_surface_t* surf =
      cairo_image_surface_create(CAIRO_FORMAT_ARGB32, kPanelWidth, kPanelHeight);
  cairo_t* cr = cairo_create(surf);

  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/pedal-board.png").c_str());

  applyModuleDefaults(s, 0, static_cast<int>(ModuleId::TrebleBoost));
  applyModuleDefaults(s, 1, static_cast<int>(ModuleId::Overdrive));
  applyModuleDefaults(s, 2, static_cast<int>(ModuleId::BluesDrive));
  applyModuleDefaults(s, 3, static_cast<int>(ModuleId::Distortion));
  state.selectedSlot = 1;
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/pedal-board-drive.png").c_str());

  state.boardPage = 1;
  state.selectedSlot = 4;
  applyModuleDefaults(s, 4, static_cast<int>(ModuleId::Rat));
  applyModuleDefaults(s, 5, static_cast<int>(ModuleId::BigFuzz));
  applyModuleDefaults(s, 6, static_cast<int>(ModuleId::FuzzFace));
  applyModuleDefaults(s, 7, static_cast<int>(ModuleId::Drive808));
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/pedal-board-page2.png").c_str());

  state.browserSlot = 4;
  state.browserCategory = 1;
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/pedal-browser.png").c_str());
  state.browserSlot = -1;

  state.presetOpen = 1;
  state.presetIndex = kPresetCount;
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/pedal-presets.png").c_str());
  state.presetOpen = 0;

  state.aboutOpen = true;
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/pedal-about.png").c_str());

  cairo_destroy(cr);
  cairo_surface_destroy(surf);
  return 0;
}
