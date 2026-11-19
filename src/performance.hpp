// Pedal Suite — plugin-side performance data
// SPDX-License-Identifier: GPL-3.0-or-later
//
// The editor owns the setlist manager, but the CLAP plugin needs a snapshot
// of the setlists and banks so MIDI still works when the editor is closed.
// Everything here is built on the main thread and read-only afterwards.
#pragma once

#include <algorithm>
#include <atomic>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "modules.hpp"
#include "params.hpp"
#include "presets.hpp"
#include "setlists.hpp"
#include "user_presets.hpp"

namespace pedal {

struct PerformanceData {
  // One vector of complete parameter snapshots per bank.  Bank 0 is ALL, the
  // next entries are factory categories, and the last is USER.
  std::vector<std::vector<std::vector<double>>> banks;
  std::vector<std::vector<std::vector<double>>> setlists;
};

inline std::vector<double> factoryPresetValues(int index) {
  std::vector<double> values(P_COUNT, 0.0);
  if (index < 0 || index >= kPresetCount) return values;
  Shared temp;
  for (int slot = 0; slot < kSlotCount; ++slot) {
    applyModuleDefaults(temp, slot, static_cast<int>(ModuleId::Off), false);
  }
  temp.setFromHost(P_INPUT, 0.0);
  temp.setFromHost(P_OUTPUT, 0.0);
  const Preset& preset = kPresets[index];
  for (int i = 0; i < preset.count; ++i) {
    const PresetValue& v = preset.values[i];
    if (v.param >= 0 && v.param < static_cast<int>(P_COUNT)) {
      temp.setFromHost(static_cast<uint32_t>(v.param), v.value);
    }
  }
  for (uint32_t i = 0; i < P_COUNT; ++i) values[i] = temp.get(i);
  return values;
}

inline std::vector<double> userPresetValues(const UserPreset& preset) {
  std::vector<double> values(P_COUNT, 0.0);
  for (uint32_t i = 0; i < P_COUNT; ++i) {
    values[i] = i < preset.values.size() ? preset.values[i]
                                         : kParamSpecs[i].def;
  }
  return values;
}

inline std::vector<double> padSnapshot(const std::vector<double>& snapshot) {
  std::vector<double> values(P_COUNT, 0.0);
  const size_t n = std::min(snapshot.size(), static_cast<size_t>(P_COUNT));
  for (size_t i = 0; i < n; ++i) values[i] = snapshot[i];
  return values;
}

inline std::shared_ptr<const PerformanceData> loadPerformanceData() {
  auto data = std::make_shared<PerformanceData>();
  const std::vector<UserPreset> users = loadUserPresets(userPresetDir());
  std::vector<std::vector<double>> factory;
  factory.reserve(kPresetCount);
  for (int i = 0; i < kPresetCount; ++i) factory.push_back(factoryPresetValues(i));
  std::vector<std::vector<double>> userValues;
  userValues.reserve(users.size());
  for (const auto& user : users) userValues.push_back(userPresetValues(user));

  std::vector<std::string> categories;
  for (int i = 0; i < kPresetCount; ++i) {
    const std::string category =
        kPresets[i].category ? kPresets[i].category : "Other";
    if (std::find(categories.begin(), categories.end(), category) ==
        categories.end()) {
      categories.push_back(category);
    }
  }

  // Bank 0: ALL.
  std::vector<std::vector<double>> all = factory;
  all.insert(all.end(), userValues.begin(), userValues.end());
  data->banks.push_back(std::move(all));
  for (const std::string& category : categories) {
    std::vector<std::vector<double>> bank;
    for (int i = 0; i < kPresetCount; ++i) {
      const std::string c = kPresets[i].category ? kPresets[i].category : "";
      if (c == category) bank.push_back(factory[static_cast<size_t>(i)]);
    }
    data->banks.push_back(std::move(bank));
  }
  data->banks.push_back(userValues);

  std::unordered_map<std::string, size_t> factoryByName;
  for (int i = 0; i < kPresetCount; ++i) {
    factoryByName[kPresets[i].name] = static_cast<size_t>(i);
  }
  std::unordered_map<std::string, size_t> userByName;
  for (size_t i = 0; i < users.size(); ++i) {
    userByName[users[i].name] = i;
  }

  for (const Setlist& setlist : loadSetlists(setlistDir())) {
    std::vector<std::vector<double>> entries;
    entries.reserve(setlist.entries.size());
    for (const SetlistEntry& entry : setlist.entries) {
      if (!entry.snapshot.empty()) {
        entries.push_back(padSnapshot(entry.snapshot));
      } else if (entry.factory) {
        auto it = factoryByName.find(entry.name);
        if (it != factoryByName.end()) {
          entries.push_back(factory[it->second]);
        } else {
          entries.emplace_back();
        }
      } else {
        auto it = userByName.find(entry.name);
        if (it != userByName.end()) {
          entries.push_back(userValues[it->second]);
        } else {
          entries.emplace_back();
        }
      }
    }
    data->setlists.push_back(std::move(entries));
  }
  return data;
}


class PerformanceController {
public:
  void reload() { std::atomic_store(&data_, loadPerformanceData()); }

