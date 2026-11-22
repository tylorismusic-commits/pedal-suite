// Pedal Suite — panel.cpp
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ui/panel.hpp"
#include "dsp/ir_bank.hpp"

#include "factory_module_presets.hpp"
#include "midi_map.hpp"
#include "modules.hpp"
#include "presets.hpp"
#include "value_text.hpp"

#include <cairo-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace pedal {
namespace {

constexpr double kPi = 3.14159265358979323846;

// Pedalboard geometry.  The right-hand strip leaves room for the utility
// buttons and keeps the fourth pedal clear of the output meter.
constexpr double kPedalStartX = 46.0;
constexpr double kPedalWidth = 228.0;
constexpr double kPedalGap = 12.0;
constexpr double kPedalY = 150.0;
constexpr double kPedalH = 380.0;
constexpr double kUtilityTunerX = 842.0;
constexpr double kUtilityMidiX = 922.0;
constexpr double kUtilityY = 106.0;
constexpr double kUtilityW = 72.0;
constexpr double kUtilityH = 24.0;

// Rack preset browser geometry.
constexpr double kPresetListX = 72.0;
constexpr double kPresetListY = 54.0;
constexpr double kPresetListW = 380.0;
constexpr double kPresetRowH = 20.0;
constexpr double kPresetBottomGap = 8.0;
constexpr int kPresetVisibleRows = 12;
constexpr double kPresetHeaderH = 46.0;
constexpr double kPresetFooterH = 34.0;

int presetFilterTypeFor(const Shared& shared, const PanelState& state) {
  if (!state.browserPresetFilter || state.browserSlot < 0) return -1;
  return static_cast<int>(
      shared.get(slotParam(state.browserSlot, kSlotOffsetType)) + 0.5);
}

int presetRowsFor(int count) {
  return std::clamp(count, 3, kPresetVisibleRows);
}

bool cursorVisible() {
  using namespace std::chrono;
  const auto ms = duration_cast<milliseconds>(
                      steady_clock::now().time_since_epoch())
                      .count();
  return (ms / 500) % 2 == 0;
}
struct Rgb { double r, g, b; };
const Rgb kAccent{0.93, 0.55, 0.18};
const Rgb kText{0.94, 0.90, 0.84};
const Rgb kDim{0.62, 0.56, 0.49};
const Rgb kEdge{0.34, 0.29, 0.24};

void roundRect(cairo_t* cr, double x, double y, double w, double h, double r) {
  cairo_new_sub_path(cr);
  cairo_arc(cr, x + w - r, y + r, r, -kPi / 2, 0);
  cairo_arc(cr, x + w - r, y + h - r, r, 0, kPi / 2);
  cairo_arc(cr, x + r, y + h - r, r, kPi / 2, kPi);
  cairo_arc(cr, x + r, y + r, r, kPi, 3 * kPi / 2);
  cairo_close_path(cr);
}
void setRgb(cairo_t* cr, Rgb c, double a = 1.0) {
  cairo_set_source_rgba(cr, c.r, c.g, c.b, a);
}
double knobAngle(double f) { return -2.35 + f * 4.70; }
double clamp01(double x) { return std::clamp(x, 0.0, 1.0); }
const char* slotModuleName(int type) {
  return type == static_cast<int>(ModuleId::Off) ? "EMPTY"
                                                 : moduleDesc(type)->name;
}

struct ModuleStyle {
  Rgb body;
  Rgb edge;
  Rgb accent;
  Rgb knob;
};

ModuleStyle styleFor(int type) {
  switch (static_cast<ModuleId>(type)) {
    case ModuleId::Boost:
      return {{0.34, 0.26, 0.08}, {0.46, 0.35, 0.11},
              {0.98, 0.80, 0.38}, {0.96, 0.90, 0.78}};
    case ModuleId::Drive808:
      return {{0.18, 0.42, 0.20}, {0.10, 0.24, 0.12},
              {0.62, 0.95, 0.50}, {0.86, 0.95, 0.84}};
    case ModuleId::TrebleBoost:
      return {{0.42, 0.10, 0.07}, {0.18, 0.04, 0.03},
              {1.00, 0.55, 0.34}, {0.98, 0.84, 0.76}};
    case ModuleId::Overdrive:
      return {{0.10, 0.30, 0.30}, {0.04, 0.14, 0.14},
              {0.46, 0.96, 0.86}, {0.84, 0.97, 0.94}};
    case ModuleId::BluesDrive:
      return {{0.14, 0.24, 0.42}, {0.06, 0.10, 0.20},
              {0.58, 0.78, 1.00}, {0.86, 0.92, 1.00}};
    case ModuleId::Distortion:
      return {{0.48, 0.22, 0.06}, {0.20, 0.08, 0.02},
              {1.00, 0.68, 0.28}, {0.99, 0.88, 0.72}};
    case ModuleId::Rat:
      return {{0.12, 0.12, 0.13}, {0.04, 0.04, 0.05},
              {0.96, 0.36, 0.30}, {0.92, 0.84, 0.80}};
    case ModuleId::BigFuzz:
      return {{0.22, 0.10, 0.28}, {0.08, 0.03, 0.12},
              {0.86, 0.55, 1.00}, {0.94, 0.88, 0.98}};
    case ModuleId::FuzzFace:
      return {{0.10, 0.22, 0.40}, {0.03, 0.08, 0.18},
              {0.56, 0.78, 1.00}, {0.86, 0.93, 1.00}};
    case ModuleId::ToneBender:
      return {{0.36, 0.24, 0.08}, {0.16, 0.10, 0.03},
              {0.98, 0.76, 0.32}, {0.97, 0.91, 0.74}};
    case ModuleId::BassDrive:
      return {{0.10, 0.28, 0.18}, {0.04, 0.12, 0.07},
              {0.58, 0.96, 0.52}, {0.86, 0.96, 0.84}};
    case ModuleId::BassFuzz:
      return {{0.20, 0.10, 0.26}, {0.08, 0.03, 0.12},
              {0.88, 0.52, 1.00}, {0.94, 0.87, 0.98}};
    case ModuleId::Whammy:
      return {{0.42, 0.07, 0.10}, {0.18, 0.02, 0.04},
              {1.00, 0.42, 0.36}, {0.98, 0.86, 0.82}};
    case ModuleId::Drop:
      return {{0.20, 0.10, 0.40}, {0.08, 0.03, 0.18},
              {0.72, 0.42, 1.00}, {0.92, 0.86, 1.00}};
    case ModuleId::Retune:
      return {{0.07, 0.20, 0.38}, {0.02, 0.08, 0.18},
              {0.44, 0.82, 1.00}, {0.82, 0.93, 1.00}};
    case ModuleId::Chorus:
      return {{0.06, 0.30, 0.30}, {0.02, 0.12, 0.12},
              {0.42, 1.00, 0.88}, {0.82, 0.98, 0.94}};
    case ModuleId::Flanger:
      return {{0.26, 0.08, 0.40}, {0.10, 0.02, 0.16},
              {0.86, 0.50, 1.00}, {0.93, 0.86, 0.98}};
    case ModuleId::Phaser:
      return {{0.44, 0.20, 0.04}, {0.18, 0.07, 0.01},
              {1.00, 0.66, 0.26}, {0.99, 0.88, 0.70}};
    case ModuleId::Tremolo:
      return {{0.30, 0.22, 0.05}, {0.12, 0.09, 0.02},
              {1.00, 0.84, 0.34}, {0.98, 0.93, 0.72}};
    case ModuleId::AnalogDelay:
      return {{0.12, 0.28, 0.24}, {0.04, 0.12, 0.10},
              {0.58, 0.98, 0.80}, {0.86, 0.98, 0.92}};
    case ModuleId::SpringReverb:
      return {{0.20, 0.24, 0.30}, {0.07, 0.09, 0.12},
              {0.66, 0.84, 1.00}, {0.88, 0.94, 1.00}};
    case ModuleId::BassChorus:
      return {{0.05, 0.26, 0.34}, {0.02, 0.10, 0.14},
              {0.40, 0.90, 1.00}, {0.82, 0.96, 1.00}};
    case ModuleId::Vibrato:
      return {{0.34, 0.08, 0.28}, {0.14, 0.02, 0.11},
              {1.00, 0.48, 0.86}, {0.98, 0.86, 0.95}};
    case ModuleId::Octave:
      return {{0.30, 0.14, 0.03}, {0.12, 0.05, 0.01},
              {1.00, 0.72, 0.26}, {0.99, 0.90, 0.68}};
    case ModuleId::TapeEcho:
      return {{0.38, 0.30, 0.18}, {0.16, 0.12, 0.07},
              {1.00, 0.86, 0.62}, {0.99, 0.95, 0.86}};
    case ModuleId::DigitalDelay:
      return {{0.05, 0.22, 0.30}, {0.02, 0.08, 0.12},
              {0.44, 0.94, 1.00}, {0.84, 0.98, 1.00}};
    case ModuleId::PingPongDelay:
      return {{0.10, 0.16, 0.40}, {0.03, 0.05, 0.16},
              {0.60, 0.74, 1.00}, {0.88, 0.92, 1.00}};
    case ModuleId::PlateReverb:
      return {{0.28, 0.26, 0.34}, {0.10, 0.09, 0.14},
              {0.82, 0.78, 1.00}, {0.95, 0.93, 1.00}};
    case ModuleId::RoomReverb:
      return {{0.30, 0.24, 0.16}, {0.12, 0.09, 0.05},
              {0.92, 0.82, 0.62}, {0.98, 0.95, 0.88}};
    case ModuleId::FetComp:
      return {{0.12, 0.20, 0.14}, {0.04, 0.08, 0.05},
              {0.72, 0.94, 0.62}, {0.90, 0.98, 0.86}};
    case ModuleId::OptoComp:
      return {{0.14, 0.14, 0.16}, {0.04, 0.04, 0.05},
              {0.98, 0.80, 0.36}, {0.99, 0.93, 0.76}};
    case ModuleId::EnvFilter:
      return {{0.42, 0.18, 0.04}, {0.18, 0.06, 0.01},
              {1.00, 0.70, 0.30}, {0.99, 0.89, 0.72}};
    case ModuleId::AutoWah:
      return {{0.10, 0.32, 0.20}, {0.03, 0.10, 0.06},
              {0.66, 1.00, 0.72}, {0.86, 0.99, 0.90}};
    case ModuleId::NoiseGate:
      return {{0.16, 0.18, 0.20}, {0.05, 0.06, 0.07},
              {0.68, 0.88, 1.00}, {0.88, 0.96, 1.00}};
    case ModuleId::TightGate:
      return {{0.14, 0.10, 0.11}, {0.04, 0.03, 0.03},
              {1.00, 0.40, 0.36}, {0.98, 0.84, 0.82}};
    case ModuleId::Eq3:
      return {{0.08, 0.18, 0.34}, {0.02, 0.06, 0.14},
              {0.50, 0.82, 1.00}, {0.84, 0.94, 1.00}};
    case ModuleId::BassEq:
      return {{0.05, 0.26, 0.26}, {0.02, 0.10, 0.10},
              {0.44, 0.98, 0.86}, {0.82, 0.98, 0.93}};
    case ModuleId::LoFi:
      return {{0.28, 0.10, 0.36}, {0.10, 0.03, 0.14},
              {0.94, 0.56, 1.00}, {0.96, 0.88, 1.00}};
    case ModuleId::TapeSat:
      return {{0.36, 0.26, 0.10}, {0.14, 0.10, 0.04},
              {1.00, 0.82, 0.48}, {0.99, 0.94, 0.82}};
    case ModuleId::PrecisionDrive:
      return {{0.10, 0.20, 0.12}, {0.03, 0.07, 0.04},
              {0.66, 1.00, 0.54}, {0.90, 0.99, 0.84}};
    case ModuleId::Wah:
      return {{0.30, 0.08, 0.07}, {0.11, 0.02, 0.02},
              {1.00, 0.72, 0.52}, {0.99, 0.90, 0.80}};
    case ModuleId::Laser:
      return {{0.12, 0.06, 0.30}, {0.04, 0.02, 0.12},
              {0.46, 0.98, 1.00}, {0.84, 0.99, 1.00}};
    case ModuleId::Cab:
      return {{0.16, 0.13, 0.09}, {0.06, 0.05, 0.03},
              {0.90, 0.74, 0.46}, {0.98, 0.93, 0.80}};
    default:
      return {{0.105, 0.10, 0.095}, {0.24, 0.22, 0.19},
              {0.55, 0.50, 0.44}, {0.72, 0.70, 0.66}};
  }
}

int cabRankForSlot(const Shared& shared, int slot) {
  int rank = 0;
  for (int s = 0; s < slot; ++s) {
    if (static_cast<int>(shared.get(slotParam(s, kSlotOffsetType)) + 0.5) ==
            static_cast<int>(ModuleId::Cab) &&
        shared.get(slotParam(s, kSlotOffsetBypass)) <= 0.5) {
      ++rank;
    }
  }
  return rank;
}

int namRankForSlot(const Shared& shared, int slot) {
  int rank = 0;
  for (int s = 0; s < slot; ++s) {
    if (static_cast<int>(shared.get(slotParam(s, kSlotOffsetType)) + 0.5) ==
        static_cast<int>(ModuleId::Nam)) {
      ++rank;
    }
  }
  return rank;
}

bool browserCategoryMatches(int id, int category) {
  const ModuleDesc* d = moduleDesc(id);
  if (category == 1) return std::strcmp(d->category, "Drive") == 0;
  if (category == 2) return std::strcmp(d->category, "Mod") == 0;
  if (category == 3) return std::strcmp(d->category, "Dyn") == 0;
  if (category == 4) return std::strcmp(d->category, "Time") == 0;
  if (category == 5) return std::strcmp(d->category, "Util") == 0;
  if (category == 6) return std::strcmp(d->category, "Amp") == 0;
  return true;
}

bool browserModuleMatchesQuery(int id, const std::string& query) {
  if (query.empty()) return true;
  const ModuleDesc* d = moduleDesc(id);
  std::string hay = std::string(d->name) + " " + d->category;
  hay = lowerText(hay);
  return hay.find(lowerText(query)) != std::string::npos;
}

int browserVisibleCount(const std::string& query) {
  return query.empty() ? 16 : 8;
}

int browserEntryCount(int category, const std::string& query) {
  int count = query.empty() ? 1 : 0;  // Off
  for (int id = 1; id < kModuleCount; ++id) {
    if (!browserCategoryMatches(id, category)) continue;
    if (!browserModuleMatchesQuery(id, query)) continue;
    ++count;
  }
  return count;
}

} // namespace

Panel::Panel() {
  userPresets_ = loadUserPresets(userPresetDir());
  modulePresets_ = loadModulePresets(modulePresetDir());
  setlists_ = loadSetlists(setlistDir());
}

void Panel::reloadSetlists() {
  setlists_ = loadSetlists(setlistDir());
}

std::vector<std::string> Panel::presetBankNames() const {
  std::vector<std::string> names{"ALL"};
  for (int i = 0; i < kPresetCount; ++i) {
    const std::string category =
        kPresets[i].category ? kPresets[i].category : "Other";
    if (std::find(names.begin(), names.end(), category) == names.end()) {
      names.push_back(category);
    }
  }
  names.push_back("USER");
  return names;
}

std::vector<int> Panel::presetOrderForBank(int bank) const {
  const std::vector<std::string> banks = presetBankNames();
  bank = std::clamp(bank, 0, static_cast<int>(banks.size()) - 1);
  std::vector<int> order;
  if (bank == 0) {
    for (int i = 0; i < kPresetCount; ++i) order.push_back(i);
    for (int i = 0; i < static_cast<int>(userPresets_.size()); ++i) {
      order.push_back(kPresetCount + i);
    }
    return order;
  }
  if (bank == static_cast<int>(banks.size()) - 1) {
    for (int i = 0; i < static_cast<int>(userPresets_.size()); ++i) {
      order.push_back(kPresetCount + i);
    }
    return order;
  }
  const std::string& category = banks[static_cast<size_t>(bank)];
  for (int i = 0; i < kPresetCount; ++i) {
    const std::string c = kPresets[i].category ? kPresets[i].category : "";
    if (c == category) order.push_back(i);
  }
  return order;
}

int Panel::resolveSetlistEntry(const SetlistEntry& entry) const {
  if (!entry.snapshot.empty()) return -1;
  if (entry.factory) {
    for (int i = 0; i < kPresetCount; ++i) {
      if (entry.name == kPresets[i].name) return i;
    }
    return -1;
  }
  for (int i = 0; i < static_cast<int>(userPresets_.size()); ++i) {
    if (entry.name == userPresets_[static_cast<size_t>(i)].name) {
      return kPresetCount + i;
    }
  }
  return -1;
}

void Panel::applySetlistEntry(Shared& shared, PanelState& state,
                              const SetlistEntry& entry) const {
  if (!entry.snapshot.empty()) {
    const int n = std::min(static_cast<int>(entry.snapshot.size()),
                           static_cast<int>(P_COUNT));
    for (int i = 0; i < n; ++i) {
      shared.set(static_cast<uint32_t>(i), entry.snapshot[static_cast<size_t>(i)]);
    }
    state.presetIndex = -1;
    state.presetDirty = true;
    state.presetOpen = 0;
    return;
  }
  const int resolved = resolveSetlistEntry(entry);
  if (resolved >= 0) applyPreset(shared, state, resolved);
}
Panel::~Panel() {
  if (font_) cairo_font_face_destroy(font_);
  if (ftFace_) FT_Done_Face(static_cast<FT_Face>(ftFace_));
  if (ft_) FT_Done_FreeType(static_cast<FT_Library>(ft_));
  if (grain_) cairo_surface_destroy(grain_);
}

void Panel::loadFont(const std::string& dir) {
  if (ft_) return;
  std::string path = dir;
  if (!path.empty() && path.back() != '/') path += '/';
  path += "Jost-ExtraBold.ttf";
  FT_Library lib = nullptr;
  if (FT_Init_FreeType(&lib) != 0) return;
  FT_Face face = nullptr;
  if (FT_New_Face(lib, path.c_str(), 0, &face) != 0) {
    FT_Done_FreeType(lib);
    return;
  }
  font_ = cairo_ft_font_face_create_for_ft_face(face, 0);
  ft_ = lib;
  ftFace_ = face;
}

void Panel::text(cairo_t* cr, const char* s, double x, double y, double size,
                 int align, double r, double g, double b) const {
  cairo_set_font_face(cr, font_);
  cairo_set_font_size(cr, size);
  cairo_set_source_rgb(cr, r, g, b);
  cairo_text_extents_t e;
  cairo_text_extents(cr, s, &e);
  double tx = x;
  if (align == 1) tx = x - e.x_advance * 0.5;
  else if (align == 2) tx = x - e.x_advance;
  cairo_move_to(cr, tx, y);
  cairo_show_text(cr, s);
}

