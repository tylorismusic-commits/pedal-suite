// Pedal Suite — user preset files
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <vector>

#include "params.hpp"

namespace pedal {

// Files written by 0.3.3 and earlier used Suite Rack's extension by mistake.
// New files use .pedal-suite; the loader still accepts the legacy extension.
inline constexpr const char* kUserPresetExt = ".pedal-suite";
inline constexpr const char* kLegacyPresetExt = ".suite-rack";

struct UserPreset {
  std::string name;
  std::vector<double> values;
};

struct ModulePreset {
  std::string name;
  int type = 0;
  std::array<float, 12> controls{};
};

inline std::filesystem::path userPresetDir() {
  const char* xdg = std::getenv("XDG_CONFIG_HOME");
  std::filesystem::path base;
  if (xdg && *xdg) {
    base = xdg;
  } else {
    const char* home = std::getenv("HOME");
    base = home && *home ? std::filesystem::path(home) / ".config"
                         : std::filesystem::path(".");
  }
  return base / "PedalSuite" / "presets";
}

inline std::filesystem::path modulePresetDir() {
  return userPresetDir().parent_path() / "module-presets";
}

inline std::string sanitizePresetName(const std::string& raw) {
  std::string out;
  for (char c : raw) {
    const unsigned char u = static_cast<unsigned char>(c);
    if (std::isalnum(u) || c == ' ' || c == '-' || c == '_' || c == '.' ||
        c == '(' || c == ')') {
      out.push_back(c);
    } else {
      out.push_back('_');
    }
  }
  const size_t a = out.find_first_not_of(' ');
  if (a == std::string::npos) return {};
  const size_t b = out.find_last_not_of(' ');
  out = out.substr(a, b - a + 1);
  if (out.size() > 60) out.resize(60);
  return out;
}

inline bool writeUserPreset(const std::filesystem::path& dir,
                            const std::string& rawName,
                            const std::vector<double>& values) {
  const std::string name = sanitizePresetName(rawName);
  if (name.empty()) return false;
  std::error_code ec;
  std::filesystem::create_directories(dir, ec);
  if (ec) return false;

  const std::filesystem::path finalPath = dir / (name + kUserPresetExt);
  const std::filesystem::path tempPath =
      dir / (name + std::string(kUserPresetExt) + ".tmp");
  std::ofstream out(tempPath, std::ios::trunc);
  if (!out) return false;
  out << "PedalSuitePreset 1\n";
  out << "name " << name << "\n";
  out.precision(17);
  for (uint32_t i = 0; i < values.size(); ++i) {
    out << i << ' ' << values[i] << '\n';
  }
  out.close();
  if (!out) return false;
  std::filesystem::rename(tempPath, finalPath, ec);
  if (ec) {
    std::filesystem::remove(finalPath, ec);
    ec.clear();
    std::filesystem::rename(tempPath, finalPath, ec);
  }
  if (!ec) {
    // Drop a migration duplicate with the old extension.
    std::error_code ignored;
    std::filesystem::remove(dir / (name + kLegacyPresetExt), ignored);
  }
  return !ec;
}

inline bool saveUserPreset(const std::filesystem::path& dir,
                           const std::string& rawName, const Shared& shared) {
  std::vector<double> values;
  values.reserve(P_COUNT);
  for (uint32_t i = 0; i < P_COUNT; ++i) values.push_back(shared.get(i));
  return writeUserPreset(dir, rawName, values);
}

inline bool deleteUserPreset(const std::filesystem::path& dir,
                             const std::string& rawName) {
  const std::string name = sanitizePresetName(rawName);
  if (name.empty()) return false;
  std::error_code ec;
  if (std::filesystem::remove(dir / (name + kUserPresetExt), ec) && !ec) {
    return true;
  }
  ec.clear();
  return std::filesystem::remove(dir / (name + kLegacyPresetExt), ec) && !ec;
}

inline bool renameUserPreset(const std::filesystem::path& dir,
                             const std::string& oldName,
                             const std::string& newName,
                             const std::vector<double>& values) {
  const std::string oldClean = sanitizePresetName(oldName);
  const std::string newClean = sanitizePresetName(newName);
  if (oldClean.empty() || newClean.empty()) return false;
  if (oldClean != newClean) {
    std::error_code ec;
    if ((std::filesystem::exists(dir / (newClean + kUserPresetExt), ec) &&
         !ec) ||
        (std::filesystem::exists(dir / (newClean + kLegacyPresetExt), ec) &&
         !ec)) {
      return false;
    }
  }
  if (!writeUserPreset(dir, newClean, values)) return false;
  if (oldClean != newClean) deleteUserPreset(dir, oldClean);
  return true;
}

inline bool writeModulePreset(const std::filesystem::path& dir,
                              const std::string& rawName, int type,
                              const std::array<float, 12>& controls) {
  const std::string name = sanitizePresetName(rawName);
  if (name.empty()) return false;
  std::error_code ec;
  std::filesystem::create_directories(dir, ec);
  if (ec) return false;
  const std::filesystem::path finalPath = dir / (name + ".suite-module");
  const std::filesystem::path tempPath = dir / (name + ".suite-module.tmp");
  std::ofstream out(tempPath, std::ios::trunc);
  if (!out) return false;
  out << "PedalSuiteModulePreset 1\n";
  out << "name " << name << "\n";
  out << "type " << type << "\n";
  out.precision(9);
  for (size_t i = 0; i < controls.size(); ++i) {
    out << i << ' ' << controls[i] << '\n';
  }
  out.close();
  if (!out) return false;
  std::filesystem::rename(tempPath, finalPath, ec);
  if (ec) {
    std::filesystem::remove(finalPath, ec);
    ec.clear();
    std::filesystem::rename(tempPath, finalPath, ec);
  }
  if (!ec) {
    // Drop a migration duplicate with the old extension.
    std::error_code ignored;
    std::filesystem::remove(dir / (name + kLegacyPresetExt), ignored);
  }
  return !ec;
}

inline bool saveModulePreset(const std::filesystem::path& dir,
                             const std::string& rawName, int type,
                             const std::array<float, 12>& controls) {
  return writeModulePreset(dir, rawName, type, controls);
}

inline bool deleteModulePreset(const std::filesystem::path& dir,
                               const std::string& rawName) {
  const std::string name = sanitizePresetName(rawName);
  if (name.empty()) return false;
  std::error_code ec;
  return std::filesystem::remove(dir / (name + ".suite-module"), ec) && !ec;
}

inline bool renameModulePreset(const std::filesystem::path& dir,
                               const std::string& oldName,
                               const std::string& newName, int type,
                               const std::array<float, 12>& controls) {
  const std::string oldClean = sanitizePresetName(oldName);
  const std::string newClean = sanitizePresetName(newName);
  if (oldClean.empty() || newClean.empty()) return false;
  if (oldClean != newClean) {
    std::error_code ec;
    if (std::filesystem::exists(dir / (newClean + ".suite-module"), ec) &&
        !ec) {
      return false;
    }
  }
  if (!writeModulePreset(dir, newClean, type, controls)) return false;
  if (oldClean != newClean) deleteModulePreset(dir, oldClean);
  return true;
}

inline std::vector<ModulePreset> loadModulePresets(
    const std::filesystem::path& dir) {
  std::vector<ModulePreset> out;
  std::error_code ec;
  if (!std::filesystem::is_directory(dir, ec)) return out;
  std::vector<std::filesystem::path> files;
  for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
    if (ec) break;
    if (!entry.is_regular_file()) continue;
    if (entry.path().extension() != ".suite-module") continue;
    files.push_back(entry.path());
  }
  std::sort(files.begin(), files.end());
  for (const auto& path : files) {
    std::ifstream in(path);
    if (!in) continue;
    std::string magic;
    int version = 0;
    in >> magic >> version;
    if (magic != "PedalSuiteModulePreset" || version != 1) continue;
    ModulePreset preset;
    std::string key;
    in >> key;
    if (key == "name") {
      std::getline(in, preset.name);
      const size_t a = preset.name.find_first_not_of(' ');
      preset.name = a == std::string::npos ? "" : preset.name.substr(a);
    }
    in >> key;
    if (key == "type") in >> preset.type;
    if (preset.name.empty()) preset.name = path.stem().string();
    int index = 0;
    float value = 0.0f;
    while (in >> index >> value) {
      if (index >= 0 && index < 12) {
        preset.controls[static_cast<size_t>(index)] = value;
      }
    }
    out.push_back(std::move(preset));
  }
  return out;
}

