// Pedal Suite — setlists / banks
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <vector>

#include "user_presets.hpp"

namespace pedal {

inline constexpr const char* kSetlistExt = ".setlist";

struct SetlistEntry {
  std::string name;
  bool factory = false;
  // A snapshot captures the live board, including unsaved edits.  Reference
  // entries leave this empty and point at a factory or user preset by name.
  std::vector<double> snapshot;
};

struct Setlist {
  std::string name;
  std::vector<SetlistEntry> entries;
};

inline std::filesystem::path setlistDir() {
  return userPresetDir().parent_path() / "setlists";
}

inline bool writeSetlist(const std::filesystem::path& dir,
                         const Setlist& setlist) {
  const std::string name = sanitizePresetName(setlist.name);
  if (name.empty()) return false;
  std::error_code ec;
  std::filesystem::create_directories(dir, ec);
  if (ec) return false;
  const std::filesystem::path finalPath = dir / (name + kSetlistExt);
  const std::filesystem::path tempPath =
      dir / (name + std::string(kSetlistExt) + ".tmp");
  std::ofstream out(tempPath, std::ios::trunc);
  if (!out) return false;
  out << "PedalSuiteSetlist 1\n";
  out << "name " << name << "\n";
  for (const SetlistEntry& e : setlist.entries) {
    out << "entry " << (e.snapshot.empty() ? (e.factory ? "F " : "U ") : "S ")
        << e.name << "\n";
    if (!e.snapshot.empty()) {
      out.precision(17);
      out << "snap " << e.snapshot.size();
      for (double v : e.snapshot) out << ' ' << v;
      out << "\n";
    }
  }
  out.close();
  if (!out) return false;
  std::filesystem::rename(tempPath, finalPath, ec);
  if (ec) {
    std::filesystem::remove(finalPath, ec);
    ec.clear();
    std::filesystem::rename(tempPath, finalPath, ec);
  }
  return !ec;
}

inline bool deleteSetlist(const std::filesystem::path& dir,
                          const std::string& rawName) {
  const std::string name = sanitizePresetName(rawName);
  if (name.empty()) return false;
  std::error_code ec;
  return std::filesystem::remove(dir / (name + kSetlistExt), ec) && !ec;
}

inline bool renameSetlist(const std::filesystem::path& dir,
                          const std::string& oldName,
                          const std::string& newName,
                          const Setlist& setlist) {
  const std::string oldClean = sanitizePresetName(oldName);
  const std::string newClean = sanitizePresetName(newName);
  if (oldClean.empty() || newClean.empty()) return false;
  if (oldClean != newClean) {
    std::error_code ec;
    if (std::filesystem::exists(dir / (newClean + kSetlistExt), ec) && !ec) {
      return false;
    }
  }
  Setlist renamed = setlist;
  renamed.name = newClean;
  if (!writeSetlist(dir, renamed)) return false;
  if (oldClean != newClean) deleteSetlist(dir, oldClean);
  return true;
}

inline std::vector<Setlist> loadSetlists(const std::filesystem::path& dir) {
  std::vector<Setlist> out;
  std::error_code ec;
  if (!std::filesystem::is_directory(dir, ec)) return out;
  std::vector<std::filesystem::path> files;
  for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
    if (ec) break;
    if (!entry.is_regular_file()) continue;
    if (entry.path().extension() != kSetlistExt) continue;
    files.push_back(entry.path());
  }
  std::sort(files.begin(), files.end());
  for (const auto& path : files) {
    std::ifstream in(path);
    if (!in) continue;
    std::string magic;
    int version = 0;
    in >> magic >> version;
    if (magic != "PedalSuiteSetlist" || version != 1) continue;
    Setlist setlist;
    std::string key;
    while (in >> key) {
      if (key == "name") {
        std::getline(in, setlist.name);
        const size_t a = setlist.name.find_first_not_of(' ');
        setlist.name =
            a == std::string::npos ? std::string() : setlist.name.substr(a);
      } else if (key == "entry") {
        char kind = 'U';
        in >> kind;
        std::string entryName;
        std::getline(in, entryName);
        const size_t a = entryName.find_first_not_of(' ');
        if (a == std::string::npos) continue;
        entryName = entryName.substr(a);
        if (!entryName.empty()) {
          SetlistEntry entry;
          entry.name = entryName;
          entry.factory = kind == 'F' || kind == 'f';
          setlist.entries.push_back(std::move(entry));
        }
      } else if (key == "snap") {
        if (!setlist.entries.empty()) {
          size_t count = 0;
          in >> count;
          count = std::min<size_t>(count, static_cast<size_t>(P_COUNT));
          std::vector<double> values;
          values.reserve(count);
          bool ok = true;
          for (size_t i = 0; i < count; ++i) {
            double v = 0.0;
            if (!(in >> v)) {
              ok = false;
              break;
            }
            values.push_back(v);
          }
          if (ok) setlist.entries.back().snapshot = std::move(values);
        }
      } else {
        std::string ignored;
        std::getline(in, ignored);
      }
    }
    if (setlist.name.empty()) {
      setlist.name = path.stem().string();
    }
    out.push_back(std::move(setlist));
  }
  std::sort(out.begin(), out.end(), [](const Setlist& a, const Setlist& b) {
    return a.name < b.name;
  });
  return out;
}

} // namespace pedal