void Panel::tick(Shared& shared, PanelState& state) const {
  state.bpm = shared.bpm.load(std::memory_order_relaxed);
  state.hostTempo = shared.hostTempo.load(std::memory_order_relaxed);
  state.inLevelL = shared.inPeakLDb.load(std::memory_order_relaxed);
  state.inLevelR = shared.inPeakRDb.load(std::memory_order_relaxed);
  state.outLevelL = shared.outPeakLDb.load(std::memory_order_relaxed);
  state.outLevelR = shared.outPeakRDb.load(std::memory_order_relaxed);
  for (int i = 0; i < kSlotCount; ++i)
    state.slotGrDb[i] = shared.slotGrDb[i].load(std::memory_order_relaxed);
  state.midiLearnSlot =
      shared.midiLearnSlotState.load(std::memory_order_relaxed);
  state.midiLearnExpr =
      shared.midiLearnExprState.load(std::memory_order_relaxed);

  const uint32_t bankNext =
      shared.midiBankNextSeq.load(std::memory_order_acquire);
  if (bankNext != state.midiBankNextSeen) {
    state.midiBankNextSeen = bankNext;
    const int n = static_cast<int>(presetBankNames().size());
    if (n > 0) {
      state.presetBank = (state.presetBank + 1) % n;
      state.presetScroll = 0;
    }
  }
  const uint32_t bankPrev =
      shared.midiBankPrevSeq.load(std::memory_order_acquire);
  if (bankPrev != state.midiBankPrevSeen) {
    state.midiBankPrevSeen = bankPrev;
    const int n = static_cast<int>(presetBankNames().size());
    if (n > 0) {
      state.presetBank = (state.presetBank + n - 1) % n;
      state.presetScroll = 0;
    }
  }

  auto loadSetlistIndex = [&](int index) {
    if (setlists_.empty()) return;
    const int setCount = static_cast<int>(setlists_.size());
    state.setlistIndex = std::clamp(state.setlistIndex, 0, setCount - 1);
    const Setlist& set = setlists_[static_cast<size_t>(state.setlistIndex)];
    if (index < 0 || index >= static_cast<int>(set.entries.size())) return;
    state.setlistEntry = index;
    state.setlistEntryScroll =
        std::max(0, std::min(index - 5,
                             static_cast<int>(set.entries.size()) - 12));
    applySetlistEntry(shared, state, set.entries[static_cast<size_t>(index)]);
  };

  const uint32_t setlistNext =
      shared.midiSetlistNextSeq.load(std::memory_order_acquire);
  if (setlistNext != state.midiSetlistNextSeen) {
    state.midiSetlistNextSeen = setlistNext;
    if (!setlists_.empty()) {
      state.setlistIndex = std::clamp(
          state.setlistIndex, 0, static_cast<int>(setlists_.size()) - 1);
      const int count = static_cast<int>(
          setlists_[static_cast<size_t>(state.setlistIndex)].entries.size());
      if (count > 0) {
        const int next =
            state.setlistEntry < 0 || state.setlistEntry + 1 >= count
                ? 0
                : state.setlistEntry + 1;
        loadSetlistIndex(next);
      }
    }
  }
  const uint32_t setlistPrev =
      shared.midiSetlistPrevSeq.load(std::memory_order_acquire);
  if (setlistPrev != state.midiSetlistPrevSeen) {
    state.midiSetlistPrevSeen = setlistPrev;
    if (!setlists_.empty()) {
      state.setlistIndex = std::clamp(
          state.setlistIndex, 0, static_cast<int>(setlists_.size()) - 1);
      const int count = static_cast<int>(
          setlists_[static_cast<size_t>(state.setlistIndex)].entries.size());
      if (count > 0) {
        const int prev = state.setlistEntry <= 0 ? count - 1
                                                 : state.setlistEntry - 1;
        loadSetlistIndex(prev);
      }
    }
  }

  const uint32_t pcSeq =
      shared.midiSetlistPcSeq.load(std::memory_order_acquire);
  if (pcSeq != state.midiSetlistPcSeen) {
    state.midiSetlistPcSeen = pcSeq;
    const int program =
        shared.midiSetlistPcValue.load(std::memory_order_relaxed);
    if (program >= 0) {
      bool usedSetlist = false;
      if (!setlists_.empty()) {
        state.setlistIndex = std::clamp(
            state.setlistIndex, 0, static_cast<int>(setlists_.size()) - 1);
        const int count = static_cast<int>(
            setlists_[static_cast<size_t>(state.setlistIndex)].entries.size());
        if (program < count) {
          loadSetlistIndex(program);
          usedSetlist = true;
        }
      }
      if (!usedSetlist) {
        const std::vector<int> order = presetOrderForBank(state.presetBank);
        if (program < static_cast<int>(order.size())) {
          applyPreset(shared, state, order[static_cast<size_t>(program)]);
        }
      }
    }
  }

  const uint32_t perfRevision =
      shared.perfRevision.load(std::memory_order_acquire);
  if (perfRevision != state.perfSeen) {
    state.presetBank =
        shared.perfBankIndex.load(std::memory_order_relaxed);
    state.setlistIndex =
        shared.perfSetlistIndex.load(std::memory_order_relaxed);
    state.setlistEntry =
        shared.perfSetlistEntry.load(std::memory_order_relaxed);
    state.perfSeen = perfRevision;
  }
  const uint32_t programRevision =
      shared.perfProgramRevision.load(std::memory_order_acquire);
  if (programRevision != state.perfProgramSeen) {
    state.perfProgramSeen = programRevision;
    state.presetIndex = -1;
    state.presetDirty = true;
  }
  if (perfRevision == state.perfSeen &&
      (state.presetBank !=
           shared.perfBankIndex.load(std::memory_order_relaxed) ||
       state.setlistIndex !=
           shared.perfSetlistIndex.load(std::memory_order_relaxed) ||
       state.setlistEntry !=
           shared.perfSetlistEntry.load(std::memory_order_relaxed))) {
    shared.perfBankIndex.store(state.presetBank, std::memory_order_relaxed);
    shared.perfSetlistIndex.store(state.setlistIndex,
                                  std::memory_order_relaxed);
    shared.perfSetlistEntry.store(state.setlistEntry,
                                  std::memory_order_relaxed);
    state.perfSeen =
        shared.perfRevision.fetch_add(1, std::memory_order_release) + 1;
  }
}

void Panel::layout(const Shared& shared, const PanelState& state) {
  widgets_.clear();
  Widget p{};
  p.kind = Kind::Power; p.param = P_POWER; p.x = 34; p.y = 30; p.r = 14;
  widgets_.push_back(p);

  auto knob = [&](uint32_t param, double x, double y, double r,
                  const char* label, int styleSlot = -1, bool compact = true,
                  int moduleControl = -1, int steps = 0) {
    Widget k{};
    k.kind = Kind::Knob; k.param = param; k.x = x; k.y = y; k.r = r;
    k.label = label; k.labelBelow = false; k.compact = compact;
    k.styleSlot = styleSlot;
    k.styleType = styleSlot >= 0
                      ? static_cast<int>(shared.get(
                            slotParam(styleSlot, kSlotOffsetType)) + 0.5)
                      : -1;
    k.moduleControl = moduleControl;
    k.steps = steps;
    widgets_.push_back(k);
  };

  knob(P_INPUT, 140, 675, 16, "", -1, true);
  knob(P_OUTPUT, 860, 675, 16, "", -1, true);
  {
    Widget sy{};
    sy.kind = Kind::Pill;
    sy.param = P_SYNC;
    sy.x = 556; sy.y = 662; sy.w = 68; sy.h = 24;
    sy.label = "SYNC";
    widgets_.push_back(sy);
  }

  const double colW = kPedalWidth;
  const double gap = kPedalGap;
  const double y = kPedalY;
  const double h = kPedalH;
  const int base = state.boardPage * 4;
  for (int i = 0; i < 4; ++i) {
    const int slot = base + i;
    const double x = kPedalStartX + i * (colW + gap);
    const int type = static_cast<int>(
        shared.get(slotParam(slot, kSlotOffsetType)) + 0.5);

    Widget name{};
    name.kind = Kind::ModuleName;
    name.param = static_cast<uint32_t>(slot);
    name.value = slot;
    name.x = x + 12; name.y = y + 12; name.w = colW - 24; name.h = 32;
    name.styleSlot = slot;
    name.styleType = type;
    widgets_.push_back(name);

    if (type != static_cast<int>(ModuleId::Off)) {
      Widget presets{};
      presets.kind = Kind::ModulePresetButton;
      presets.param = static_cast<uint32_t>(slot);
      presets.value = slot;
      presets.x = x + colW - 72;
      presets.y = y + 8;
      presets.w = 60;
      presets.h = 15;
      presets.label = "PRESET";
      presets.styleSlot = slot;
      presets.styleType = type;
      widgets_.push_back(presets);

      Widget foot{};
      foot.kind = Kind::Footswitch;
      foot.param = slotParam(slot, kSlotOffsetBypass);
      foot.x = x + colW * 0.5;
      foot.y = y + 326;
      foot.r = 16;
      foot.label = "FOOTSWITCH";
      foot.styleSlot = slot;
      foot.styleType = type;
      widgets_.push_back(foot);

      if (type == static_cast<int>(ModuleId::Cab) &&
          cabRankForSlot(shared, slot) < kMaxCabSlots) {
        Widget ir{};
        ir.kind = Kind::CabIrButton;
        ir.param = slotParam(slot, kSlotOffsetType);
        ir.value = slot;
        ir.x = x + colW - 112;
        ir.y = y + 48;
        ir.w = 92;
        ir.h = 18;
        ir.label = "LOAD IR";
        ir.styleSlot = slot;
        ir.styleType = type;
        widgets_.push_back(ir);
      }
      if (type == static_cast<int>(ModuleId::Nam) &&
          namRankForSlot(shared, slot) < kMaxNamSlots) {
        Widget amp{};
        amp.kind = Kind::NamModelButton;
        amp.param = slotParam(slot, kSlotOffsetType);
        amp.value = slot;
        amp.x = x + colW - 112;
        amp.y = y + 48;
        amp.w = 92;
        amp.h = 18;
        amp.label = "LOAD AMP";
        amp.styleSlot = slot;
        amp.styleType = type;
        widgets_.push_back(amp);
      }
    }

    Widget handle{};
    handle.kind = Kind::ModuleDragHandle;
    handle.param = static_cast<uint32_t>(slot);
    handle.value = slot;
    handle.x = x + 4; handle.y = y + 4; handle.w = colW - 8; handle.h = 46;
    handle.styleSlot = slot;
    handle.styleType = type;
    widgets_.push_back(handle);

    const ModuleDesc* module = moduleDesc(type);
    const double cx1 = x + colW * 0.26;
    const double cx2 = x + colW * 0.74;
    if (module->controlCount > 0) {
      const bool multiRow = module->controlCount >= 5;
      for (int k = 0; k < module->controlCount; ++k) {
        const ControlDesc& control = module->controls[k];
        double kx = (k & 1) ? cx2 : cx1;
        double ky = y + 132.0 + (k / 2) * 86.0;
        double kr = 22.0;
        if (multiRow) {
          kr = 18.0;
          if (k < 3) {
            kx = x + colW * ((k + 0.5) / 3.0);
            ky = y + 120.0;
          } else {
            const int remaining = module->controlCount - 3;
            kx = x + colW * (((k - 3) + 0.5) / remaining);
            ky = y + 204.0;
          }
        } else if (module->controlCount == 3 && k == 2) {
          kx = x + colW * 0.5;
          ky = y + 232.0;
        }
        int steps = control.stepped ? control.labels.count : 0;
        if (control.stepped && control.format == ControlFormat::Semitones) {
          steps = 25;
        }
        knob(slotParam(slot, kSlotOffsetC0 + control.param), kx, ky, kr,
             control.label, slot, true, control.param, steps);
      }
    }

    if (type != static_cast<int>(ModuleId::Off)) {
      knob(slotParam(slot, kSlotOffsetMix), x + colW * 0.34, 568, 14,
           "MIX", slot);
      knob(slotParam(slot, kSlotOffsetGain), x + colW * 0.66, 568, 14,
           "GAIN", slot);
    }

    Widget card{};
    card.kind = Kind::ModuleDragHandle;
    card.param = static_cast<uint32_t>(slot);
    card.value = slot;
    card.x = x; card.y = y; card.w = colW; card.h = h;
    card.styleSlot = slot;
    card.styleType = type;
    widgets_.push_back(card);
  }

  if (state.browserSlot >= 0 && state.browserSlot < kSlotCount) {
    const double bx = 240, bw = 560;
    const int filterType = presetFilterTypeFor(shared, state);
    const double bh =
        browserHeight(state.browserCategory, state.browserQuery, filterType);
    const double by =
        browserY(state.browserCategory, state.browserQuery, filterType);
    const int tabCount = 8;
    const double tabW = 64.0, tabGap = 2.0;
    const double tabTotal = tabCount * tabW + (tabCount - 1) * tabGap;
    const double tabStart = bx + (bw - tabTotal) * 0.5;
    static const char* tabLabels[8] = {"ALL",  "DRIVE", "MOD",  "DYN",
                                       "TIME", "UTIL",  "AMP",  "PRESETS"};
    for (int tab = 0; tab < tabCount; ++tab) {
      Widget t{};
      t.kind = Kind::BrowserTab;
      t.param = slotParam(state.browserSlot, kSlotOffsetType);
      t.value = tab;
      t.x = tabStart + tab * (tabW + tabGap);
      t.y = by + 46;
      t.w = tabW;
      t.h = 24.0;
      t.label = tabLabels[tab];
      widgets_.push_back(t);
    }

    struct BEntry {
      int value = 0;
      int styleType = 0;
      const char* label = "";
      const char* subLabel = nullptr;
    };
    std::vector<BEntry> entries;
    if (state.browserCategory == 7) {
      const std::string needle = lowerText(state.browserQuery);
      for (int i = 0; i < kFactoryModulePresetCount; ++i) {
        const FactoryModulePreset& p = kFactoryModulePresets[i];
        if (filterType >= 0 && static_cast<int>(p.type) != filterType) continue;
        if (!needle.empty() &&
            lowerText(p.name).find(needle) == std::string::npos) {
          continue;
        }
        entries.push_back({-1000 - i, static_cast<int>(p.type), p.name,
                           p.source});
      }
      for (int i = 0; i < static_cast<int>(modulePresets_.size()); ++i) {
        const ModulePreset& p = modulePresets_[static_cast<size_t>(i)];
        if (filterType >= 0 && p.type != filterType) continue;
        if (!needle.empty() &&
            lowerText(p.name).find(needle) == std::string::npos) {
          continue;
        }
        entries.push_back({-2000 - i, p.type, p.name.c_str(),
                           "User module preset"});
      }
    } else {
      if (state.browserQuery.empty())
        entries.push_back({static_cast<int>(ModuleId::Off),
                           static_cast<int>(ModuleId::Off), "OFF", nullptr});
      for (int id = 1; id < kModuleCount; ++id) {
        if (!browserCategoryMatches(id, state.browserCategory)) continue;
        if (!browserModuleMatchesQuery(id, state.browserQuery)) continue;
        entries.push_back({id, id, slotModuleName(id),
                           moduleDesc(id)->category});
      }
    }
    const int columns = 4;
    const int visible = browserVisibleCount(state.browserQuery);
    const int maxScroll = std::max(0, static_cast<int>(entries.size()) - visible);
    const int scroll = std::clamp(state.browserScroll, 0, maxScroll);
    const double ow = 120.0, oh = 46.0, ox = 6.0, oy = 8.0;
    for (int k = scroll;
         k < static_cast<int>(entries.size()) && k < scroll + visible; ++k) {
      const int pos = k - scroll;
      const int rowStart = (k / columns) * columns;
      const int rowCount = std::min(
          columns, static_cast<int>(entries.size()) - rowStart);
      const double rowWidth =
          rowCount * ow + std::max(0, rowCount - 1) * ox;
      const double rowX = bx + (bw - rowWidth) * 0.5;
      const BEntry& entry = entries[static_cast<size_t>(k)];
      Widget b{};
      b.kind = Kind::BrowserOption;
      b.param = slotParam(state.browserSlot, kSlotOffsetType);
      b.value = entry.value;
      b.x = rowX + static_cast<double>(k % columns) * (ow + ox);
      b.y = by + 92 + static_cast<double>(pos / columns) * (oh + oy);
      b.w = ow;
      b.h = oh;
      b.label = entry.label;
      b.subLabel = entry.subLabel;
      b.styleType = entry.styleType;
      widgets_.push_back(b);
    }
    Widget search{};
    search.kind = Kind::BrowserSearch;
    search.param = slotParam(state.browserSlot, kSlotOffsetType);
    search.x = bx + 150;
    search.y = by + bh - 42;
    search.w = bw - 290;
    search.h = 28;
    search.label = "SEARCH";
    widgets_.push_back(search);

    if (maxScroll > 0) {
      Widget up{};
      up.kind = Kind::BrowserScroll;
      up.value = -1.0;
      up.x = bx + 18;
      up.y = by + bh - 42;
      up.w = 58;
      up.h = 28;
      up.label = "UP";
      widgets_.push_back(up);
      Widget down{};
      down.kind = Kind::BrowserScroll;
      down.value = 1.0;
      down.x = bx + 82;
      down.y = by + bh - 42;
      down.w = 58;
      down.h = 28;
      down.label = "DOWN";
      widgets_.push_back(down);
    }

    Widget cancel{};
    cancel.kind = Kind::BrowserOption;
    cancel.param = slotParam(state.browserSlot, kSlotOffsetType);
    cancel.value = -1;
    cancel.x = bx + bw - 130; cancel.y = by + bh - 42;
    cancel.w = 110; cancel.h = 28;
    cancel.label = "CANCEL";
    widgets_.push_back(cancel);
  }
}

void Panel::drawHeader(cairo_t* cr, const Shared& shared, const PanelState& state) const {
  cairo_set_source_rgb(cr, 0.055, 0.052, 0.048);
  cairo_rectangle(cr, 0, 0, kPanelWidth, 78);
  cairo_fill(cr);
  setRgb(cr, kEdge, 0.7);
  cairo_set_line_width(cr, 1.0);
  cairo_move_to(cr, 0, 77.5); cairo_line_to(cr, kPanelWidth, 77.5); cairo_stroke(cr);

  const bool on = shared.get(P_POWER) > 0.5;
  cairo_arc(cr, 34, 32, 14, 0, 2 * kPi);
  setRgb(cr, {0.09, 0.08, 0.07}); cairo_fill(cr);
  cairo_arc(cr, 34, 32, 11, 0, 2 * kPi);
  setRgb(cr, on ? Rgb{0.38, 0.86, 0.46} : Rgb{0.42, 0.40, 0.38});
  cairo_set_line_width(cr, 2.2); cairo_stroke(cr);
  cairo_move_to(cr, 34, 23); cairo_line_to(cr, 34, 31); cairo_stroke(cr);

  // Comp Suite-style preset bar.
  roundRect(cr, 72, 14, 330, 36, 4);
  setRgb(cr, {0.15, 0.13, 0.11}); cairo_fill(cr);
  setRgb(cr, kEdge, 0.8); cairo_set_line_width(cr, 1.0);
  roundRect(cr, 72.5, 14.5, 329, 35, 4); cairo_stroke(cr);
  text(cr, "<", 90, 39, 16, 1, kText.r, kText.g, kText.b);
  text(cr, ">", 384, 39, 16, 1, kText.r, kText.g, kText.b);
  text(cr, presetName(state.presetIndex), 237, 39, 13, 1, kText.r, kText.g,
       kText.b);

  text(cr, "PEDAL SUITE", 520, 44, 24, 1, kText.r, kText.g, kText.b);

  struct Pill { const char* label; double x; };
  for (int page = 0; page < 2; ++page) {
    const double px = 610.0 + page * 48.0;
    const bool active = state.boardPage == page;
    roundRect(cr, px, 20, 40, 28, 4);
    setRgb(cr, active ? kAccent : Rgb{0.20, 0.17, 0.14});
    cairo_fill(cr);
    char label[4];
    std::snprintf(label, sizeof(label), "%d", page + 1);
    text(cr, label, px + 20, 39, 10, 1, active ? 0.10 : kDim.r,
         active ? 0.08 : kDim.g, active ? 0.05 : kDim.b);
  }
  const Pill pills[] = {{"PRESETS", 706}, {"SAVE", 812}, {"ABOUT", 918}};
  for (const Pill& pill : pills) {
    const bool active = (std::string(pill.label) == "ABOUT" && state.aboutOpen) ||
                        (std::string(pill.label) == "SAVE" && state.savePreset) ||
                        (std::string(pill.label) == "PRESETS" && state.presetOpen);
    roundRect(cr, pill.x, 20, 96, 28, 4);
    setRgb(cr, active ? kAccent : Rgb{0.20, 0.17, 0.14});
    cairo_fill(cr);
    text(cr, pill.label, pill.x + 48, 39, 9, 1,
         active ? 0.10 : kDim.r, active ? 0.08 : kDim.g,
         active ? 0.05 : kDim.b);
  }
}

