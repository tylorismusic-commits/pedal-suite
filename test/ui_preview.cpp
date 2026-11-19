// Pedal Suite — UI preview
// SPDX-License-Identifier: GPL-3.0-or-later
#include <cairo.h>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>

#include "module_ids.hpp"
#include "modules.hpp"
#include "params.hpp"
#include "presets.hpp"
#include "setlists.hpp"
#include "ui/panel.hpp"
#include "user_presets.hpp"

using namespace pedal;

int main(int argc, char** argv) {
  if (argc < 3) return 2;
  const std::string font = argv[1], out = argv[2];
  setenv("XDG_CONFIG_HOME", "/tmp/opencode/pedal-preview-config", 1);
  std::filesystem::remove_all(setlistDir().parent_path());
  Shared s;
  initializeRack(s);
  saveUserPreset(userPresetDir(), "Preview Board", s);
  {
    Setlist preview;
    preview.name = "Live Set";
    preview.entries = {{"Clean Platform", true},
                       {"808 Stack", true},
                       {"Lead Lift", true},
                       {"Ambient Wash", true},
                       {"Bass Growl", true}};
    writeSetlist(setlistDir(), preview);
  }
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
  applyModuleDefaults(s, 4, static_cast<int>(ModuleId::Chorus));
  applyModuleDefaults(s, 5, static_cast<int>(ModuleId::Flanger));
  applyModuleDefaults(s, 6, static_cast<int>(ModuleId::Phaser));
  applyModuleDefaults(s, 7, static_cast<int>(ModuleId::Tremolo));
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/pedal-board-page2.png").c_str());

  applyModuleDefaults(s, 4, static_cast<int>(ModuleId::BassDrive));
  applyModuleDefaults(s, 5, static_cast<int>(ModuleId::BassFuzz));
  applyModuleDefaults(s, 6, static_cast<int>(ModuleId::PitchDrop));
  applyModuleDefaults(s, 7, static_cast<int>(ModuleId::Retune));
  state.selectedSlot = 6;
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/pedal-board-pitch.png").c_str());
  state.selectedSlot = 4;

  applyModuleDefaults(s, 4, static_cast<int>(ModuleId::AnalogDelay));
  applyModuleDefaults(s, 5, static_cast<int>(ModuleId::SpringReverb));
  applyModuleDefaults(s, 6, static_cast<int>(ModuleId::Chorus));
  applyModuleDefaults(s, 7, static_cast<int>(ModuleId::Tremolo));
  state.selectedSlot = 4;
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/pedal-board-time.png").c_str());

  applyModuleDefaults(s, 4, static_cast<int>(ModuleId::BassChorus));
  applyModuleDefaults(s, 5, static_cast<int>(ModuleId::Octave));
  applyModuleDefaults(s, 6, static_cast<int>(ModuleId::TapeEcho));
  applyModuleDefaults(s, 7, static_cast<int>(ModuleId::PlateReverb));
  state.selectedSlot = 6;
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/pedal-board-more.png").c_str());
  state.selectedSlot = 4;

  applyModuleDefaults(s, 4, static_cast<int>(ModuleId::FetComp));
  applyModuleDefaults(s, 5, static_cast<int>(ModuleId::TightGate));
  applyModuleDefaults(s, 6, static_cast<int>(ModuleId::EnvFilter));
  applyModuleDefaults(s, 7, static_cast<int>(ModuleId::Eq3));
  state.slotGrDb[4] = 5.0f;
  state.slotGrDb[5] = 2.0f;
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/pedal-board-dyn.png").c_str());

  applyModuleDefaults(s, 4, static_cast<int>(ModuleId::PrecisionDrive));
  applyModuleDefaults(s, 5, static_cast<int>(ModuleId::Wah));
  applyModuleDefaults(s, 6, static_cast<int>(ModuleId::Laser));
  applyModuleDefaults(s, 7, static_cast<int>(ModuleId::Cab));
  for (float& gr : state.slotGrDb) gr = 0.0f;
  state.slotGrDb[4] = 3.5f;
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf,
                            (out + "/pedal-board-signature.png").c_str());

  state.irNames = {"Greenback SM57.wav", "V30 121.wav", "Recto 57.wav",
                   "Bass 52.wav"};
  state.irScroll = 0;
  state.irPickerSlot = 7;
  s.set(P_CAB_IR, 2);
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/pedal-cab-ir.png").c_str());
  state.irPickerSlot = -1;
  s.set(P_CAB_IR, 0);

  applyModuleDefaults(s, 4, static_cast<int>(ModuleId::Nam));
  applyModuleDefaults(s, 5, static_cast<int>(ModuleId::Cab));
  applyModuleDefaults(s, 6, static_cast<int>(ModuleId::PrecisionDrive));
  applyModuleDefaults(s, 7, static_cast<int>(ModuleId::NoiseGate));
  state.selectedSlot = 4;
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/pedal-board-nam.png").c_str());

  state.irNames = {"Misha Rhythm.nam", "Gojira Clean.nam",
                   "V30 Crunch.nam", "Bass DI.nam"};
  state.irScroll = 0;
  state.irPickerSlot = 4;
  state.pickerNam = true;
  s.set(P_NAM_MODEL, 1);
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/pedal-nam.png").c_str());
  state.irPickerSlot = -1;
  state.pickerNam = false;
  s.set(P_NAM_MODEL, 0);

  applyModuleDefaults(s, 5, static_cast<int>(ModuleId::Nam));
  applyModuleDefaults(s, 6, static_cast<int>(ModuleId::Nam));
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/pedal-nam-limit.png").c_str());

  state.browserSlot = 4;
  state.browserCategory = 6;
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/pedal-browser.png").c_str());
  state.browserSlot = -1;

  state.presetOpen = 1;
  state.presetIndex = kPresetCount;
  state.presetScroll = 0;
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/pedal-presets.png").c_str());
  state.presetOpen = 0;
  state.presetScroll = 0;

  state.browserSlot = -1;

  state.tunerOpen = true;
  s.tunerHz.store(440.0f);
  s.tunerCents.store(3.2f);
  s.tunerConfidence.store(0.93f);
  s.tunerNote.store(69);
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/pedal-tuner.png").c_str());
  state.tunerOpen = false;

  state.midiOpen = true;
  state.midiLearnSlot = 0;
  state.midiLearnExpr = false;
  s.set(P_MIDI_EXPR_ON, 1.0);
  s.set(P_MIDI_EXPR_SLOT, 4.0);
  s.set(P_MIDI_EXPR_CTL, 0.0);
  s.set(P_MIDI_EXPR_CC, 11.0);
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/pedal-midi.png").c_str());
  state.midiOpen = false;
  state.midiLearnSlot = -1;

  state.setlistOpen = true;
  state.setlistIndex = 0;
  state.setlistEntry = 2;
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/pedal-setlist.png").c_str());
  state.setlistOpen = false;

  state.aboutOpen = true;
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/pedal-about.png").c_str());

  cairo_destroy(cr);
  cairo_surface_destroy(surf);
  return 0;
}