inline std::vector<UserPreset> loadUserPresets(
    const std::filesystem::path& dir) {
  std::vector<UserPreset> out;
  std::error_code ec;
  if (!std::filesystem::is_directory(dir, ec)) return out;
  std::vector<std::filesystem::path> files;
  for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
    if (ec) break;
    if (!entry.is_regular_file()) continue;
    const std::string ext = entry.path().extension().string();
    if (ext != kUserPresetExt && ext != kLegacyPresetExt) continue;
    files.push_back(entry.path());
  }
  std::sort(files.begin(), files.end());

  for (const auto& path : files) {
    std::ifstream in(path);
    if (!in) continue;
    std::string magic;
    int version = 0;
    in >> magic >> version;
    if (magic != "PedalSuitePreset" || version != 1) continue;
    UserPreset preset;
    preset.values.assign(P_COUNT, 0.0);
    std::string key;
    in >> key;
    if (key == "name") {
      std::getline(in, preset.name);
      const size_t a = preset.name.find_first_not_of(' ');
      preset.name =
          a == std::string::npos ? "" : preset.name.substr(a);
    }
    if (preset.name.empty()) preset.name = path.stem().string();
    int index = 0;
    double value = 0.0;
    while (in >> index >> value) {
      if (index >= 0 && index < static_cast<int>(P_COUNT)) {
        preset.values[static_cast<size_t>(index)] = value;
      }
    }
    out.push_back(std::move(preset));
  }
  return out;
}

} // namespace pedal