void Panel::drawRack(cairo_t* cr, const Shared& shared,
                     const PanelState& state) const {
  char pageLabel[48];
  std::snprintf(pageLabel, sizeof(pageLabel), "PEDALBOARD  ·  PAGE %d",
                state.boardPage + 1);
  text(cr, pageLabel, 520, 118, 15, 1, kText.r, kText.g, kText.b);
  {
    char tempo[48];
    if (state.hostTempo) {
      std::snprintf(tempo, sizeof(tempo), "HOST %.0f BPM", state.bpm);
    } else {
      std::snprintf(tempo, sizeof(tempo), "FREE %.0f BPM", state.bpm);
    }
    text(cr, tempo, 690, 679, 9, 0, kDim.r, kDim.g, kDim.b);
    if (shared.get(P_SYNC) > 0.5 && !state.hostTempo) {
      text(cr, "NO HOST TEMPO", 690, 692, 7, 0, 0.85f, 0.55f, 0.35f);
    }
  }

  const double colW = kPedalWidth, gap = kPedalGap, y = kPedalY, h = kPedalH;
  const int base = state.boardPage * 4;

  // Patch cables run through the gaps between pedals.
  for (int i = 0; i < 3; ++i) {
    const double x1 = kPedalStartX + i * (colW + gap) + colW + 1.0;
    const double x2 = x1 + gap - 2.0;
    const double yj = y + 300.0;
    cairo_move_to(cr, x1, yj);
    cairo_curve_to(cr, x1 + 5.0, yj + 11.0, x2 - 5.0, yj + 11.0, x2, yj);
    cairo_set_source_rgba(cr, 0.015, 0.015, 0.018, 0.95);
    cairo_set_line_width(cr, 4.5);
    cairo_stroke_preserve(cr);
    cairo_set_source_rgba(cr, 0.20, 0.21, 0.23, 0.55);
    cairo_set_line_width(cr, 1.2);
    cairo_stroke(cr);
  }

  for (int i = 0; i < 4; ++i) {
    const int slot = base + i;
    const double x = kPedalStartX + i * (colW + gap);
    const int type = static_cast<int>(
        shared.get(slotParam(slot, kSlotOffsetType)) + 0.5);
    const ModuleStyle st = styleFor(type);

    roundRect(cr, x, y, colW, h, 8);
    cairo_pattern_t* pat = cairo_pattern_create_linear(0, y, 0, y + h);
    cairo_pattern_add_color_stop_rgb(pat, 0.0, st.body.r * 1.35,
                                     st.body.g * 1.35, st.body.b * 1.35);
    cairo_pattern_add_color_stop_rgb(pat, 1.0, st.body.r * 0.55,
                                     st.body.g * 0.55, st.body.b * 0.55);
    cairo_set_source(cr, pat);
    cairo_fill(cr);
    cairo_pattern_destroy(pat);
    setRgb(cr, {0, 0, 0}, 0.38);
    cairo_set_line_width(cr, 1.0);
    roundRect(cr, x + 0.5, y + 0.5, colW - 1, h - 1, 8);
    cairo_stroke(cr);

    const bool dropTarget = slotDragging_ && dropSlot_ == slot;
    cairo_set_line_width(
        cr, dropTarget ? 2.5 : (state.browserSlot == slot ? 2.0 : 1.0));
    setRgb(cr,
           dropTarget ? st.accent
                      : (state.browserSlot == slot ? st.accent : st.edge),
           dropTarget ? 1.0 : (state.browserSlot == slot ? 1.0 : 0.65));
    roundRect(cr, x + 0.5, y + 0.5, colW - 1, h - 1, 8);
    cairo_stroke(cr);

    char num[24];
    std::snprintf(num, sizeof(num), "PEDAL %d", slot + 1);
    text(cr, num, x + 12, y + 22, 7.5, 0, 0.90, 0.90, 0.86);

    if (type == static_cast<int>(ModuleId::Off)) {
      text(cr, "EMPTY", x + colW * 0.5, y + 190, 15, 1, 0.92, 0.92, 0.88);
      text(cr, "CHOOSE A", x + colW * 0.5, y + 216, 7, 1, 0.86, 0.86, 0.82);
      text(cr, "PEDAL", x + colW * 0.5, y + 229, 7, 1, 0.86, 0.86, 0.82);
    } else {
      text(cr, slotModuleName(type), x + colW * 0.5, y + 38, 15, 1, 0.95,
           0.95, 0.92);
      if (moduleHasGr(type)) {
        drawGrMeter(cr, x + 28, y + 54, colW - 56, 6, state.slotGrDb[slot]);
      }
      if (type == static_cast<int>(ModuleId::Nam) &&
          namRankForSlot(shared, slot) >= kMaxNamSlots) {
        text(cr, "2 AMP LIMIT", x + colW * 0.5, y + 58, 8, 1, 1.0, 0.45, 0.34);
      }
      if (type == static_cast<int>(ModuleId::Cab) &&
          shared.get(slotParam(slot, kSlotOffsetBypass)) <= 0.5 &&
          cabRankForSlot(shared, slot) >= kMaxCabSlots) {
        text(cr, "2 CAB LIMIT", x + colW * 0.5, y + 58, 8, 1, 1.0, 0.45, 0.34);
      }

    }
  }

  for (int i = 0; i < 4; ++i) {
    const double x = kPedalStartX + i * (colW + gap);
    const double yj = y + 300.0;
    for (double jx : {x + 1.5, x + colW - 1.5}) {
      cairo_arc(cr, jx, yj, 5.0, 0, 2 * kPi);
      setRgb(cr, {0.12, 0.12, 0.13});
      cairo_fill(cr);
      cairo_arc(cr, jx, yj, 3.2, 0, 2 * kPi);
      setRgb(cr, {0.42, 0.44, 0.46});
      cairo_fill(cr);
    }
  }
}

int Panel::browserOptionCount(int category, const std::string& query,
                              int presetFilterType) const {
  if (category == 7) {
    const std::string needle = lowerText(query);
    int count = 0;
    for (int i = 0; i < kFactoryModulePresetCount; ++i) {
      if (presetFilterType >= 0 &&
          static_cast<int>(kFactoryModulePresets[i].type) != presetFilterType) {
        continue;
      }
      if (!needle.empty() &&
          lowerText(kFactoryModulePresets[i].name).find(needle) ==
              std::string::npos) {
        continue;
      }
      ++count;
    }
    for (const auto& p : modulePresets_) {
      if (presetFilterType >= 0 && p.type != presetFilterType) continue;
      if (!needle.empty() &&
          lowerText(p.name).find(needle) == std::string::npos) {
        continue;
      }
      ++count;
    }
    return count;
  }
  return browserEntryCount(category, query);
}

double Panel::browserHeight(int category, const std::string& query,
                            int presetFilterType) const {
  const int count = browserOptionCount(category, query, presetFilterType);
  const int visible =
      query.empty() ? std::min(count, 16) : browserVisibleCount(query);
  const int rows = std::max(1, (visible + 3) / 4);
  return 92.0 + rows * 54.0 + 70.0;
}

double Panel::browserY(int category, const std::string& query,
                       int presetFilterType) const {
  const double h = browserHeight(category, query, presetFilterType);
  return std::max(84.0, (720.0 - h) * 0.5);
}

void Panel::drawBrowser(cairo_t* cr, const Shared& shared,
                        const PanelState& state) const {
  if (state.browserSlot < 0 || state.browserSlot >= kSlotCount) return;
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.64);
  cairo_paint(cr);
  const double bx = 240, bw = 560;
  const double bh = browserHeight(state.browserCategory, state.browserQuery,
                                  presetFilterTypeFor(shared, state));
  const double by = browserY(state.browserCategory, state.browserQuery,
                             presetFilterTypeFor(shared, state));
  roundRect(cr, bx, by, bw, bh, 6);
  setRgb(cr, {0.13, 0.115, 0.10}); cairo_fill(cr);
  setRgb(cr, kAccent, 0.85); cairo_set_line_width(cr, 1.5);
  roundRect(cr, bx + 0.5, by + 0.5, bw - 1, bh - 1, 6); cairo_stroke(cr);
  char title[48];
  std::snprintf(title, sizeof(title), "CHOOSE PEDAL · PEDAL %d",
                state.browserSlot + 1);
  text(cr, title, bx + bw * 0.5, by + 30, 14, 1, kText.r, kText.g, kText.b);
  const char* subtitle =
      state.browserCategory == 7
          ? (state.browserPresetFilter
                 ? "Presets for the loaded module · click one to load"
                 : "Factory and user module presets · click one to load")
          : "One job per pedal · 3–6 controls · no deep pages";
  text(cr, subtitle, bx + bw * 0.5, by + 86, 8.5, 1, kDim.r, kDim.g, kDim.b);
  if (state.browserCategory == 7 &&
      browserOptionCount(6, state.browserQuery,
                         presetFilterTypeFor(shared, state)) == 0) {
    text(cr, "No presets for this module yet", bx + bw * 0.5, by + 160, 10, 1,
         kDim.r, kDim.g, kDim.b);
  }
  const double tabW = 64.0, tabGap = 2.0;
  const double tabTotal = 8 * tabW + 7 * tabGap;
  const double tx = bx + (bw - tabTotal) * 0.5 - 8;
  const double ty = by + 46;
  cairo_set_source_rgba(cr, 0.20, 0.17, 0.14, 0.7);
  cairo_rectangle(cr, tx, ty, tabTotal + 16, 24);
  cairo_fill(cr);

  // Search field, sharing the bottom row with UP/DOWN and CANCEL.
  const double sx = bx + 150, sy = by + bh - 42, sw = bw - 290, sh = 28;
  setRgb(cr, {0.07, 0.065, 0.06});
  roundRect(cr, sx, sy, sw, sh, 4);
  cairo_fill(cr);
  setRgb(cr, state.browserSearch ? kAccent : kEdge, 0.8);
  cairo_set_line_width(cr, 1.0);
  roundRect(cr, sx + 0.5, sy + 0.5, sw - 1, sh - 1, 4);
  cairo_stroke(cr);
  std::string shown =
      state.browserQuery.empty() ? "Search modules" : state.browserQuery;
  if (state.browserSearch && cursorVisible()) shown += "_";
  text(cr, shown.c_str(), sx + 10, sy + 18, 9.0, 0,
       state.browserQuery.empty() ? kDim.r : kText.r,
       state.browserQuery.empty() ? kDim.g : kText.g,
       state.browserQuery.empty() ? kDim.b : kText.b);
}

int Panel::aboutMaxScroll() const {
  const int total = kModuleCount - 1;
  const int visible = 26;  // 13 rows x 2 columns
  if (total <= visible) return 0;
  return (total - visible + 1) / 2;
}

void Panel::drawBrowserMenu(cairo_t* cr, const PanelState& state) const {
  if (state.modulePresetMenu < 0 ||
      state.modulePresetMenu >= static_cast<int>(modulePresets_.size())) {
    return;
  }
  const ModulePreset& preset =
      modulePresets_[static_cast<size_t>(state.modulePresetMenu)];
  const double x = 340.0, y = 252.0, w = 300.0, h = 150.0;
  setRgb(cr, {0.0, 0.0, 0.0}, 0.45);
  cairo_paint(cr);
  setRgb(cr, {0.02, 0.02, 0.03}, 0.55);
  roundRect(cr, x + 4, y + 5, w, h, 6);
  cairo_fill(cr);
  setRgb(cr, {0.12, 0.105, 0.09});
  roundRect(cr, x, y, w, h, 6);
  cairo_fill(cr);
  setRgb(cr, kAccent, 0.9);
  cairo_set_line_width(cr, 1.3);
  roundRect(cr, x + 0.5, y + 0.5, w - 1, h - 1, 6);
  cairo_stroke(cr);

  text(cr, "PEDAL PRESET", x + w * 0.5, y + 24, 11, 1, kText.r, kText.g,
       kText.b);
  text(cr, preset.name.c_str(), x + w * 0.5, y + 50, 10, 1, kAccent.r,
       kAccent.g, kAccent.b);

  const double by = y + 84;
  roundRect(cr, x + 16, by, 80, 26, 4);
  setRgb(cr, {0.20, 0.17, 0.14});
  cairo_fill(cr);
  text(cr, "RENAME", x + 56, by + 17, 8, 1, kText.r, kText.g, kText.b);
  roundRect(cr, x + 104, by, 92, 26, 4);
  setRgb(cr, state.deleteModuleConfirm ? kAccent : Rgb{0.20, 0.17, 0.14});
  cairo_fill(cr);
  text(cr, state.deleteModuleConfirm ? "CONFIRM" : "DELETE", x + 150, by + 17,
       8, 1, state.deleteModuleConfirm ? 0.10 : kText.r,
       state.deleteModuleConfirm ? 0.08 : kText.g,
       state.deleteModuleConfirm ? 0.05 : kText.b);
  roundRect(cr, x + 204, by, 80, 26, 4);
  setRgb(cr, {0.12, 0.11, 0.10});
  cairo_fill(cr);
  text(cr, "CANCEL", x + 244, by + 17, 8, 1, kText.r, kText.g, kText.b);
}

void Panel::drawPresetList(cairo_t* cr, const Shared& /*shared*/,
                           const PanelState& state) const {
  if (!state.presetOpen) return;
  const std::vector<int> order = presetOrderForBank(state.presetBank);
  const int count = static_cast<int>(order.size());
  const int rows = presetRowsFor(count);
  const int maxScroll = std::max(0, count - rows);
  const int scroll = std::clamp(state.presetScroll, 0, maxScroll);
  const double h = kPresetHeaderH + rows * kPresetRowH +
                   kPresetBottomGap + kPresetFooterH;

  setRgb(cr, {0.02, 0.02, 0.03}, 0.55);
  roundRect(cr, kPresetListX + 4, kPresetListY + 5, kPresetListW, h, 6);
  cairo_fill(cr);
  setRgb(cr, {0.12, 0.105, 0.09});
  roundRect(cr, kPresetListX, kPresetListY, kPresetListW, h, 6);
  cairo_fill(cr);
  setRgb(cr, kAccent, 0.9);
  cairo_set_line_width(cr, 1.3);
  roundRect(cr, kPresetListX + 0.5, kPresetListY + 0.5, kPresetListW - 1,
            h - 1, 6);
  cairo_stroke(cr);

  text(cr, "BOARD PRESETS", kPresetListX + 14, kPresetListY + 20, 12, 0,
       kText.r, kText.g, kText.b);
  text(cr, "Factory boards and your saved boards", kPresetListX + 14,
       kPresetListY + 34, 7.0, 0, kDim.r, kDim.g, kDim.b);

  const std::vector<std::string> banks = presetBankNames();
  const int bankIndex =
      std::clamp(state.presetBank, 0, static_cast<int>(banks.size()) - 1);
  char bankLabel[48];
  std::snprintf(bankLabel, sizeof(bankLabel), "BANK: %s",
                banks[static_cast<size_t>(bankIndex)].c_str());
  roundRect(cr, kPresetListX + kPresetListW - 190, kPresetListY + 8, 112, 20,
            3);
  setRgb(cr, {0.20, 0.17, 0.14});
  cairo_fill(cr);
  text(cr, bankLabel, kPresetListX + kPresetListW - 134, kPresetListY + 22, 6.5,
       1, kText.r, kText.g, kText.b);
  roundRect(cr, kPresetListX + kPresetListW - 72, kPresetListY + 8, 58, 20, 3);
  setRgb(cr, {0.20, 0.17, 0.14});
  cairo_fill(cr);
  text(cr, "SETLIST", kPresetListX + kPresetListW - 43, kPresetListY + 22, 6.5,
       1, kText.r, kText.g, kText.b);

  for (int row = 0; row < rows; ++row) {
    const int index = scroll + row;
    if (index >= count) break;
    const int actual = order[static_cast<size_t>(index)];
    const double y = kPresetListY + kPresetHeaderH + row * kPresetRowH;
    const bool current = actual == state.presetIndex;
    if (current) {
      setRgb(cr, kAccent, 0.22);
      cairo_rectangle(cr, kPresetListX + 6, y, kPresetListW - 12,
                      kPresetRowH - 1);
      cairo_fill(cr);
    }
    text(cr, presetName(actual), kPresetListX + 14, y + 13, 8.5, 0,
         current ? kAccent.r : kText.r, current ? kAccent.g : kText.g,
         current ? kAccent.b : kText.b);
    text(cr, actual < kPresetCount ? kPresets[actual].category : "User",
         kPresetListX + kPresetListW - 14, y + 13, 6.5, 2,
         current ? kAccent.r : kDim.r, current ? kAccent.g : kDim.g,
         current ? kAccent.b : kDim.b);
    if (row < rows - 1) {
      setRgb(cr, kEdge, 0.18);
      cairo_set_line_width(cr, 0.6);
      cairo_move_to(cr, kPresetListX + 14, y + kPresetRowH - 1);
      cairo_line_to(cr, kPresetListX + kPresetListW - 14,
                    y + kPresetRowH - 1);
      cairo_stroke(cr);
    }
  }

  const double footerY =
      kPresetListY + kPresetHeaderH + rows * kPresetRowH + kPresetBottomGap + 4;
  if (maxScroll > 0) {
    roundRect(cr, kPresetListX + 14, footerY, 54, 24, 4);
    setRgb(cr, scroll > 0 ? Rgb{0.20, 0.17, 0.14} : Rgb{0.12, 0.11, 0.10});
    cairo_fill(cr);
    text(cr, "UP", kPresetListX + 41, footerY + 16, 8, 1, kText.r, kText.g,
         kText.b);
    roundRect(cr, kPresetListX + 74, footerY, 54, 24, 4);
    setRgb(cr, scroll < maxScroll ? Rgb{0.20, 0.17, 0.14}
                                  : Rgb{0.12, 0.11, 0.10});
    cairo_fill(cr);
    text(cr, "DOWN", kPresetListX + 101, footerY + 16, 8, 1, kText.r,
         kText.g, kText.b);
  }
  const bool userCurrent = state.presetIndex >= kPresetCount &&
                           state.presetIndex < count;
  if (userCurrent) {
    roundRect(cr, kPresetListX + 140, footerY, 68, 24, 4);
    setRgb(cr, {0.20, 0.17, 0.14});
    cairo_fill(cr);
    text(cr, "RENAME", kPresetListX + 174, footerY + 16, 7.5, 1, kText.r,
         kText.g, kText.b);
    roundRect(cr, kPresetListX + 214, footerY, 72, 24, 4);
    setRgb(cr, state.deletePresetConfirm ? kAccent : Rgb{0.20, 0.17, 0.14});
    cairo_fill(cr);
    text(cr, state.deletePresetConfirm ? "CONFIRM" : "DELETE",
         kPresetListX + 250, footerY + 16, 7.5, 1,
         state.deletePresetConfirm ? 0.10 : kText.r,
         state.deletePresetConfirm ? 0.08 : kText.g,
         state.deletePresetConfirm ? 0.05 : kText.b);
  }
  roundRect(cr, kPresetListX + kPresetListW - 84, footerY, 70, 24, 4);
  setRgb(cr, {0.20, 0.17, 0.14});
  cairo_fill(cr);
  text(cr, "CLOSE", kPresetListX + kPresetListW - 49, footerY + 16, 8, 1,
       kText.r, kText.g, kText.b);
}