  void applyValues(Shared& shared, const std::vector<double>& values) const {
    if (values.empty()) return;
    const size_t n = std::min(values.size(), static_cast<size_t>(P_COUNT));
    for (size_t i = 0; i < n; ++i) {
      shared.set(static_cast<uint32_t>(i), values[i]);
    }
    shared.perfProgramRevision.fetch_add(1, std::memory_order_release);
  }

  void bankStep(Shared& shared, int delta) const {
    auto data = std::atomic_load(&data_);
    if (!data || data->banks.empty()) return;
    const int count = static_cast<int>(data->banks.size());
    int bank = shared.perfBankIndex.load(std::memory_order_relaxed);
    bank = (bank + delta + count) % count;
    shared.perfBankIndex.store(bank, std::memory_order_relaxed);
    shared.perfRevision.fetch_add(1, std::memory_order_release);
  }

  void setlistStep(Shared& shared, int delta) const {
    auto data = std::atomic_load(&data_);
    if (!data || data->setlists.empty()) return;
    const int setCount = static_cast<int>(data->setlists.size());
    int set = std::clamp(
        shared.perfSetlistIndex.load(std::memory_order_relaxed), 0,
        setCount - 1);
    const auto& entries = data->setlists[static_cast<size_t>(set)];
    if (entries.empty()) return;
    const int count = static_cast<int>(entries.size());
    int entry = shared.perfSetlistEntry.load(std::memory_order_relaxed);
    if (entry < 0) {
      entry = delta > 0 ? 0 : count - 1;
    } else {
      entry = (entry + delta + count) % count;
    }
    applyValues(shared, entries[static_cast<size_t>(entry)]);
    shared.perfSetlistIndex.store(set, std::memory_order_relaxed);
    shared.perfSetlistEntry.store(entry, std::memory_order_relaxed);
    shared.perfRevision.fetch_add(1, std::memory_order_release);
  }

  void program(Shared& shared, int program) const {
    if (program < 0) return;
    auto data = std::atomic_load(&data_);
    if (!data) return;
    if (!data->setlists.empty()) {
      const int setCount = static_cast<int>(data->setlists.size());
      const int set = std::clamp(
          shared.perfSetlistIndex.load(std::memory_order_relaxed), 0,
          setCount - 1);
      const auto& entries = data->setlists[static_cast<size_t>(set)];
      if (program < static_cast<int>(entries.size())) {
        applyValues(shared, entries[static_cast<size_t>(program)]);
        shared.perfSetlistEntry.store(program, std::memory_order_relaxed);
        shared.perfRevision.fetch_add(1, std::memory_order_release);
        return;
      }
    }
    if (data->banks.empty()) return;
    const int bankCount = static_cast<int>(data->banks.size());
    const int bank = std::clamp(
        shared.perfBankIndex.load(std::memory_order_relaxed), 0,
        bankCount - 1);
    const auto& presets = data->banks[static_cast<size_t>(bank)];
    if (program < static_cast<int>(presets.size())) {
      applyValues(shared, presets[static_cast<size_t>(program)]);
      shared.perfRevision.fetch_add(1, std::memory_order_release);
    }
  }

private:
  std::shared_ptr<const PerformanceData> data_;
};

} // namespace pedal