void Panel::drawAbout(cairo_t* cr, const Shared& /*shared*/,
                      const PanelState& state) const {
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.72);
  cairo_paint(cr);
  const double bx = 100, by = 50, bw = 840, bh = 650;
  roundRect(cr, bx, by, bw, bh, 7);
  setRgb(cr, {0.12, 0.105, 0.09});
  cairo_fill(cr);
  setRgb(cr, kAccent, 0.9);
  cairo_set_line_width(cr, 1.5);
  roundRect(cr, bx + 0.5, by + 0.5, bw - 1, bh - 1, 7);
  cairo_stroke(cr);

  text(cr, "ABOUT PEDAL SUITE", 520, 92, 20, 1, kText.r, kText.g, kText.b);
  text(cr, "Virtual Guitar & Bass Pedalboard · pedals, types and character",
       520, 114, 8, 1, kDim.r, kDim.g, kDim.b);

  const double colX[2] = {120.0, 530.0};
  const double headerY = 144.0;
  for (int col = 0; col < 2; ++col) {
    text(cr, "PEDAL", colX[col], headerY, 7, 0, kDim.r, kDim.g, kDim.b);
    text(cr, "TYPE", colX[col] + 104, headerY, 7, 0, kDim.r, kDim.g, kDim.b);
    text(cr, "BASED ON / CHARACTER", colX[col] + 218, headerY, 7, 0, kDim.r,
         kDim.g, kDim.b);
  }

  const int rows = 13;
  const int start = state.aboutScroll * 2;
  for (int row = 0; row < rows; ++row) {
    for (int col = 0; col < 2; ++col) {
      const int index = start + row * 2 + col;
      const int id = index + 1;
      if (id >= kModuleCount) continue;
      const double x = colX[col];
      const double y = 162.0 + row * 26.0;
      const ModuleDesc* d = moduleDesc(id);
      const ModuleStyle st = styleFor(id);
      text(cr, d->name, x, y, 8.5, 0, st.accent.r, st.accent.g, st.accent.b);
      text(cr, d->category, x + 104, y, 6.8, 0, kDim.r, kDim.g, kDim.b);
      text(cr, d->description, x + 218, y, 6.5, 0, kText.r, kText.g,
           kText.b);
      setRgb(cr, kEdge, 0.20);
      cairo_set_line_width(cr, 0.6);
      cairo_move_to(cr, x, y + 8);
      cairo_line_to(cr, x + 380, y + 8);
      cairo_stroke(cr);
    }
  }

  const int maxScroll = aboutMaxScroll();
  if (maxScroll > 0) {
    roundRect(cr, 120, 604, 58, 26, 4);
    setRgb(cr, state.aboutScroll > 0 ? Rgb{0.20, 0.17, 0.14}
                                     : Rgb{0.12, 0.11, 0.10});
    cairo_fill(cr);
    text(cr, "UP", 149, 621, 8, 1, kText.r, kText.g, kText.b);
    roundRect(cr, 184, 604, 58, 26, 4);
    setRgb(cr, state.aboutScroll < maxScroll ? Rgb{0.20, 0.17, 0.14}
                                             : Rgb{0.12, 0.11, 0.10});
    cairo_fill(cr);
    text(cr, "DOWN", 213, 621, 8, 1, kText.r, kText.g, kText.b);
    char pos[64];
    const int first = start + 1;
    const int last = std::min(start + rows * 2, kModuleCount - 1);
    std::snprintf(pos, sizeof(pos), "Pedals %d-%d of %d  ·  scroll wheel",
                  first, last, kModuleCount - 1);
    text(cr, pos, 262, 621, 7, 0, kDim.r, kDim.g, kDim.b);
  }

  text(cr, "Click a pedal name or empty card to open its browser. Drag it to reorder pedals.",
       120, 520, 7.5, 0, kText.r, kText.g, kText.b);
  text(cr, "Double-click a knob to reset it. Right-click a knob to type a value.",
       120, 534, 7.5, 0, kText.r, kText.g, kText.b);
  text(cr, "MIDI: notes 36-43 / 60-67 or CC 20-27 / 80-87 toggle pedals; CC 4 / 11 moves the wah.",
       120, 550, 7.0, 0, kDim.r, kDim.g, kDim.b);
  text(cr, "GPL-3.0-or-later. See THIRD_PARTY_NOTICES.md for credits.",
       120, 566, 7.0, 0, kDim.r, kDim.g, kDim.b);
  text(cr, "Jost font: SIL OFL 1.1. CLAP headers: MIT.",
       120, 582, 7.0, 0, kDim.r, kDim.g, kDim.b);

  roundRect(cr, 820, 654, 80, 26, 4);
  setRgb(cr, kAccent);
  cairo_fill(cr);
  text(cr, "CLOSE", 860, 672, 9, 1, 0.10, 0.08, 0.05);
}


void Panel::drawTuner(cairo_t* cr, const Shared& shared,
                      const PanelState& /*state*/) const {
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.72);
  cairo_paint(cr);
  const double bx = 200, by = 120, bw = 640, bh = 480;
  roundRect(cr, bx, by, bw, bh, 8);
  setRgb(cr, {0.10, 0.095, 0.085});
  cairo_fill(cr);
  setRgb(cr, kAccent, 0.85);
  cairo_set_line_width(cr, 1.5);
  roundRect(cr, bx + 0.5, by + 0.5, bw - 1, bh - 1, 8);
  cairo_stroke(cr);

  text(cr, "TUNER", 520, by + 42, 22, 1, kText.r, kText.g, kText.b);
  text(cr, "A4 = 440 Hz  ·  monophonic", 520, by + 62, 8, 1, kDim.r,
       kDim.g, kDim.b);

  const float hz = shared.tunerHz.load(std::memory_order_relaxed);
  const float cents = shared.tunerCents.load(std::memory_order_relaxed);
  const float conf =
      shared.tunerConfidence.load(std::memory_order_relaxed);
  const int note = shared.tunerNote.load(std::memory_order_relaxed);
  const bool tuned = note >= 0 && conf > 0.6f;

  if (!tuned) {
    text(cr, "PLAY A NOTE", 520, by + 190, 34, 1, 0.55, 0.54, 0.50);
    text(cr, "Waiting for a stable pitch", 520, by + 220, 9, 1, kDim.r,
         kDim.g, kDim.b);
  } else {
    static const char* kNames[12] = {"C",  "C#", "D",  "D#", "E",  "F",
                                     "F#", "G",  "G#", "A",  "A#", "B"};
    const int pc = ((note % 12) + 12) % 12;
    const int octave = note / 12 - 1;
    char noteBuf[16];
    std::snprintf(noteBuf, sizeof(noteBuf), "%d", octave);
    setRgb(cr, std::fabs(cents) < 5.0f ? Rgb{0.40, 0.92, 0.50}
                                       : Rgb{0.98, 0.78, 0.34});
    text(cr, kNames[pc], 512, by + 180, 96, 2, kText.r, kText.g, kText.b);
    text(cr, noteBuf, 600, by + 180, 26, 0, kDim.r, kDim.g, kDim.b);
  }

  const double cx = 520.0, cy = by + 250.0, half = 240.0;
  setRgb(cr, kEdge, 0.8);
  cairo_set_line_width(cr, 1.2);
  cairo_move_to(cr, cx - half, cy);
  cairo_line_to(cr, cx + half, cy);
  cairo_stroke(cr);
  for (int tick = -5; tick <= 5; ++tick) {
    const double tx = cx + half * (tick / 5.0);
    const bool major = tick == 0 || std::abs(tick) == 5;
    setRgb(cr, tick == 0 ? kAccent : kEdge, tick == 0 ? 1.0 : 0.7);
    cairo_set_line_width(cr, tick == 0 ? 2.0 : 1.0);
    cairo_move_to(cr, tx, cy - (major ? 14.0 : 8.0));
    cairo_line_to(cr, tx, cy + (major ? 14.0 : 8.0));
    cairo_stroke(cr);
  }
  setRgb(cr, {0.30, 0.85, 0.45}, 0.18);
  cairo_rectangle(cr, cx - half * 0.10, cy - 18, half * 0.20, 36);
  cairo_fill(cr);

  const double clamped = std::clamp(static_cast<double>(cents), -50.0, 50.0);
  const double mx = cx + half * (clamped / 50.0);
  setRgb(cr, std::fabs(cents) < 5.0f ? Rgb{0.40, 0.92, 0.50}
                                     : Rgb{0.98, 0.70, 0.30});
  cairo_arc(cr, mx, cy, 7.0, 0, 2 * kPi);
  cairo_fill(cr);

  char buf[64];
  if (tuned) {
    std::snprintf(buf, sizeof(buf), "%+.1f cents", cents);
    text(cr, buf, 520, by + 300, 18, 1, kText.r, kText.g, kText.b);
    std::snprintf(buf, sizeof(buf), "%.1f Hz", hz);
    text(cr, buf, 520, by + 326, 11, 1, kDim.r, kDim.g, kDim.b);
  } else {
    text(cr, "—", 520, by + 306, 22, 1, kDim.r, kDim.g, kDim.b);
  }

  // Confidence bar.
  roundRect(cr, 370, by + 360, 300, 8, 3);
  setRgb(cr, {0.16, 0.15, 0.13});
  cairo_fill(cr);
  roundRect(cr, 371, by + 361, 298 * std::clamp(conf, 0.0f, 1.0f), 6, 3);
  setRgb(cr, conf > 0.6f ? Rgb{0.40, 0.90, 0.50} : Rgb{0.80, 0.55, 0.25});
  cairo_fill(cr);
  text(cr, "CONFIDENCE", 370, by + 353, 6.5, 0, kDim.r, kDim.g, kDim.b);

  roundRect(cr, 720, 548, 80, 26, 4);
  setRgb(cr, kAccent);
  cairo_fill(cr);
  text(cr, "CLOSE", 760, 565, 9, 1, 0.10, 0.08, 0.05);
}


void Panel::drawMidi(cairo_t* cr, const Shared& shared,
                     const PanelState& state) const {
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.64);
  cairo_paint(cr);
  const double bx = 180, by = 100, bw = 680, bh = 540;
  roundRect(cr, bx, by, bw, bh, 6);
  setRgb(cr, {0.13, 0.115, 0.10});
  cairo_fill(cr);
  setRgb(cr, kAccent, 0.85);
  cairo_set_line_width(cr, 1.5);
  roundRect(cr, bx + 0.5, by + 0.5, bw - 1, bh - 1, 6);
  cairo_stroke(cr);

  const bool midiOn = shared.get(P_MIDI_ENABLE) > 0.5;
  roundRect(cr, bx + 24, by + 16, 168, 26, 4);
  setRgb(cr, midiOn ? kAccent : Rgb{0.20, 0.17, 0.14});
  cairo_fill(cr);
  text(cr, midiOn ? "MIDI CONTROL  ON" : "MIDI CONTROL  OFF", bx + 108,
       by + 33, 8.5, 1, midiOn ? 0.10 : kDim.r, midiOn ? 0.08 : kDim.g,
       midiOn ? 0.05 : kDim.b);
  text(cr, "ARM NOTE 0", bx + bw - 24, by + 33, 8, 2, kDim.r, kDim.g, kDim.b);

  text(cr, "MIDI MAPPING", bx + bw * 0.5, by + 36, 18, 1, kText.r, kText.g,
       kText.b);
  text(cr, "Learn a note or CC per pedal · expression maps one CC to a control",
       bx + bw * 0.5, by + 58, 8.5, 1, kDim.r, kDim.g, kDim.b);

  text(cr, "PEDAL", bx + 30, by + 86, 8, 0, kDim.r, kDim.g, kDim.b);
  text(cr, "NOTE", bx + 230, by + 86, 8, 0, kDim.r, kDim.g, kDim.b);
  text(cr, "CC", bx + 330, by + 86, 8, 0, kDim.r, kDim.g, kDim.b);
  text(cr, "LEARN", bx + 466, by + 86, 8, 0, kDim.r, kDim.g, kDim.b);

  for (int slot = 0; slot < kSlotCount; ++slot) {
    const double ry = by + 106 + slot * 34;
    if (slot % 2 == 0) {
      setRgb(cr, {0.10, 0.09, 0.08}, 0.65);
      roundRect(cr, bx + 18, ry - 14, bw - 36, 28, 3);
      cairo_fill(cr);
    }
    const int type = static_cast<int>(
        shared.get(slotParam(slot, kSlotOffsetType)) + 0.5);
    char name[64];
    std::snprintf(name, sizeof(name), "PEDAL %d  ·  %s", slot + 1,
                  slotModuleName(type));
    text(cr, name, bx + 30, ry + 4, 9, 0, kText.r, kText.g, kText.b);

    char note[32];
    std::snprintf(note, sizeof(note), "NOTE %d",
                  static_cast<int>(std::lround(shared.get(midiNoteParam(slot)))));
    text(cr, note, bx + 230, ry + 4, 9, 0, kText.r, kText.g, kText.b);
    char cc[32];
    std::snprintf(cc, sizeof(cc), "CC %d",
                  static_cast<int>(std::lround(shared.get(midiCcParam(slot)))));
    text(cr, cc, bx + 330, ry + 4, 9, 0, kText.r, kText.g, kText.b);

    const bool learning = state.midiLearnSlot == slot;
    roundRect(cr, bx + 458, ry - 12, 100, 24, 4);
    setRgb(cr, learning ? kAccent : Rgb{0.20, 0.17, 0.14});
    cairo_fill(cr);
    text(cr, learning ? "MOVE NOTE / CC" : "LEARN", bx + 508, ry + 4, 8, 1,
         learning ? 0.10 : kDim.r, learning ? 0.08 : kDim.g,
         learning ? 0.05 : kDim.b);
  }

  const double ey = by + 410;
  setRgb(cr, kEdge, 0.55);
  cairo_set_line_width(cr, 1.0);
  cairo_move_to(cr, bx + 30, ey - 24);
  cairo_line_to(cr, bx + bw - 30, ey - 24);
  cairo_stroke(cr);

  text(cr, "EXPRESSION", bx + 30, ey + 4, 8, 0, kDim.r, kDim.g, kDim.b);
  const bool exprOn = shared.get(P_MIDI_EXPR_ON) > 0.5;
  roundRect(cr, bx + 126, ey - 12, 74, 24, 4);
  setRgb(cr, exprOn ? kAccent : Rgb{0.20, 0.17, 0.14});
  cairo_fill(cr);
  text(cr, exprOn ? "ON" : "OFF", bx + 163, ey + 4, 8, 1,
       exprOn ? 0.10 : kDim.r, exprOn ? 0.08 : kDim.g, exprOn ? 0.05 : kDim.b);

  char ccLabel[24];
  std::snprintf(ccLabel, sizeof(ccLabel), "CC %d", expressionCc(shared));
  roundRect(cr, bx + 216, ey - 12, 74, 24, 4);
  setRgb(cr, {0.16, 0.14, 0.12});
  cairo_fill(cr);
  text(cr, ccLabel, bx + 253, ey + 4, 8, 1, kText.r, kText.g, kText.b);

  const int targetSlot = expressionSlot(shared);
  char slotLabel[24];
  std::snprintf(slotLabel, sizeof(slotLabel), "PEDAL %d", targetSlot + 1);
  roundRect(cr, bx + 306, ey - 12, 92, 24, 4);
  setRgb(cr, {0.16, 0.14, 0.12});
  cairo_fill(cr);
  text(cr, slotLabel, bx + 352, ey + 4, 8, 1, kText.r, kText.g, kText.b);

  const int ctl = expressionControl(shared);
  const int targetType = static_cast<int>(
      shared.get(slotParam(targetSlot, kSlotOffsetType)) + 0.5);
  const ModuleDesc* targetModule = moduleDesc(targetType);
  const char* ctlLabel =
      targetModule->controlCount > 0 && ctl < targetModule->controlCount
          ? targetModule->controls[ctl].label
          : "—";
  roundRect(cr, bx + 414, ey - 12, 132, 24, 4);
  setRgb(cr, {0.16, 0.14, 0.12});
  cairo_fill(cr);
  text(cr, ctlLabel, bx + 480, ey + 4, 8, 1, kText.r, kText.g, kText.b);

  const bool learningExpr = state.midiLearnExpr;
  roundRect(cr, bx + 562, ey - 12, 92, 24, 4);
  setRgb(cr, learningExpr ? kAccent : Rgb{0.20, 0.17, 0.14});
  cairo_fill(cr);
  text(cr, learningExpr ? "MOVE CC" : "LEARN CC", bx + 608, ey + 4, 8, 1,
       learningExpr ? 0.10 : kDim.r, learningExpr ? 0.08 : kDim.g,
       learningExpr ? 0.05 : kDim.b);

  text(cr, "Click PEDAL or CONTROL to change the target. With expression off, CC 4/11 drive the first Wah.",
       bx + 30, ey + 40, 7.5, 0, kDim.r, kDim.g, kDim.b);
  text(cr, "PERFORMANCE: PC selects a setlist/bank item · CC 28/29 setlist · CC 30/31 bank",
       bx + 30, ey + 54, 7.0, 0, kDim.r, kDim.g, kDim.b);

  roundRect(cr, bx + bw - 110, by + bh - 40, 90, 24, 4);
  setRgb(cr, {0.20, 0.17, 0.14});
  cairo_fill(cr);
  text(cr, "CLOSE", bx + bw - 65, by + bh - 24, 8.5, 1, kDim.r, kDim.g,
       kDim.b);
}

void Panel::drawSetlist(cairo_t* cr, const Shared& /*shared*/,
                        const PanelState& state) const {
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.66);
  cairo_paint(cr);
  const double bx = 130, by = 60, bw = 780, bh = 600;
  roundRect(cr, bx, by, bw, bh, 7);
  setRgb(cr, {0.13, 0.115, 0.10});
  cairo_fill(cr);
  setRgb(cr, kAccent, 0.85);
  cairo_set_line_width(cr, 1.5);
  roundRect(cr, bx + 0.5, by + 0.5, bw - 1, bh - 1, 7);
  cairo_stroke(cr);

  text(cr, "SETLIST MANAGER", bx + bw * 0.5, by + 30, 18, 1, kText.r,
       kText.g, kText.b);
  text(cr, "Build ordered performance lists from factory and user boards",
       bx + bw * 0.5, by + 50, 8.5, 1, kDim.r, kDim.g, kDim.b);

  const double leftX = bx + 20, rightX = bx + 230;
  const double listY = by + 92;
  const int visible = 11;
  text(cr, "SETLISTS", leftX, by + 70, 8, 0, kDim.r, kDim.g, kDim.b);
  text(cr, "ENTRIES", rightX, by + 70, 8, 0, kDim.r, kDim.g, kDim.b);

  const int setlistCount = static_cast<int>(setlists_.size());
  const int maxScroll = std::max(0, setlistCount - visible);
  const int scroll = std::clamp(state.setlistScroll, 0, maxScroll);
  if (setlistCount == 0) {
    text(cr, "No setlists yet.", leftX, listY + 18, 9, 0, kDim.r, kDim.g,
         kDim.b);
    text(cr, "Click NEW to create one.", leftX, listY + 34, 8, 0, kDim.r,
         kDim.g, kDim.b);
  }
  for (int row = 0; row < visible && scroll + row < setlistCount; ++row) {
    const int index = scroll + row;
    const double ry = listY + row * 27;
    const bool active = index == state.setlistIndex;
    roundRect(cr, leftX - 4, ry - 13, 188, 24, 3);
    if (active) {
      setRgb(cr, kAccent, 0.24);
      cairo_fill(cr);
    }
    text(cr, setlists_[static_cast<size_t>(index)].name.c_str(),
         leftX + 4, ry + 4, 8.5, 0, active ? kAccent.r : kText.r,
         active ? kAccent.g : kText.g, active ? kAccent.b : kText.b);
    char count[24];
    std::snprintf(count, sizeof(count), "%d",
                  static_cast<int>(setlists_[static_cast<size_t>(index)]
                                       .entries.size()));
    text(cr, count, leftX + 174, ry + 4, 7, 2,
         active ? kAccent.r : kDim.r, active ? kAccent.g : kDim.g,
         active ? kAccent.b : kDim.b);
  }

  const Setlist* current =
      state.setlistIndex >= 0 && state.setlistIndex < setlistCount
          ? &setlists_[static_cast<size_t>(state.setlistIndex)]
          : nullptr;
  if (current) {
    std::string entryTitle = "ENTRIES · " + current->name;
    text(cr, entryTitle.c_str(), rightX, by + 70, 8, 0, kDim.r, kDim.g,
         kDim.b);
    const int entryCount = static_cast<int>(current->entries.size());
    const int entryMaxScroll = std::max(0, entryCount - 12);
    const int entryScroll =
        std::clamp(state.setlistEntryScroll, 0, entryMaxScroll);
    for (int row = 0; row < 12 && entryScroll + row < entryCount; ++row) {
      const int index = entryScroll + row;
      const double ry = listY + row * 25;
      const bool active = index == state.setlistEntry;
      roundRect(cr, rightX - 4, ry - 12, 530, 22, 3);
      if (active) {
        setRgb(cr, kAccent, 0.22);
        cairo_fill(cr);
      }
      char number[16];
      std::snprintf(number, sizeof(number), "%02d", index + 1);
      text(cr, number, rightX + 4, ry + 4, 7.5, 0,
           active ? kAccent.r : kDim.r, active ? kAccent.g : kDim.g,
           active ? kAccent.b : kDim.b);
      const SetlistEntry& entry =
          current->entries[static_cast<size_t>(index)];
      const bool snapshot = !entry.snapshot.empty();
      const int resolved = snapshot ? -2 : resolveSetlistEntry(entry);
      const bool missing = !snapshot && resolved < 0;
      text(cr, entry.name.c_str(), rightX + 34, ry + 4, 8.5, 0,
           missing ? 1.0 : kText.r, missing ? 0.45 : kText.g,
           missing ? 0.34 : kText.b);
      const char* tag = snapshot
                            ? "SNAPSHOT"
                            : (missing ? "MISSING"
                                       : (entry.factory ? "FACTORY" : "USER"));
      text(cr, tag, rightX + 520, ry + 4, 6.5, 2,
           missing ? 1.0 : kDim.r, missing ? 0.45 : kDim.g,
           missing ? 0.34 : kDim.b);
    }
    if (entryCount == 0) {
      text(cr, "No entries. Load a board, then click ADD CURRENT.",
           rightX + 4, listY + 18, 9, 0, kDim.r, kDim.g, kDim.b);
    }
  }

  const double leftFooter = by + bh - 42;
  auto button = [&](double x, double y, double w, const char* label,
                    bool lit, double size = 8.0) {
    roundRect(cr, x, y, w, 26, 4);
    setRgb(cr, lit ? kAccent : Rgb{0.20, 0.17, 0.14});
    cairo_fill(cr);
    text(cr, label, x + w * 0.5, y + 17, size, 1,
         lit ? 0.10 : kText.r, lit ? 0.08 : kText.g, lit ? 0.05 : kText.b);
  };
  button(leftX, leftFooter, 54, "NEW", false);
  button(leftX + 62, leftFooter, 64, "RENAME", false);
  button(leftX + 134, leftFooter, 54, "DELETE",
         state.deleteSetlistConfirm, 7.2);

  const double btnW = 115, btnGap = 8;
  const double row1 = by + bh - 86, row2 = by + bh - 52;
  button(rightX, row1, btnW, "ADD CURRENT", false, 7.2);
  button(rightX + (btnW + btnGap), row1, btnW, "REMOVE", false);
  button(rightX + 2 * (btnW + btnGap), row1, btnW, "MOVE UP", false);
  button(rightX + 3 * (btnW + btnGap), row1, btnW, "MOVE DOWN", false);
  button(rightX, row2, btnW, "PREVIOUS", false);
  button(rightX + (btnW + btnGap), row2, btnW, "NEXT", false);
  button(rightX + 2 * (btnW + btnGap), row2, btnW, "LOAD", false);
  button(rightX + 3 * (btnW + btnGap), row2, btnW, "CLOSE", false);

  text(cr, "Click an entry to load it. Setlists are saved automatically.",
       leftX, by + bh - 104, 7.5, 0, kDim.r, kDim.g, kDim.b);
}

void Panel::drawIrPicker(cairo_t* cr, const Shared& shared,
                         const PanelState& state) const {
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.72);
  cairo_paint(cr);
  const double bx = 260, by = 140, bw = 520, bh = 430;
  roundRect(cr, bx, by, bw, bh, 7);
  setRgb(cr, {0.12, 0.105, 0.09});
  cairo_fill(cr);
  setRgb(cr, kAccent, 0.9);
  cairo_set_line_width(cr, 1.5);
  roundRect(cr, bx + 0.5, by + 0.5, bw - 1, bh - 1, 7);
  cairo_stroke(cr);

  char title[64];
  std::snprintf(title, sizeof(title), "%s \u00b7 PEDAL %d",
                state.pickerNam ? "NAM AMP MODEL" : "CABINET IMPULSE",
                state.irPickerSlot + 1);
  text(cr, title, bx + bw * 0.5, by + 34, 14, 1, kText.r, kText.g, kText.b);
  text(cr, state.pickerNam ? "Neural amp models from the NAM folder"
                           : "WAV files from the IR folder",
       bx + bw * 0.5, by + 54, 8, 1, kDim.r, kDim.g, kDim.b);

  const int selected = static_cast<int>(std::lround(
      shared.get(state.pickerNam ? P_NAM_MODEL : P_CAB_IR)));
  const int visible = 11;
  const int count = static_cast<int>(state.irNames.size()) + 1;
  const int maxScroll = std::max(0, count - visible);
  const int scroll = std::clamp(state.irScroll, 0, maxScroll);
  const double rowStart = by + 70;
  for (int row = 0; row < visible; ++row) {
    const int index = scroll + row;
    if (index >= count) break;
    const double ry = rowStart + row * 26.0;
    const bool isSelected = index == selected;
    roundRect(cr, bx + 20, ry, bw - 40, 22, 3);
    setRgb(cr, isSelected ? Rgb{0.30, 0.24, 0.14} : Rgb{0.16, 0.145, 0.125});
    cairo_fill(cr);
    const std::string label =
        index == 0
            ? std::string(state.pickerNam ? "NONE (no amp model)"
                                          : "NONE (bypass the cabinet)")
            : state.irNames[static_cast<size_t>(index - 1)];
    text(cr, label.c_str(), bx + 34, ry + 15, 9, 0,
         isSelected ? kAccent.r : kText.r, isSelected ? kAccent.g : kText.g,
         isSelected ? kAccent.b : kText.b);
  }

  text(cr, prettyPath(state.pickerNam ? namDir() : irDir()).c_str(), bx + bw * 0.5,
       by + 384, 6.5, 1, kDim.r, kDim.g, kDim.b);
  roundRect(cr, bx + 20, by + 396, 58, 26, 4);
  setRgb(cr, scroll > 0 ? Rgb{0.20, 0.17, 0.14} : Rgb{0.12, 0.11, 0.10});
  cairo_fill(cr);
  text(cr, "UP", bx + 49, by + 413, 8, 1, kText.r, kText.g, kText.b);
  roundRect(cr, bx + 86, by + 396, 58, 26, 4);
  setRgb(cr, scroll < maxScroll ? Rgb{0.20, 0.17, 0.14}
                                : Rgb{0.12, 0.11, 0.10});
  cairo_fill(cr);
  text(cr, "DOWN", bx + 115, by + 413, 8, 1, kText.r, kText.g, kText.b);
  roundRect(cr, bx + bw - 100, by + 396, 80, 26, 4);
  setRgb(cr, kAccent);
  cairo_fill(cr);
  text(cr, "CLOSE", bx + bw - 60, by + 413, 9, 1, 0.10, 0.08, 0.05);
}

void Panel::draw(cairo_t* cr, Shared& shared, const PanelState& state) {
  layout(shared, state);
  cairo_set_source_rgb(cr, 0.07, 0.065, 0.06);
  cairo_paint(cr);
  drawHeader(cr, shared, state);
  drawRack(cr, shared, state);
  for (const Widget& w : widgets_) {
    if (w.kind != Kind::BrowserOption && w.kind != Kind::BrowserTab &&
        w.kind != Kind::BrowserScroll) {
      drawWidget(cr, w, shared, state);
    }
  }
  drawBrowser(cr, shared, state);
  for (const Widget& w : widgets_) {
    if (w.kind == Kind::BrowserOption || w.kind == Kind::BrowserTab ||
        w.kind == Kind::BrowserScroll) {
      drawWidget(cr, w, shared, state);
    }
  }
  if (state.modulePresetMenu >= 0) drawBrowserMenu(cr, state);
  if (slotDragging_) drawDragGhost(cr, shared);
  drawLevelMeter(cr, 20, 100, 14, 520, state.inLevelL, state.inLevelR);
  drawLevelMeter(cr, 1006, 100, 14, 520, state.outLevelL, state.outLevelR);
  // Utility buttons sit in the page-title row, above the pedals.
  roundRect(cr, kUtilityTunerX, kUtilityY, kUtilityW, kUtilityH, 4);
  setRgb(cr, state.tunerOpen ? kAccent : Rgb{0.20, 0.17, 0.14});
  cairo_fill(cr);
  text(cr, "TUNER", kUtilityTunerX + kUtilityW * 0.5, kUtilityY + 16, 8.5,
       1, state.tunerOpen ? 0.10 : kDim.r, state.tunerOpen ? 0.08 : kDim.g,
       state.tunerOpen ? 0.05 : kDim.b);
  roundRect(cr, kUtilityMidiX, kUtilityY, kUtilityW, kUtilityH, 4);
  setRgb(cr, state.midiOpen ? kAccent : Rgb{0.20, 0.17, 0.14});
  cairo_fill(cr);
  text(cr, "MIDI", kUtilityMidiX + kUtilityW * 0.5, kUtilityY + 16, 8.5, 1,
       state.midiOpen ? 0.10 : kDim.r, state.midiOpen ? 0.08 : kDim.g,
       state.midiOpen ? 0.05 : kDim.b);
  text(cr, "IN", 27, 640, 8, 1, kDim.r, kDim.g, kDim.b);
  text(cr, "OUT", 1013, 640, 8, 1, kDim.r, kDim.g, kDim.b);
  text(cr, "INPUT", 96, 681, 7.5, 2, kDim.r, kDim.g, kDim.b);
  text(cr, "OUTPUT", 904, 681, 7.5, 0, kDim.r, kDim.g, kDim.b);
  text(cr, "PEDAL SUITE · VIRTUAL GUITAR & BASS PEDALBOARD · EIGHT SLOTS", 520, 714, 10.5, 1,
       kText.r, kText.g, kText.b);
  if (state.presetOpen) drawPresetList(cr, shared, state);
  if (state.aboutOpen) drawAbout(cr, shared, state);
  if (state.tunerOpen) drawTuner(cr, shared, state);
  if (state.midiOpen) drawMidi(cr, shared, state);
  if (state.setlistOpen) drawSetlist(cr, shared, state);
  if (state.irPickerSlot >= 0) drawIrPicker(cr, shared, state);
  drawTextEntry(cr, state);
}

void Panel::drawWidget(cairo_t* cr, const Widget& w, const Shared& shared,
                       const PanelState& state) const {
  const int type = w.styleSlot >= 0
                       ? static_cast<int>(shared.get(slotParam(w.styleSlot, kSlotOffsetType)) + 0.5)
                       : -1;
  const ModuleStyle st = styleFor(type);
  switch (w.kind) {
    case Kind::Power: {
      // Shared Comp Suite power button, unchanged.
      const bool on = shared.get(P_POWER) > 0.5;
      cairo_arc(cr, w.x, w.y, w.r, 0, 2 * kPi);
      setRgb(cr, {0.12, 0.12, 0.12});
      cairo_fill(cr);
      cairo_arc(cr, w.x, w.y, w.r - 1.5, 0, 2 * kPi);
      setRgb(cr, on ? Rgb{0.36, 0.86, 0.46} : Rgb{0.42, 0.42, 0.42});
      cairo_set_line_width(cr, 2.2);
      cairo_new_path(cr);
      cairo_arc(cr, w.x, w.y, 7, -kPi / 2 + 0.6, -kPi / 2 + 2 * kPi - 0.6);
      cairo_stroke(cr);
      cairo_move_to(cr, w.x, w.y - 9);
      cairo_line_to(cr, w.x, w.y - 1);
      cairo_stroke(cr);
      break;
    }
    case Kind::SlotCard:
    case Kind::ModuleDragHandle:
      break;
    case Kind::ModuleName: {
      const int slot = static_cast<int>(w.value);
      const int type = static_cast<int>(
          shared.get(slotParam(slot, kSlotOffsetType)) + 0.5);
      roundRect(cr, w.x, w.y, w.w, w.h, 4);
      setRgb(cr, st.body); cairo_fill(cr);
      setRgb(cr, st.edge, 0.9); cairo_set_line_width(cr, 1.0);
      roundRect(cr, w.x + 0.5, w.y + 0.5, w.w - 1, w.h - 1, 4); cairo_stroke(cr);
      text(cr, slotModuleName(type), w.x + 12, w.y + w.h * 0.5 + 5, 12, 0,
           st.accent.r, st.accent.g, st.accent.b);
      if (type == static_cast<int>(ModuleId::Off)) {
        text(cr, "CHOOSE", w.x + w.w - 10, w.y + w.h * 0.5 + 4, 7, 2,
             kDim.r, kDim.g, kDim.b);
      }
      break;
    }
    case Kind::BrowserTab: {
      const bool active = static_cast<int>(w.value) == state.browserCategory;
      roundRect(cr, w.x, w.y, w.w, w.h, 4);
      setRgb(cr, active ? kAccent : Rgb{0.20, 0.17, 0.14});
      cairo_fill(cr);
      setRgb(cr, kEdge, 0.8);
      cairo_set_line_width(cr, 1.0);
      roundRect(cr, w.x + 0.5, w.y + 0.5, w.w - 1, w.h - 1, 4);
      cairo_stroke(cr);
      text(cr, w.label, w.x + w.w * 0.5, w.y + w.h * 0.5 + 4, 9, 1,
           active ? 0.10 : kText.r, active ? 0.08 : kText.g,
           active ? 0.05 : kText.b);
      break;
    }
    case Kind::BrowserScroll: {
      roundRect(cr, w.x, w.y, w.w, w.h, 4);
      setRgb(cr, {0.20, 0.17, 0.14});
      cairo_fill(cr);
      setRgb(cr, kEdge, 0.8);
      cairo_set_line_width(cr, 1.0);
      roundRect(cr, w.x + 0.5, w.y + 0.5, w.w - 1, w.h - 1, 4);
      cairo_stroke(cr);
      text(cr, w.label, w.x + w.w * 0.5, w.y + w.h * 0.5 + 4, 9, 1,
           kText.r, kText.g, kText.b);
      break;
    }
    case Kind::Footswitch: {
      const bool on = shared.get(w.param) <= 0.5;
      cairo_arc(cr, w.x, w.y, w.r + 4, 0, 2 * kPi);
      setRgb(cr, {0.03, 0.03, 0.03}, 0.55);
      cairo_fill(cr);
      cairo_pattern_t* pat = cairo_pattern_create_radial(
          w.x - w.r * 0.3, w.y - w.r * 0.35, w.r * 0.1, w.x, w.y, w.r);
      cairo_pattern_add_color_stop_rgb(pat, 0.0, 0.76, 0.78, 0.80);
      cairo_pattern_add_color_stop_rgb(pat, 1.0, 0.22, 0.23, 0.25);
      cairo_arc(cr, w.x, w.y, w.r, 0, 2 * kPi);
      cairo_set_source(cr, pat);
      cairo_fill(cr);
      cairo_pattern_destroy(pat);
      cairo_arc(cr, w.x, w.y, w.r * 0.55, 0, 2 * kPi);
      setRgb(cr, {0.10, 0.10, 0.11});
      cairo_fill(cr);
      cairo_arc(cr, w.x, w.y - w.r - 13, 5, 0, 2 * kPi);
      setRgb(cr, on ? st.accent
                    : Rgb{st.accent.r * 0.25, st.accent.g * 0.25,
                          st.accent.b * 0.25});
      cairo_fill(cr);
      text(cr, on ? "ON" : "BYPASS", w.x, w.y + w.r + 17, 8, 1,
           on ? 0.95 : 0.72, on ? 0.95 : 0.72, on ? 0.90 : 0.70);
      break;
    }
    case Kind::NamModelButton: {
      const bool open =
          state.irPickerSlot == static_cast<int>(w.value) && state.pickerNam;
      const bool loaded = shared.get(P_NAM_MODEL) > 0.5;
      roundRect(cr, w.x, w.y, w.w, w.h, 4);
      setRgb(cr, open ? kAccent : Rgb{0.26, 0.21, 0.13});
      cairo_fill(cr);
      setRgb(cr, loaded ? kAccent : st.accent, loaded ? 1.0 : 0.7);
      cairo_set_line_width(cr, 1.2);
      roundRect(cr, w.x + 0.5, w.y + 0.5, w.w - 1, w.h - 1, 4);
      cairo_stroke(cr);
      text(cr, w.label, w.x + w.w * 0.5, w.y + 12, 7.5, 1,
           open ? 0.10 : 0.95, open ? 0.08 : 0.92, open ? 0.05 : 0.84);
      if (loaded) {
        setRgb(cr, {0.42, 0.92, 0.50});
        cairo_arc(cr, w.x + w.w - 7, w.y + 9, 2.2, 0, 2 * kPi);
        cairo_fill(cr);
      }
      break;
    }
    case Kind::CabIrButton: {
      const bool open =
          state.irPickerSlot == static_cast<int>(w.value) && !state.pickerNam;
      const bool loaded = shared.get(P_CAB_IR) > 0.5;
      roundRect(cr, w.x, w.y, w.w, w.h, 4);
      setRgb(cr, open ? kAccent : Rgb{0.26, 0.21, 0.13});
      cairo_fill(cr);
      setRgb(cr, loaded ? kAccent : st.accent, loaded ? 1.0 : 0.7);
      cairo_set_line_width(cr, 1.2);
      roundRect(cr, w.x + 0.5, w.y + 0.5, w.w - 1, w.h - 1, 4);
      cairo_stroke(cr);
      text(cr, w.label, w.x + w.w * 0.5, w.y + 12, 7.5, 1,
           open ? 0.10 : 0.95, open ? 0.08 : 0.92, open ? 0.05 : 0.84);
      if (loaded) {
        setRgb(cr, {0.42, 0.92, 0.50});
        cairo_arc(cr, w.x + w.w - 7, w.y + 9, 2.2, 0, 2 * kPi);
        cairo_fill(cr);
      }
      break;
    }
    case Kind::ModulePresetButton: {
      roundRect(cr, w.x, w.y, w.w, w.h, 3);
      setRgb(cr, st.body);
      cairo_fill(cr);
      setRgb(cr, st.accent, 0.85);
      cairo_set_line_width(cr, 0.9);
      roundRect(cr, w.x + 0.5, w.y + 0.5, w.w - 1, w.h - 1, 3);
      cairo_stroke(cr);
      text(cr, w.label, w.x + w.w * 0.5, w.y + 10, 6.0, 1, st.accent.r,
           st.accent.g, st.accent.b);
      break;
    }
    case Kind::BrowserSearch:
      break;
    case Kind::BrowserOption: {
      const bool cancel = w.value == -1;
      const bool current = !cancel && w.value >= 0 &&
          std::fabs(shared.get(w.param) - w.value) < 0.5;
      const ModuleStyle bst = cancel ? styleFor(0) : styleFor(w.styleType);
      roundRect(cr, w.x, w.y, w.w, w.h, 4);
      const Rgb fill =
          current
              ? Rgb{bst.body.r * 0.62f + bst.accent.r * 0.38f,
                    bst.body.g * 0.62f + bst.accent.g * 0.38f,
                    bst.body.b * 0.62f + bst.accent.b * 0.38f}
              : bst.body;
      setRgb(cr, fill);
      cairo_fill(cr);
      setRgb(cr, current ? bst.accent : bst.edge, current ? 1.0 : 0.9);
      cairo_set_line_width(cr, current ? 1.8 : 1.0);
      roundRect(cr, w.x + 0.5, w.y + 0.5, w.w - 1, w.h - 1, 4); cairo_stroke(cr);
      auto textWidth = [&](const std::string& text, double size) {
        cairo_set_font_face(cr, font_);
        cairo_set_font_size(cr, size);
        cairo_text_extents_t extents;
        cairo_text_extents(cr, text.c_str(), &extents);
        return extents.x_advance;
      };
      const std::string label = w.label ? w.label : "";
      const double avail = w.w - 10.0;
      double nameSize = cancel ? 9.0 : 13.0;
      std::string line1 = label;
      std::string line2;
      if (!cancel && textWidth(label, 13.0) > avail) {
        if (textWidth(label, 11.0) <= avail) {
          nameSize = 11.0;
        } else {
          nameSize = 10.2;
          std::vector<std::string> words;
          size_t start = 0;
          while (start < label.size()) {
            const size_t space = label.find(' ', start);
            words.push_back(label.substr(
                start, space == std::string::npos ? std::string::npos
                                                  : space - start));
            if (space == std::string::npos) break;
            start = space + 1;
          }
          for (double size = 10.2; size >= 8.2; size -= 0.4) {
            line1.clear();
            line2.clear();
            for (const auto& word : words) {
              const std::string candidate =
                  line1.empty() ? word : line1 + " " + word;
              if (line1.empty() || textWidth(candidate, size) <= avail) {
                line1 = candidate;
              } else if (line2.empty()) {
                line2 = word;
              } else {
                const std::string extended = line2 + " " + word;
                if (textWidth(extended, size) <= avail) line2 = extended;
              }
            }
            if (line2.empty() || textWidth(line2, size) <= avail) {
              nameSize = size;
              break;
            }
          }
        }
      }
      const double nameR = current ? 0.97 : bst.accent.r;
      const double nameG = current ? 0.95 : bst.accent.g;
      const double nameB = current ? 0.90 : bst.accent.b;
      if (line2.empty()) {
        text(cr, line1.c_str(), w.x + w.w * 0.5, w.y + w.h * 0.5 + 2,
             nameSize, 1, nameR, nameG, nameB);
      } else {
        text(cr, line1.c_str(), w.x + w.w * 0.5, w.y + 17, nameSize, 1,
             nameR, nameG, nameB);
        text(cr, line2.c_str(), w.x + w.w * 0.5, w.y + 28, nameSize, 1,
             nameR, nameG, nameB);
      }
      if (!cancel) {
        const char* src = w.subLabel;
        if (src && *src) {
          text(cr, src, w.x + w.w * 0.5,
               w.y + (line2.empty() ? w.h - 7 : w.h - 4),
               line2.empty() ? 6.0 : 5.2, 1,
               current ? 0.82 : kDim.r, current ? 0.80 : kDim.g,
               current ? 0.74 : kDim.b);
        }
      }
      break;
    }
    case Kind::ModuleToggle: {
      const bool on = shared.get(w.param) > 0.5;
      roundRect(cr, w.x, w.y, w.w, w.h, 4);
      setRgb(cr, on ? st.accent : st.body);
      cairo_fill(cr);
      setRgb(cr, st.edge, 0.9);
      cairo_set_line_width(cr, 1.0);
      roundRect(cr, w.x + 0.5, w.y + 0.5, w.w - 1, w.h - 1, 4);
      cairo_stroke(cr);
      text(cr, w.label, w.x + w.w * 0.5, w.y + w.h * 0.5 + 4, 8, 1,
           on ? 0.08 : st.accent.r, on ? 0.06 : st.accent.g,
           on ? 0.04 : st.accent.b);
      break;
    }
    case Kind::ModuleButton: {
      const bool lit = std::fabs(shared.get(w.param) - w.value) < 0.5;
      roundRect(cr, w.x, w.y, w.w, w.h, 4);
      setRgb(cr, lit ? st.accent : st.body); cairo_fill(cr);
      text(cr, w.label, w.x + w.w * 0.5, w.y + w.h * 0.5 + 4, 10, 1,
           lit ? 0.08 : st.accent.r, lit ? 0.06 : st.accent.g,
           lit ? 0.04 : st.accent.b);
      break;
    }
    case Kind::ModelButton: {
      const bool lit = std::fabs(shared.get(w.param) - w.value) < 0.5;
      roundRect(cr, w.x, w.y, w.w, w.h, 4);
      setRgb(cr, lit ? st.accent : st.body); cairo_fill(cr);
      text(cr, w.label, w.x + w.w * 0.5, w.y + w.h * 0.5 + 4, 10, 1,
           lit ? 0.08 : st.accent.r, lit ? 0.06 : st.accent.g,
           lit ? 0.04 : st.accent.b);
      break;
    }
    case Kind::Pill: {
      const bool on = shared.get(w.param) > 0.5;
      drawPill(cr, w, on, w.label);
      break;
    }
    case Kind::Button:
      drawButton(cr, w, false);
      break;
    case Kind::Knob:
      drawKnob(cr, w, shared, shared.get(w.param));
      break;
  }
}

void Panel::drawKnob(cairo_t* cr, const Widget& w, const Shared& shared,
                     double value) const {
  const ParamSpec& s = kParamSpecs[w.param];
  const bool stepped01 = w.steps > 1 && w.moduleControl >= 0;
  const double vmax = (w.steps > 1 && !stepped01) ? w.steps - 1 : s.max;
  const double span = vmax - s.min;
  const double f = clamp01(span > 0.0 ? (value - s.min) / span : 0.0);
  const double a = knobAngle(f);
  const int knobType = w.styleSlot >= 0 ? w.styleType : -1;
  const ModuleStyle st = styleFor(knobType);
  setRgb(cr, st.accent, 0.45);
  cairo_set_line_width(cr, 1.0);
  for (int i = 0; i <= 10; ++i) {
    const double aa = knobAngle(i / 10.0);
    cairo_move_to(cr, w.x + std::cos(aa) * w.r * 1.10,
                  w.y + std::sin(aa) * w.r * 1.10);
    cairo_line_to(cr, w.x + std::cos(aa) * w.r * 1.20,
                  w.y + std::sin(aa) * w.r * 1.20);
    cairo_stroke(cr);
  }
  cairo_arc(cr, w.x, w.y, w.r, 0, 2 * kPi);
  setRgb(cr, st.body); cairo_fill(cr);
  cairo_arc(cr, w.x, w.y, w.r * 0.82, 0, 2 * kPi);
  setRgb(cr, st.knob); cairo_fill(cr);
  cairo_set_line_width(cr, 2.0);
  setRgb(cr, st.accent);
  cairo_arc(cr, w.x, w.y, w.r + 2, knobAngle(0), a); cairo_stroke(cr);
  cairo_set_line_width(cr, 2.4);
  setRgb(cr, st.body);
  cairo_move_to(cr, w.x + std::cos(a) * w.r * 0.2, w.y + std::sin(a) * w.r * 0.2);
  cairo_line_to(cr, w.x + std::cos(a) * w.r * 0.65, w.y + std::sin(a) * w.r * 0.65);
  cairo_stroke(cr);
  const double labelY = w.labelBelow ? w.y + w.r * 1.5 + 20
                                     : w.y - w.r * 1.5 - (w.compact ? 9.0 : 16.0);
  text(cr, w.label, w.x, labelY, w.compact ? 8.5 : 10.0, 1,
       kText.r, kText.g, kText.b);
  const std::string val = formatParamValue(shared, w.param, value);
  text(cr, val.c_str(), w.x, w.y + w.r + (w.compact ? 11.0 : 15.0),
       w.compact ? 8.5 : 8.5, 1, st.accent.r, st.accent.g, st.accent.b);
}

void Panel::drawPill(cairo_t* cr, const Widget& w, bool on, const char* label) const {
  roundRect(cr, w.x, w.y, w.w, w.h, 4);
  setRgb(cr, on ? kAccent : Rgb{0.22, 0.19, 0.16}); cairo_fill(cr);
  text(cr, label ? label : w.label, w.x + w.w * 0.5, w.y + w.h * 0.5 + 4, 10, 1,
       on ? 0.10 : 0.78, on ? 0.08 : 0.74, on ? 0.05 : 0.68);
}
void Panel::drawButton(cairo_t* cr, const Widget& w, bool lit) const {
  roundRect(cr, w.x, w.y, w.w, w.h, 4);
  setRgb(cr, lit ? kAccent : Rgb{0.22, 0.19, 0.16}); cairo_fill(cr);
  text(cr, w.label, w.x + w.w * 0.5, w.y + w.h * 0.5 + 4, 10, 1,
       lit ? 0.10 : 0.78, lit ? 0.08 : 0.74, lit ? 0.05 : 0.68);
}
void Panel::drawLevelMeter(cairo_t* cr, double x, double y, double w, double h,
                           double dbL, double dbR) const {
  roundRect(cr, x, y, w, h, 3); setRgb(cr, {0.055, 0.052, 0.048}); cairo_fill(cr);
  const double half = w * 0.5;
  for (int ch = 0; ch < 2; ++ch) {
    const double db = ch == 0 ? dbL : dbR;
    const double f = clamp01((db + 60.0) / 60.0);
    const double bx = x + 2 + ch * (half - 2);
    setRgb(cr, f > 0.92 ? Rgb{0.95, 0.25, 0.18}
                       : (f > 0.78 ? kAccent : Rgb{0.36, 0.78, 0.40}), 0.9);
    cairo_rectangle(cr, bx, y + h - 3 - (h - 6) * f, half - 4, (h - 6) * f);
    cairo_fill(cr);
  }
}

void Panel::drawGrMeter(cairo_t* cr, double x, double y, double w, double h,
                       double grDb) const {
  roundRect(cr, x, y, w, h, 2.5);
  setRgb(cr, {0.055, 0.052, 0.048});
  cairo_fill(cr);
  const int segments = 12;
  const double gap = 1.0;
  const double segW = (w - 2.0 - gap * (segments - 1)) / segments;
  const double db = std::clamp(grDb, 0.0, 12.0);
  for (int i = 0; i < segments; ++i) {
    const double sx = x + 1.0 + i * (segW + gap);
    const bool on = db >= static_cast<double>(i + 1);
    Rgb c = {0.16, 0.14, 0.12};
    if (on) {
      if (i < 3) c = {0.36, 0.80, 0.42};
      else if (i < 6) c = {0.94, 0.62, 0.20};
      else c = {0.95, 0.26, 0.18};
    }
    setRgb(cr, c, on ? 1.0 : 0.55);
    cairo_rectangle(cr, sx, y + 1.5, segW, h - 3.0);
    cairo_fill(cr);
  }
  text(cr, "GR", x, y - 3.0, 7.0, 0, kDim.r, kDim.g, kDim.b);
  const double ticks[5] = {0.0, 3.0, 6.0, 9.0, 12.0};
  for (double t : ticks) {
    const double tx = x + 1.0 + (w - 2.0) * (t / 12.0);
    char label[8];
    std::snprintf(label, sizeof(label), "%.0f", t);
    text(cr, label, tx, y + h + 6.0, 7.0, 1, kDim.r, kDim.g, kDim.b);
  }
}

bool Panel::hitWidget(const Widget& w, double x, double y) const {
  if (w.kind == Kind::Knob) {
    const double dx = x - w.x, dy = y - w.y;
    return dx * dx + dy * dy <= w.r * w.r * 1.5;
  }
  if (w.kind == Kind::Power) {
    const double dx = x - w.x, dy = y - w.y;
    return dx * dx + dy * dy <= w.r * w.r * 1.3;
  }
  if (w.kind == Kind::Footswitch) {
    const double dx = x - w.x, dy = y - w.y;
    const double hit = w.r + 6.0;
    return dx * dx + dy * dy <= hit * hit;
  }
  return x >= w.x && x <= w.x + w.w && y >= w.y && y <= w.y + w.h;
}

int Panel::slotAtX(double x, int page) const {
  constexpr double colW = kPedalWidth;
  constexpr double gap = kPedalGap;
  constexpr double start = kPedalStartX;
  const double center = start + colW * 0.5;
  const int column =
      static_cast<int>(std::lround((x - center) / (colW + gap)));
  return page * 4 + std::clamp(column, 0, 3);
}

void Panel::moveSlot(Shared& shared, int from, int to) {
  if (from == to || from < 0 || to < 0 || from >= kSlotCount ||
      to >= kSlotCount) {
    return;
  }
  double temp[kSlotStride];
  for (uint32_t off = 0; off < kSlotStride; ++off) {
    temp[off] = shared.get(slotParam(from, static_cast<int>(off)));
  }
  if (from < to) {
    for (int slot = from; slot < to; ++slot) {
      for (uint32_t off = 0; off < kSlotStride; ++off) {
        shared.set(slotParam(slot, static_cast<int>(off)),
                   shared.get(slotParam(slot + 1, static_cast<int>(off))));
      }
    }
  } else {
    for (int slot = from; slot > to; --slot) {
      for (uint32_t off = 0; off < kSlotStride; ++off) {
        shared.set(slotParam(slot, static_cast<int>(off)),
                   shared.get(slotParam(slot - 1, static_cast<int>(off))));
      }
    }
  }
  for (uint32_t off = 0; off < kSlotStride; ++off) {
    shared.set(slotParam(to, static_cast<int>(off)), temp[off]);
  }
}

void Panel::drawDragGhost(cairo_t* cr, const Shared& shared) const {
  if (dragSlot_ < 0) return;
  const int type =
      static_cast<int>(shared.get(slotParam(dragSlot_, kSlotOffsetType)) + 0.5);
  const ModuleStyle st = styleFor(type);
  const double x = dragX_ + 12.0;
  const double y = dragY_ - 20.0;
  const double w = 132.0;
  const double h = 34.0;
  roundRect(cr, x + 2, y + 2, w, h, 5);
  setRgb(cr, {0.0, 0.0, 0.0}, 0.35);
  cairo_fill(cr);
  roundRect(cr, x, y, w, h, 5);
  setRgb(cr, st.body, 0.98);
  cairo_fill(cr);
  setRgb(cr, st.accent, 0.95);
  cairo_set_line_width(cr, 1.5);
  roundRect(cr, x + 0.5, y + 0.5, w - 1, h - 1, 5);
  cairo_stroke(cr);
  text(cr, slotModuleName(type), x + w * 0.5, y + h * 0.5 + 4, 12, 1,
       st.accent.r, st.accent.g, st.accent.b);
}

void Panel::beginSlotDrag(PanelState& state, int slot, double x, double y) {
  state.selectedSlot = slot;
  dragSlot_ = slot;
  dropSlot_ = slot;
  slotDragging_ = false;
  dragStartX_ = x;
  dragStartY_ = y;
  dragX_ = x;
  dragY_ = y;
}

bool Panel::pointerDown(Shared& shared, PanelState& state, double x, double y,
                        bool fine) {
  (void)fine;
  if (state.saveModule) {
    state.saveModule = false;
    state.saveModuleSlot = -1;
    state.textBuffer.clear();
  } else if (state.savePreset) {
    state.savePreset = false;
    state.textBuffer.clear();
  } else if (state.renameSetlist) {
    textKey(shared, state, kTextEnter, 0);
  } else if (state.textParam >= 0) {
    textKey(shared, state, kTextEnter, 0);
  }
  if (state.tunerOpen) {
    if ((x >= 720 && x <= 800 && y >= 548 && y <= 574) ||
        x < 200 || x > 840 || y < 120 || y > 600) {
      state.tunerOpen = false;
    }
    return true;
  }
  if (state.midiOpen) {
    const double bx = 180, by = 100, bw = 680, bh = 540;
    if (x < bx || x > bx + bw || y < by || y > by + bh) {
      state.midiOpen = false;
      return true;
    }
    if (x >= bx + 24 && x <= bx + 192 && y >= by + 16 && y <= by + 42) {
      shared.set(P_MIDI_ENABLE,
                 shared.get(P_MIDI_ENABLE) > 0.5 ? 0.0 : 1.0);
      return true;
    }
    if (x >= bx + bw - 110 && x <= bx + bw - 20 && y >= by + bh - 40 &&
        y <= by + bh - 16) {
      state.midiOpen = false;
      return true;
    }
    if (x >= bx + 458 && x <= bx + 558) {
      for (int slot = 0; slot < kSlotCount; ++slot) {
        const double ry = by + 106 + slot * 34;
        if (y >= ry - 12 && y <= ry + 12) {
          const int current =
              shared.midiLearnSlot.load(std::memory_order_relaxed);
          const int next = current == slot ? -1 : slot;
          shared.midiLearnSlot.store(next, std::memory_order_relaxed);
          shared.midiLearnSlotState.store(next, std::memory_order_relaxed);
          shared.midiLearnExpr.store(false, std::memory_order_relaxed);
          shared.midiLearnExprState.store(false, std::memory_order_relaxed);
          state.midiLearnSlot = next;
          state.midiLearnExpr = false;
          return true;
        }
      }
    }
    const double ey = by + 410;
    if (y >= ey - 12 && y <= ey + 12) {
      if (x >= bx + 126 && x <= bx + 200) {
        shared.set(P_MIDI_EXPR_ON,
                   shared.get(P_MIDI_EXPR_ON) > 0.5 ? 0.0 : 1.0);
        return true;
      }
      if (x >= bx + 306 && x <= bx + 398) {
        const int next = (expressionSlot(shared) + 1) % kSlotCount;
        shared.set(P_MIDI_EXPR_SLOT, static_cast<double>(next));
        shared.set(P_MIDI_EXPR_CTL, 0.0);
        return true;
      }
      if (x >= bx + 414 && x <= bx + 546) {
        const int targetSlot = expressionSlot(shared);
        const int type = static_cast<int>(
            shared.get(slotParam(targetSlot, kSlotOffsetType)) + 0.5);
        const int count = std::max(1, moduleDesc(type)->controlCount);
        const int next = (expressionControl(shared) + 1) % count;
        shared.set(P_MIDI_EXPR_CTL, static_cast<double>(next));
        return true;
      }
      if (x >= bx + 562 && x <= bx + 654) {
        const bool current =
            shared.midiLearnExpr.load(std::memory_order_relaxed);
        shared.midiLearnExpr.store(!current, std::memory_order_relaxed);
        shared.midiLearnExprState.store(!current, std::memory_order_relaxed);
        shared.midiLearnSlot.store(-1, std::memory_order_relaxed);
        shared.midiLearnSlotState.store(-1, std::memory_order_relaxed);
        state.midiLearnExpr = !current;
        state.midiLearnSlot = -1;
        return true;
      }
      return true;
    }
    return true;
  }
  if (state.setlistOpen) {
    const double bx = 130, by = 60, bw = 780, bh = 600;
    const double leftX = bx + 20, rightX = bx + 230, listY = by + 92;
    if (x < bx || x > bx + bw || y < by || y > by + bh) {
      state.setlistOpen = false;
      return true;
    }
    const int setlistCount = static_cast<int>(setlists_.size());
    Setlist* current =
        state.setlistIndex >= 0 && state.setlistIndex < setlistCount
            ? &setlists_[static_cast<size_t>(state.setlistIndex)]
            : nullptr;
    auto persist = [&]() {
      if (current) writeSetlist(setlistDir(), *current);
    };
    auto loadEntry = [&](int index) {
      if (!current || index < 0 ||
          index >= static_cast<int>(current->entries.size())) {
        return;
      }
      applySetlistEntry(shared, state,
                        current->entries[static_cast<size_t>(index)]);
      state.setlistEntry = index;
      state.setlistEntryScroll =
          std::max(0, std::min(index - 5,
                               static_cast<int>(current->entries.size()) - 12));
      state.deleteSetlistConfirm = false;
    };

    if (x >= leftX - 4 && x <= leftX + 184) {
      for (int row = 0; row < 11; ++row) {
        const double ry = listY + row * 27;
        if (y >= ry - 13 && y <= ry + 11) {
          const int index = state.setlistScroll + row;
          if (index >= 0 && index < setlistCount) {
            state.setlistIndex = index;
            state.setlistEntry = -1;
            state.setlistEntryScroll = 0;
            state.deleteSetlistConfirm = false;
          }
          return true;
        }
      }
    }
    if (current && x >= rightX - 4 && x <= rightX + 526) {
      const int entryScroll = std::clamp(
          state.setlistEntryScroll, 0,
          std::max(0, static_cast<int>(current->entries.size()) - 12));
      for (int row = 0; row < 12; ++row) {
        const double ry = listY + row * 25;
        if (y >= ry - 12 && y <= ry + 10) {
          const int index = entryScroll + row;
          if (index >= 0 &&
              index < static_cast<int>(current->entries.size())) {
            loadEntry(index);
          }
          return true;
        }
      }
    }

    const double leftFooter = by + bh - 42;
    if (y >= leftFooter && y <= leftFooter + 26) {
      if (x >= leftX && x <= leftX + 54) {
        std::string name = "Setlist 1";
        int n = 2;
        auto exists = [&](const std::string& candidate) {
          for (const auto& s : setlists_) {
            if (s.name == candidate) return true;
          }
          return false;
        };
        while (exists(name)) {
          name = "Setlist " + std::to_string(n++);
        }
        Setlist s;
        s.name = name;
        setlists_.push_back(s);
        writeSetlist(setlistDir(), setlists_.back());
        state.setlistIndex = static_cast<int>(setlists_.size()) - 1;
        state.setlistEntry = -1;
        state.setlistScroll =
            std::max(0, state.setlistIndex - 10);
        state.setlistEntryScroll = 0;
        state.deleteSetlistConfirm = false;
        return true;
      }
      if (x >= leftX + 62 && x <= leftX + 126 && current) {
        state.renameSetlist = true;
        state.textBuffer = current->name;
        state.textFresh = true;
        state.setlistOpen = false;
        state.presetOpen = 0;
        return true;
      }
      if (x >= leftX + 134 && x <= leftX + 188 && current) {
        if (!state.deleteSetlistConfirm) {
          state.deleteSetlistConfirm = true;
          return true;
        }
        deleteSetlist(setlistDir(), current->name);
        setlists_.erase(setlists_.begin() + state.setlistIndex);
        state.deleteSetlistConfirm = false;
        if (state.setlistIndex >= static_cast<int>(setlists_.size())) {
          state.setlistIndex = std::max(0, static_cast<int>(setlists_.size()) - 1);
        }
        state.setlistEntry = -1;
        state.setlistEntryScroll = 0;
        return true;
      }
    }

    const double btnW = 115, btnGap = 8;
    const double row1 = by + bh - 86, row2 = by + bh - 52;
    auto inButton = [&](double bx0, double by0) {
      return x >= bx0 && x <= bx0 + btnW && y >= by0 && y <= by0 + 26;
    };
    if (inButton(rightX, row1)) {
      if (setlists_.empty()) {
        Setlist created;
        created.name = "Setlist 1";
        setlists_.push_back(created);
        state.setlistIndex = 0;
      }
      state.setlistIndex = std::clamp(
          state.setlistIndex, 0, static_cast<int>(setlists_.size()) - 1);
      Setlist& target = setlists_[static_cast<size_t>(state.setlistIndex)];
      SetlistEntry entry;
      entry.name = state.presetIndex >= 0 &&
                           state.presetIndex < presetCount()
                       ? presetName(state.presetIndex)
                       : "Current Board";
      entry.snapshot.reserve(P_COUNT);
      for (uint32_t i = 0; i < P_COUNT; ++i) {
        entry.snapshot.push_back(shared.get(i));
      }
      target.entries.push_back(entry);
      writeSetlist(setlistDir(), target);
      state.setlistEntry = static_cast<int>(target.entries.size()) - 1;
      state.setlistEntryScroll = std::max(0, state.setlistEntry - 11);
      return true;
    }
    if (inButton(rightX + btnW + btnGap, row1) && current &&
        state.setlistEntry >= 0 &&
        state.setlistEntry < static_cast<int>(current->entries.size())) {
      current->entries.erase(current->entries.begin() + state.setlistEntry);
      persist();
      if (state.setlistEntry >= static_cast<int>(current->entries.size())) {
        state.setlistEntry =
            std::max(0, static_cast<int>(current->entries.size()) - 1);
      }
      return true;
    }
    if ((inButton(rightX + 2 * (btnW + btnGap), row1) ||
         inButton(rightX + 3 * (btnW + btnGap), row1)) &&
        current && state.setlistEntry >= 0 &&
        state.setlistEntry < static_cast<int>(current->entries.size())) {
      const int other =
          inButton(rightX + 2 * (btnW + btnGap), row1)
              ? state.setlistEntry - 1
              : state.setlistEntry + 1;
      if (other >= 0 && other < static_cast<int>(current->entries.size())) {
        std::swap(current->entries[static_cast<size_t>(state.setlistEntry)],
                  current->entries[static_cast<size_t>(other)]);
        state.setlistEntry = other;
        persist();
      }
      return true;
    }
    if (inButton(rightX, row2) && current && !current->entries.empty()) {
      const int count = static_cast<int>(current->entries.size());
      const int next = state.setlistEntry <= 0 ? count - 1
                                                : state.setlistEntry - 1;
      loadEntry(next);
      return true;
    }
    if (inButton(rightX + btnW + btnGap, row2) && current &&
        !current->entries.empty()) {
      const int count = static_cast<int>(current->entries.size());
      const int next = state.setlistEntry < 0 ||
                               state.setlistEntry + 1 >= count
                           ? 0
                           : state.setlistEntry + 1;
      loadEntry(next);
      return true;
    }
    if (inButton(rightX + 2 * (btnW + btnGap), row2) && current) {
      loadEntry(state.setlistEntry);
      return true;
    }
    if (inButton(rightX + 3 * (btnW + btnGap), row2)) {
      state.setlistOpen = false;
      return true;
    }
    return true;
  }
  if (x >= kUtilityTunerX && x <= kUtilityTunerX + kUtilityW &&
      y >= kUtilityY && y <= kUtilityY + kUtilityH) {
    state.tunerOpen = !state.tunerOpen;
    state.midiOpen = false;
    state.aboutOpen = false;
    state.presetOpen = 0;
    state.browserSlot = -1;
    return true;
  }
  if (x >= kUtilityMidiX && x <= kUtilityMidiX + kUtilityW &&
      y >= kUtilityY && y <= kUtilityY + kUtilityH) {
    state.midiOpen = !state.midiOpen;
    state.tunerOpen = false;
    state.aboutOpen = false;
    state.presetOpen = 0;
    state.browserSlot = -1;
    return true;
  }
  if (state.irPickerSlot >= 0) {
    const double bx = 260, by = 140, bw = 520, bh = 430;
    const double rowStart = by + 70;
    const int visible = 11;
    if (x >= bx && x <= bx + bw && y >= rowStart &&
        y < rowStart + visible * 26.0) {
      const int index =
          state.irScroll + static_cast<int>((y - rowStart) / 26.0);
      if (index >= 0 && index <= static_cast<int>(state.irNames.size())) {
        shared.set(state.pickerNam ? P_NAM_MODEL : P_CAB_IR, index);
      }
      state.irPickerSlot = -1;
      return true;
    }
    if (x >= bx + 20 && x <= bx + 78 && y >= by + 396 && y <= by + 422) {
      state.irScroll = std::max(0, state.irScroll - 1);
      return true;
    }
    if (x >= bx + 86 && x <= bx + 144 && y >= by + 396 && y <= by + 422) {
      const int maxScroll =
          std::max(0, static_cast<int>(state.irNames.size()) + 1 - visible);
      state.irScroll = std::min(maxScroll, state.irScroll + 1);
      return true;
    }
    if ((x >= bx + bw - 100 && x <= bx + bw - 20 && y >= by + 396 &&
         y <= by + 422) ||
        x < bx || x > bx + bw || y < by || y > by + bh) {
      state.irPickerSlot = -1;
    }
    return true;
  }
  if (state.aboutOpen) {
    if (x >= 120 && x <= 178 && y >= 604 && y <= 630) {
      state.aboutScroll = std::max(0, state.aboutScroll - 1);
      return true;
    }
    if (x >= 184 && x <= 242 && y >= 604 && y <= 630) {
      state.aboutScroll = std::min(aboutMaxScroll(), state.aboutScroll + 1);
      return true;
    }
    if ((x >= 820 && x <= 900 && y >= 654 && y <= 680) ||
        x < 100 || x > 940 || y < 50 || y > 700) {
      state.aboutOpen = false;
    }
    return true;
  }
  if (y >= 20 && y <= 48 && x >= 610 && x <= 650) {
    state.boardPage = 0;
    state.selectedSlot = 0;
    state.browserSlot = -1;
    return true;
  }
  if (y >= 20 && y <= 48 && x >= 658 && x <= 698) {
    state.boardPage = 1;
    state.selectedSlot = 4;
    state.browserSlot = -1;
    return true;
  }
  if (x >= 918 && x <= 1014 && y >= 20 && y <= 48) {
    state.aboutOpen = true;
    state.aboutScroll = 0;
    state.presetOpen = 0;
    state.tunerOpen = false;
    state.midiOpen = false;
    state.irPickerSlot = -1;
    return true;
  }
  if (x >= 706 && x <= 802 && y >= 20 && y <= 48) {
    state.presetOpen = state.presetOpen ? 0 : 1;
    state.browserSlot = -1;
    state.deletePresetConfirm = false;
    if (state.presetOpen) {
      const int rows = presetRowsFor(presetCount());
      const int maxScroll = std::max(0, presetCount() - rows);
      state.presetScroll = state.presetIndex >= 0
                               ? std::clamp(state.presetIndex - rows + 1, 0,
                                            maxScroll)
                               : 0;
    }
    return true;
  }
  if (state.presetOpen) {
    if (x >= kPresetListX + kPresetListW - 190 &&
        x <= kPresetListX + kPresetListW - 78 && y >= kPresetListY + 8 &&
        y <= kPresetListY + 28) {
      const int banks = static_cast<int>(presetBankNames().size());
      state.presetBank = (state.presetBank + 1) % std::max(1, banks);
      state.presetScroll = 0;
      return true;
    }
    if (x >= kPresetListX + kPresetListW - 72 &&
        x <= kPresetListX + kPresetListW - 14 && y >= kPresetListY + 8 &&
        y <= kPresetListY + 28) {
      reloadSetlists();
      state.setlistOpen = true;
      state.setlistScroll = 0;
      state.setlistEntryScroll = 0;
      state.deleteSetlistConfirm = false;
      state.presetOpen = 0;
      return true;
    }
    const std::vector<int> order = presetOrderForBank(state.presetBank);
    const int count = static_cast<int>(order.size());
    const int rows = presetRowsFor(count);
    const int maxScroll = std::max(0, count - rows);
    const int scroll = std::clamp(state.presetScroll, 0, maxScroll);
    const double footerY =
        kPresetListY + kPresetHeaderH + rows * kPresetRowH +
        kPresetBottomGap + 4;
    if (x >= kPresetListX + 14 && x <= kPresetListX + 68 &&
        y >= footerY && y <= footerY + 24) {
      state.presetScroll = std::max(0, scroll - 1);
      return true;
    }
    if (x >= kPresetListX + 74 && x <= kPresetListX + 128 &&
        y >= footerY && y <= footerY + 24) {
      state.presetScroll = std::min(maxScroll, scroll + 1);
      return true;
    }
    const bool userCurrent = state.presetIndex >= kPresetCount &&
                             state.presetIndex < count;
    if (userCurrent && x >= kPresetListX + 140 && x <= kPresetListX + 208 &&
        y >= footerY && y <= footerY + 24) {
      state.renamePreset = true;
      state.textBuffer = presetName(state.presetIndex);
      state.textFresh = true;
      state.deletePresetConfirm = false;
      state.presetOpen = 0;
      return true;
    }
    if (userCurrent && x >= kPresetListX + 214 && x <= kPresetListX + 286 &&
        y >= footerY && y <= footerY + 24) {
      if (!state.deletePresetConfirm) {
        state.deletePresetConfirm = true;
        return true;
      }
      const std::string name = presetName(state.presetIndex);
      if (deleteUserPreset(userPresetDir(), name)) {
        userPresets_ = loadUserPresets(userPresetDir());
        state.presetIndex = -1;
        state.presetDirty = true;
      }
      state.deletePresetConfirm = false;
      state.presetOpen = 0;
      return true;
    }
    state.deletePresetConfirm = false;
    if (x >= kPresetListX + kPresetListW - 84 &&
        x <= kPresetListX + kPresetListW - 14 && y >= footerY &&
        y <= footerY + 24) {
      state.presetOpen = 0;
      return true;
    }
    if (x >= kPresetListX && x <= kPresetListX + kPresetListW &&
        y >= kPresetListY + kPresetHeaderH &&
        y < kPresetListY + kPresetHeaderH + rows * kPresetRowH) {
      const int row = static_cast<int>((y - kPresetListY - kPresetHeaderH) /
                                       kPresetRowH);
      const int index = scroll + row;
      if (index >= 0 && index < count) {
        const int actual = order[static_cast<size_t>(index)];
        applyPreset(shared, state, actual);
      }
      state.presetOpen = 0;
      return true;
    }
    if (y >= kPresetListY) {
      state.presetOpen = 0;
      state.deletePresetConfirm = false;
      return true;
    }
  }
  if (x >= 812 && x <= 908 && y >= 20 && y <= 48) {
    state.savePreset = true;
    state.textFresh = true;
    state.textParam = -1;
    if (state.presetIndex >= kPresetCount) {
      state.textBuffer = presetName(state.presetIndex);
    } else {
      state.textBuffer = "My Rack";
    }
    return true;
  }
  if (handlePresetClick(shared, state, x, y)) return true;
  if (state.browserSlot >= 0) {
    if (state.modulePresetMenu >= 0) {
      const double mx = 340.0, my = 252.0;
      const double by = my + 84.0;
      if (x >= mx + 16 && x <= mx + 96 && y >= by && y <= by + 26) {
        const int idx = state.modulePresetMenu;
        if (idx >= 0 && idx < static_cast<int>(modulePresets_.size())) {
          state.renameModulePreset = true;
          state.renameModuleIndex = idx;
          state.textBuffer = modulePresets_[static_cast<size_t>(idx)].name;
          state.textFresh = true;
          state.modulePresetMenu = -1;
          state.deleteModuleConfirm = false;
          state.browserSlot = -1;
          state.browserQuery.clear();
          state.browserSearch = false;
          state.browserPresetFilter = false;
        }
        return true;
      }
      if (x >= mx + 104 && x <= mx + 196 && y >= by && y <= by + 26) {
        if (!state.deleteModuleConfirm) {
          state.deleteModuleConfirm = true;
          return true;
        }
        const int idx = state.modulePresetMenu;
        if (idx >= 0 && idx < static_cast<int>(modulePresets_.size())) {
          const std::string name =
              modulePresets_[static_cast<size_t>(idx)].name;
          if (deleteModulePreset(modulePresetDir(), name)) {
            modulePresets_ = loadModulePresets(modulePresetDir());
          }
        }
        state.modulePresetMenu = -1;
        state.deleteModuleConfirm = false;
        return true;
      }
      if (x >= mx + 204 && x <= mx + 284 && y >= by && y <= by + 26) {
        state.modulePresetMenu = -1;
        state.deleteModuleConfirm = false;
        return true;
      }
      state.modulePresetMenu = -1;
      state.deleteModuleConfirm = false;
      return true;
    }
    for (const Widget& w : widgets_) {
      if (w.kind == Kind::BrowserTab && hitWidget(w, x, y)) {
        state.browserCategory = static_cast<int>(w.value);
        state.browserScroll = 0;
        state.browserPresetFilter = false;
        state.browserQuery.clear();
        state.browserSearch = false;
        return true;
      }
      if (w.kind == Kind::BrowserSearch && hitWidget(w, x, y)) {
        state.browserSearch = true;
        state.browserFresh = true;
        return true;
      }
      if (w.kind == Kind::BrowserScroll && hitWidget(w, x, y)) {
        const int maxScroll = std::max(
            0, browserOptionCount(state.browserCategory, state.browserQuery,
                                  presetFilterTypeFor(shared, state)) -
                   browserVisibleCount(state.browserQuery));
        state.browserScroll = std::clamp(
            state.browserScroll + static_cast<int>(w.value), 0, maxScroll);
        return true;
      }
      if (w.kind != Kind::BrowserOption || !hitWidget(w, x, y)) continue;
      if (w.value <= -2000) {
        const int idx = -2000 - w.value;
        if (idx >= 0 && idx < static_cast<int>(modulePresets_.size())) {
          const ModulePreset& p = modulePresets_[static_cast<size_t>(idx)];
          const int slot = state.browserSlot;
          shared.set(slotParam(slot, kSlotOffsetType),
                     static_cast<double>(p.type));
          for (int k = 0; k < 12; ++k) {
            shared.set(slotParam(slot, kSlotOffsetC0 + k),
                       static_cast<double>(p.controls[static_cast<size_t>(k)]));
          }
        }
      } else if (w.value <= -1000) {
        const int idx = -1000 - w.value;
        if (idx >= 0 && idx < kFactoryModulePresetCount) {
          const FactoryModulePreset& p = kFactoryModulePresets[idx];
          const int slot = state.browserSlot;
          shared.set(slotParam(slot, kSlotOffsetType),
                     static_cast<double>(p.type));
          for (int k = 0; k < 12; ++k) {
            shared.set(slotParam(slot, kSlotOffsetC0 + k),
                       static_cast<double>(p.controls[static_cast<size_t>(k)]));
          }
        }
      } else if (w.value >= 0) {
        applyModuleDefaults(shared, state.browserSlot,
                            static_cast<int>(w.value));
      }
      state.browserSlot = -1;
      state.browserSearch = false;
      state.browserQuery.clear();
      state.browserPresetFilter = false;
      state.presetDirty = true;
      return true;
    }
    state.browserSlot = -1;
    state.browserSearch = false;
    state.browserQuery.clear();
    state.browserPresetFilter = false;
    return true;
  }
  for (const Widget& w : widgets_) {
    if (!hitWidget(w, x, y)) continue;
    switch (w.kind) {
      case Kind::Power:
        shared.set(P_POWER, shared.get(P_POWER) > 0.5 ? 0.0 : 1.0);
        state.presetDirty = true;
        return true;
      case Kind::SlotCard:
        state.selectedSlot = static_cast<int>(w.param);
        return true;
      case Kind::NamModelButton: {
        state.irPickerSlot = static_cast<int>(w.value);
        state.irScroll = 0;
        state.pickerNam = true;
        state.irNames = listNamFiles(namDir());
        state.aboutOpen = false;
        state.tunerOpen = false;
        state.presetOpen = 0;
        state.browserSlot = -1;
        return true;
      }
    case Kind::CabIrButton: {
        state.irPickerSlot = static_cast<int>(w.value);
        state.irScroll = 0;
        state.pickerNam = false;
        state.irNames = listIrFiles(irDir());
        state.aboutOpen = false;
        state.tunerOpen = false;
        state.presetOpen = 0;
        state.browserSlot = -1;
        return true;
      }
      case Kind::ModulePresetButton:
        state.browserSlot = static_cast<int>(w.value);
        state.browserCategory = 7;
        state.browserScroll = 0;
        state.browserQuery.clear();
        state.browserSearch = false;
        state.browserPresetFilter = true;
        state.presetOpen = 0;
        return true;
      case Kind::ModuleName:
      case Kind::ModuleDragHandle:
        beginSlotDrag(state, static_cast<int>(w.value), x, y);
        return true;
      case Kind::BrowserOption:
      case Kind::BrowserTab:
      case Kind::BrowserScroll:
      case Kind::BrowserSearch:
        break;
      case Kind::Footswitch:
        shared.set(w.param, shared.get(w.param) > 0.5 ? 0.0 : 1.0);
        state.presetDirty = true;
        return true;
      case Kind::ModuleToggle:
        shared.set(w.param, shared.get(w.param) > 0.5 ? 0.0 : 1.0);
        state.presetDirty = true;
        return true;
      case Kind::ModuleButton:
        shared.set(w.param, w.value);
        state.presetDirty = true;
        return true;
      case Kind::ModelButton:
        shared.set(w.param, w.value);
        state.presetDirty = true;
        return true;
      case Kind::Pill:
        shared.set(w.param, shared.get(w.param) > 0.5 ? 0.0 : 1.0);
        state.presetDirty = true;
        return true;
      case Kind::Button:
        shared.set(w.param, w.value);
        state.presetDirty = true;
        return true;
      case Kind::Knob: {
        const auto now = std::chrono::steady_clock::now();
        if (lastClickParam_ == static_cast<int>(w.param) &&
            now - lastClickTime_ <= std::chrono::milliseconds(350)) {
          double reset = kParamSpecs[w.param].def;
          if (w.param >= P_SLOT0_TYPE) {
            const int slot =
                static_cast<int>((w.param - P_SLOT0_TYPE) / kSlotStride);
            const int offset =
                static_cast<int>((w.param - P_SLOT0_TYPE) % kSlotStride);
            if (slot >= 0 && slot < kSlotCount && offset >= kSlotOffsetC0) {
              const int control = offset - kSlotOffsetC0;
              const int type = static_cast<int>(
                  shared.get(slotParam(slot, kSlotOffsetType)) + 0.5);
              reset = defaultControlValue(moduleDesc(type)->id, control);
            }
          }
          shared.set(w.param, reset);
          state.presetDirty = true;
          lastClickParam_ = -1;
          dragParam_ = -1;
          return true;
        }
        lastClickParam_ = static_cast<int>(w.param);
        lastClickTime_ = now;
        dragParam_ = static_cast<int>(w.param);
        dragSteps_ = w.steps;
        dragStartY_ = y;
        dragStartValue_ = shared.get(w.param);
        return false;
      }
    }
  }
  return false;
}

bool Panel::pointerMove(Shared& shared, PanelState& state, double x, double y,
                        bool fine) {
  if (dragSlot_ >= 0) {
    if (!slotDragging_) {
      const double dx = x - dragStartX_;
      const double dy = y - dragStartY_;
      if (dx * dx + dy * dy > 36.0) {
        slotDragging_ = true;
      }
    }
    if (slotDragging_) {
      dragX_ = x;
      dragY_ = y;
      if (y >= 20.0 && y <= 48.0) {
        if (x >= 610.0 && x <= 650.0 && state.boardPage != 0) {
          state.boardPage = 0;
          return true;
        }
        if (x >= 658.0 && x <= 698.0 && state.boardPage != 1) {
          state.boardPage = 1;
          return true;
        }
      }
      const int target = slotAtX(x, state.boardPage);
      if (target != dragSlot_) {
        moveSlot(shared, dragSlot_, target);
        dragSlot_ = target;
        dropSlot_ = target;
        state.selectedSlot = target;
        state.boardPage = target / 4;
        state.presetDirty = true;
      } else {
        dropSlot_ = target;
      }
    }
    return true;
  }
  if (dragParam_ < 0) return false;
  const uint32_t param = static_cast<uint32_t>(dragParam_);
  const ParamSpec& spec = kParamSpecs[param];
  const double perPixel = spec.stepped ? 1.0 / 18.0
                                       : (spec.max - spec.min) / (fine ? 800.0 : 200.0);
  double v = dragStartValue_ + (dragStartY_ - y) * perPixel;
  if (dragSteps_ > 1) {
    v = std::round(v * (dragSteps_ - 1)) / (dragSteps_ - 1);
  }
  shared.set(param, v);
  state.presetDirty = true;
  return true;
}

void Panel::pointerUp(Shared& shared, PanelState& state) {
  (void)shared;
  if (dragSlot_ >= 0) {
    if (!slotDragging_) {
      state.selectedSlot = dragSlot_;
      state.browserSlot = dragSlot_;
      state.browserScroll = 0;
      state.browserQuery.clear();
      state.browserSearch = false;
      state.browserPresetFilter = false;
      state.presetOpen = 0;
    }
    dragSlot_ = -1;
    dropSlot_ = -1;
    slotDragging_ = false;
  }
  dragParam_ = -1;
}

bool Panel::wheel(Shared& shared, PanelState& state, double x, double y,
                    double dy) {
  if (state.setlistOpen) {
    const double bx = 130, by = 60, bw = 780, bh = 600;
    const double listY = by + 92;
    const int setlistCount = static_cast<int>(setlists_.size());
    if (x < bx || x > bx + bw || y < by || y > by + bh) {
      state.setlistOpen = false;
      return true;
    }
    if (x >= bx + 16 && x <= bx + 212 && y >= listY - 14 &&
        y <= listY + 11 * 27) {
      const int maxScroll = std::max(0, setlistCount - 11);
      state.setlistScroll = std::clamp(
          state.setlistScroll - (dy > 0 ? 1 : -1), 0, maxScroll);
      return true;
    }
    if (state.setlistIndex >= 0 &&
        state.setlistIndex < setlistCount &&
        x >= bx + 226 && x <= bx + 760 && y >= listY - 14 &&
        y <= listY + 12 * 25) {
      const int count = static_cast<int>(
          setlists_[static_cast<size_t>(state.setlistIndex)].entries.size());
      const int maxScroll = std::max(0, count - 12);
      state.setlistEntryScroll = std::clamp(
          state.setlistEntryScroll - (dy > 0 ? 1 : -1), 0, maxScroll);
      return true;
    }
    return true;
  }

  if (state.irPickerSlot >= 0) {
    const int maxScroll =
        std::max(0, static_cast<int>(state.irNames.size()) + 1 - 11);
    state.irScroll = std::clamp(
        state.irScroll - (dy > 0 ? 1 : -1), 0, maxScroll);
    return true;
  }
  if (state.presetOpen) {
    const int maxScroll =
        std::max(0, presetCount() - presetRowsFor(presetCount()));
    state.presetScroll = std::clamp(
        state.presetScroll - (dy > 0 ? 1 : -1), 0, maxScroll);
    return true;
  }
  if (state.aboutOpen) {
    const int maxScroll = aboutMaxScroll();
    state.aboutScroll = std::clamp(
        state.aboutScroll - (dy > 0 ? 1 : -1), 0, maxScroll);
    return true;
  }
  if (state.browserSlot >= 0) {
    const int maxScroll = std::max(
        0, browserOptionCount(state.browserCategory, state.browserQuery,
                              presetFilterTypeFor(shared, state)) -
               browserVisibleCount(state.browserQuery));
    state.browserScroll = std::clamp(
        state.browserScroll - (dy > 0 ? 1 : -1), 0, maxScroll);
    return true;
  }
  for (const Widget& w : widgets_) {
    if (w.kind != Kind::Knob || !hitWidget(w, x, y)) continue;
    const ParamSpec& spec = kParamSpecs[w.param];
    if (w.steps > 1) {
      const double step = 1.0 / (w.steps - 1);
      const double cur = shared.get(w.param);
      double v = cur + (dy > 0 ? step : -step);
      v = std::round(v / step) * step;
      shared.set(w.param, v);
    } else if (spec.stepped) {
      shared.set(w.param, shared.get(w.param) - (dy > 0 ? 1.0 : -1.0));
    } else {
      shared.set(w.param, shared.get(w.param) - dy * (spec.max - spec.min) * 0.01);
    }
    state.presetDirty = true;
    return true;
  }
  return false;
}

bool Panel::handlePresetClick(Shared& shared, PanelState& state, double x,
                              double y) {
  if (y < 14 || y > 50) return false;
  if (x >= 72 && x <= 108) {
    const int next = state.presetIndex <= 0 ? 0 : state.presetIndex - 1;
    applyPreset(shared, state, next);
    state.presetOpen = 0;
    return true;
  }
  if (x >= 366 && x <= 402) {
    const int next = std::min(presetCount() - 1, state.presetIndex + 1);
    applyPreset(shared, state, next);
    state.presetOpen = 0;
    return true;
  }
  if (x >= 110 && x <= 365) {
    state.presetOpen = state.presetOpen ? 0 : 1;
    state.browserSlot = -1;
    state.deletePresetConfirm = false;
    if (state.presetOpen) {
      const int rows = presetRowsFor(presetCount());
      const int maxScroll = std::max(0, presetCount() - rows);
      state.presetScroll = state.presetIndex >= 0
                               ? std::clamp(state.presetIndex - rows + 1, 0,
                                            maxScroll)
                               : 0;
    }
    return true;
  }
  return x >= 72 && x <= 402;
}

int Panel::presetCount() const {
  return kPresetCount + static_cast<int>(userPresets_.size());
}

const char* Panel::presetName(int index) const {
  if (index >= 0 && index < kPresetCount) return kPresets[index].name;
  const int user = index - kPresetCount;
  if (user >= 0 && user < static_cast<int>(userPresets_.size())) {
    return userPresets_[static_cast<size_t>(user)].name.c_str();
  }
  return "Init";
}

void Panel::applyPreset(Shared& shared, PanelState& state, int index) const {
  if (index < 0 || index >= presetCount()) return;
  if (index < kPresetCount) {
    // Factory boards are absolute: start from empty slots and neutral globals
    // so every preset sounds the same regardless of the previous board.
    for (int slot = 0; slot < kSlotCount; ++slot) {
      applyModuleDefaults(shared, slot, static_cast<int>(ModuleId::Off));
    }
    shared.set(P_INPUT, 0.0);
    shared.set(P_OUTPUT, 0.0);
    const Preset& p = kPresets[index];
    for (int i = 0; i < p.count; ++i) {
      const PresetValue& v = p.values[i];
      if (v.param >= 0 && v.param < static_cast<int>(P_COUNT))
        shared.set(static_cast<uint32_t>(v.param), v.value);
    }
  } else {
    const UserPreset& p =
        userPresets_[static_cast<size_t>(index - kPresetCount)];
    const int n = std::min(static_cast<int>(p.values.size()),
                           static_cast<int>(P_COUNT));
    for (int i = 0; i < n; ++i) {
      shared.set(static_cast<uint32_t>(i), p.values[static_cast<size_t>(i)]);
    }
  }
  state.presetIndex = index;
  state.presetDirty = false;
}

void Panel::beginTextEntry(Shared& shared, PanelState& state, double x,
                           double y) {
  if (state.browserSlot >= 0) {
    for (const Widget& w : widgets_) {
      if (w.kind != Kind::BrowserOption || !hitWidget(w, x, y)) continue;
      if (w.value <= -2000) {
        const int idx = -2000 - w.value;
        if (idx >= 0 && idx < static_cast<int>(modulePresets_.size())) {
          state.modulePresetMenu = idx;
          state.deleteModuleConfirm = false;
        }
      }
      return;
    }
  }
  for (const Widget& w : widgets_) {
    if (w.kind != Kind::Knob || !hitWidget(w, x, y)) continue;
    state.textParam = static_cast<int>(w.param);
    state.textBuffer = formatParamValue(shared, w.param, shared.get(w.param));
    state.textFresh = true;
    return;
  }
  for (const Widget& w : widgets_) {
    if (w.kind != Kind::ModuleName || !hitWidget(w, x, y)) continue;
    const int slot = static_cast<int>(w.value);
    const int type =
        static_cast<int>(shared.get(slotParam(slot, kSlotOffsetType)) + 0.5);
    if (type <= 0) break;
    state.saveModule = true;
    state.saveModuleSlot = slot;
    state.textParam = -1;
    state.textBuffer = std::string(slotModuleName(type)) + " Setting";
    state.textFresh = true;
    return;
  }
  state.textParam = -1;
  state.textBuffer.clear();
}

bool Panel::textKey(Shared& shared, PanelState& state, int key, char ch) {
  if (state.renameSetlist) {
    if (key == kTextEscape) {
      state.renameSetlist = false;
      state.textBuffer.clear();
      return true;
    }
    if (key == kTextEnter) {
      if (state.setlistIndex >= 0 &&
          state.setlistIndex < static_cast<int>(setlists_.size())) {
        const std::string oldName =
            setlists_[static_cast<size_t>(state.setlistIndex)].name;
        const std::string newName = sanitizePresetName(state.textBuffer);
        if (!newName.empty() &&
            renameSetlist(setlistDir(), oldName, newName,
                          setlists_[static_cast<size_t>(state.setlistIndex)])) {
          reloadSetlists();
          for (int i = 0; i < static_cast<int>(setlists_.size()); ++i) {
            if (setlists_[static_cast<size_t>(i)].name == newName) {
              state.setlistIndex = i;
              break;
            }
          }
          state.setlistOpen = true;
          state.renameSetlist = false;
          state.textBuffer.clear();
        }
      }
      return true;
    }
    if (key == kTextBackspace) {
      if (state.textFresh) {
        state.textBuffer.clear();
        state.textFresh = false;
      } else if (!state.textBuffer.empty()) {
        state.textBuffer.pop_back();
      }
      return true;
    }
    if (ch >= 32 && ch < 127 && state.textBuffer.size() < 60) {
      if (state.textFresh) {
        state.textBuffer.clear();
        state.textFresh = false;
      }
      state.textBuffer.push_back(ch);
    }
    return true;
  }
  if (state.renameModulePreset) {
    if (key == kTextEscape) {
      state.renameModulePreset = false;
      state.renameModuleIndex = -1;
      state.textBuffer.clear();
      return true;
    }
    if (key == kTextEnter) {
      const int index = state.renameModuleIndex;
      if (index >= 0 && index < static_cast<int>(modulePresets_.size())) {
        const ModulePreset old = modulePresets_[static_cast<size_t>(index)];
        const std::string newName = sanitizePresetName(state.textBuffer);
        if (!newName.empty() &&
            renameModulePreset(modulePresetDir(), old.name, newName, old.type,
                               old.controls)) {
          modulePresets_ = loadModulePresets(modulePresetDir());
          state.renameModulePreset = false;
          state.renameModuleIndex = -1;
          state.textBuffer.clear();
        }
      }
      return true;
    }
    if (key == kTextBackspace) {
      if (state.textFresh) {
        state.textBuffer.clear();
        state.textFresh = false;
      } else if (!state.textBuffer.empty()) {
        state.textBuffer.pop_back();
      }
      return true;
    }
    if (ch >= 32 && ch < 127 && state.textBuffer.size() < 60) {
      if (state.textFresh) {
        state.textBuffer.clear();
        state.textFresh = false;
      }
      state.textBuffer.push_back(ch);
    }
    return true;
  }
  if (state.renamePreset) {
    if (key == kTextEscape) {
      state.renamePreset = false;
      state.textBuffer.clear();
      return true;
    }
    if (key == kTextEnter) {
      const int index = state.presetIndex;
      if (index >= kPresetCount &&
          index < kPresetCount + static_cast<int>(userPresets_.size())) {
        const std::string oldName = presetName(index);
        const auto& values =
            userPresets_[static_cast<size_t>(index - kPresetCount)].values;
        const std::string newName = sanitizePresetName(state.textBuffer);
        if (!newName.empty() &&
            renameUserPreset(userPresetDir(), oldName, newName, values)) {
          userPresets_ = loadUserPresets(userPresetDir());
          for (int i = 0; i < static_cast<int>(userPresets_.size()); ++i) {
            if (userPresets_[static_cast<size_t>(i)].name == newName) {
              state.presetIndex = kPresetCount + i;
              break;
            }
          }
          state.renamePreset = false;
          state.textBuffer.clear();
        }
      }
      return true;
    }
    if (key == kTextBackspace) {
      if (state.textFresh) {
        state.textBuffer.clear();
        state.textFresh = false;
      } else if (!state.textBuffer.empty()) {
        state.textBuffer.pop_back();
      }
      return true;
    }
    if (ch >= 32 && ch < 127 && state.textBuffer.size() < 60) {
      if (state.textFresh) {
        state.textBuffer.clear();
        state.textFresh = false;
      }
      state.textBuffer.push_back(ch);
    }
    return true;
  }
  if (state.presetOpen && key == kTextEscape) {
    state.presetOpen = 0;
    return true;
  }
  if (state.saveModule) {
    if (key == kTextEscape) {
      state.saveModule = false;
      state.saveModuleSlot = -1;
      state.textBuffer.clear();
      return true;
    }
    if (key == kTextEnter) {
      const int slot = state.saveModuleSlot;
      if (slot >= 0 && slot < kSlotCount) {
        const int type =
            static_cast<int>(shared.get(slotParam(slot, kSlotOffsetType)) + 0.5);
        if (type > 0) {
          std::array<float, 12> controls{};
          for (int k = 0; k < 12; ++k) {
            controls[static_cast<size_t>(k)] = static_cast<float>(
                shared.get(slotParam(slot, kSlotOffsetC0 + k)));
          }
          if (saveModulePreset(modulePresetDir(), state.textBuffer, type,
                               controls)) {
            modulePresets_ = loadModulePresets(modulePresetDir());
          }
        }
      }
      state.saveModule = false;
      state.saveModuleSlot = -1;
      state.textBuffer.clear();
      return true;
    }
    if (key == kTextBackspace) {
      if (state.textFresh) {
        state.textBuffer.clear();
        state.textFresh = false;
      } else if (!state.textBuffer.empty()) {
        state.textBuffer.pop_back();
      }
      return true;
    }
    if (ch >= 32 && ch < 127 && state.textBuffer.size() < 60) {
      if (state.textFresh) {
        state.textBuffer.clear();
        state.textFresh = false;
      }
      state.textBuffer.push_back(ch);
    }
    return true;
  }
  if (state.savePreset) {
    if (key == kTextEscape) {
      state.savePreset = false;
      state.textBuffer.clear();
      return true;
    }
    if (key == kTextEnter) {
      const std::string name = sanitizePresetName(state.textBuffer);
      if (!name.empty() &&
          saveUserPreset(userPresetDir(), name, shared)) {
        userPresets_ = loadUserPresets(userPresetDir());
        for (int i = 0; i < static_cast<int>(userPresets_.size()); ++i) {
          if (userPresets_[static_cast<size_t>(i)].name == name) {
            state.presetIndex = kPresetCount + i;
            state.presetDirty = false;
            break;
          }
        }
      }
      state.savePreset = false;
      state.textBuffer.clear();
      return true;
    }
    if (key == kTextBackspace) {
      if (state.textFresh) {
        state.textBuffer.clear();
        state.textFresh = false;
      } else if (!state.textBuffer.empty()) {
        state.textBuffer.pop_back();
      }
      return true;
    }
    if (ch >= 32 && ch < 127 && state.textBuffer.size() < 60) {
      if (state.textFresh) {
        state.textBuffer.clear();
        state.textFresh = false;
      }
      state.textBuffer.push_back(ch);
    }
    return true;
  }
  if (state.browserSearch) {
    if (key == kTextEscape) {
      state.browserSearch = false;
      state.browserQuery.clear();
      state.browserScroll = 0;
      return true;
    }
    if (key == kTextEnter) {
      state.browserSearch = false;
      return true;
    }
    if (key == kTextBackspace) {
      if (state.browserFresh) {
        state.browserQuery.clear();
        state.browserFresh = false;
      } else if (!state.browserQuery.empty()) {
        state.browserQuery.pop_back();
      }
      state.browserScroll = 0;
      return true;
    }
    if (ch >= 32 && ch < 127 && state.browserQuery.size() < 32) {
      if (state.browserFresh) {
        state.browserQuery.clear();
        state.browserFresh = false;
      }
      state.browserQuery.push_back(ch);
      state.browserScroll = 0;
    }
    return true;
  }
  if (state.textParam < 0) return false;
  if (key == kTextEscape) {
    state.textParam = -1;
    state.textBuffer.clear();
    return true;
  }
  if (key == kTextEnter) {
    double value = 0.0;
    if (parseParamValue(shared, static_cast<uint32_t>(state.textParam),
                        state.textBuffer, &value)) {
      shared.set(static_cast<uint32_t>(state.textParam), value);
      state.presetDirty = true;
    }
    state.textParam = -1;
    state.textBuffer.clear();
    return true;
  }
  if (key == kTextBackspace) {
    if (state.textFresh) {
      state.textBuffer.clear();
      state.textFresh = false;
    } else if (!state.textBuffer.empty()) {
      state.textBuffer.pop_back();
    }
    return true;
  }
  if (ch >= 32 && ch < 127 && state.textBuffer.size() < 32) {
    if (state.textFresh) {
      state.textBuffer.clear();
      state.textFresh = false;
    }
    state.textBuffer.push_back(ch);
  }
  return true;
}

void Panel::drawTextEntry(cairo_t* cr, const PanelState& state) const {
  if (state.textParam < 0 && !state.savePreset && !state.saveModule &&
      !state.renamePreset && !state.renameModulePreset &&
      !state.renameSetlist) {
    return;
  }
  const Widget* target = nullptr;
  if (state.textParam >= 0) {
    for (const Widget& w : widgets_) {
      if (w.kind == Kind::Knob &&
          static_cast<int>(w.param) == state.textParam) {
        target = &w;
        break;
      }
    }
  }
  const double w = 480.0, h = 44.0;
  const double x = (kPanelWidth - w) * 0.5;
  const double y = 54.0;

  setRgb(cr, {0.02, 0.02, 0.03}, 0.60);
  roundRect(cr, x + 3, y + 4, w, h, 7);
  cairo_fill(cr);
  setRgb(cr, {0.10, 0.10, 0.11});
  roundRect(cr, x, y, w, h, 7);
  cairo_fill(cr);
  setRgb(cr, {0.42, 0.86, 0.52});
  cairo_set_line_width(cr, 1.4);
  roundRect(cr, x + 0.7, y + 0.7, w - 1.4, h - 1.4, 7);
  cairo_stroke(cr);

  const char* name =
      state.renamePreset
          ? "RENAME PRESET"
          : (state.renameModulePreset
                 ? "RENAME PEDAL PRESET"
                 : (state.savePreset
                        ? "SAVE PRESET"
                        : (state.saveModule
                               ? "SAVE PEDAL PRESET"
                               : (target ? target->label : "Value"))));
  text(cr, name, x + 14, y + 16, 8.5, 0, 0.72, 0.78, 0.74);
  text(cr, (state.savePreset || state.saveModule) ? "TYPE NAME" : "TYPE VALUE",
       x + 14, y + 33, 7.0, 0, 0.50, 0.56, 0.52);
  const double valueX = x + 150.0;
  const double valueY = y + 34.0;
  cairo_save(cr);
  cairo_rectangle(cr, x + 138, y + 6, w - 210, h - 12);
  cairo_clip(cr);
  text(cr, state.textBuffer.c_str(), valueX, valueY, 16, 0, 0.94, 0.96,
       0.94);
  if (cursorVisible()) {
    cairo_set_font_face(cr, font_);
    cairo_set_font_size(cr, 16);
    cairo_text_extents_t extents;
    cairo_text_extents(cr, state.textBuffer.c_str(), &extents);
    text(cr, "_", valueX + extents.x_advance + 1.0, valueY, 16, 0, 0.94,
         0.96, 0.94);
  }
  cairo_restore(cr);
  text(cr, "ENTER", x + w - 14, y + 16, 7.0, 2, 0.50, 0.56, 0.52);
}

std::vector<Panel::WidgetRect> Panel::widgetRects() const {
  std::vector<WidgetRect> out;
  for (const Widget& w : widgets_) {
    WidgetRect r;
    r.param = w.param;
    r.label = w.label;
    r.knob = w.kind == Kind::Knob;
    r.toggle = w.kind == Kind::Pill || w.kind == Kind::Footswitch;
    r.r = w.r;
    if (w.kind == Kind::Knob) {
      r.x = w.x - w.r; r.y = w.y - w.r; r.w = w.r * 2; r.h = w.r * 2;
    } else {
      r.x = w.x; r.y = w.y; r.w = w.w; r.h = w.h;
    }
    out.push_back(r);
  }
  return out;
}

} // namespace pedal
