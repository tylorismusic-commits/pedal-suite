// Pedal Suite - Cocoa view host
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace pedal {

struct Shared;
struct PanelState;
class Panel;

// Hosts the cairo Panel inside an NSView.  Pedal Suite's panel draws straight
// from Shared/PanelState, so there is no value-sync callback: the view just
// ticks the panel and repaints.
class CocoaView {
public:
  CocoaView();
  ~CocoaView();
  CocoaView(const CocoaView&) = delete;
  CocoaView& operator=(const CocoaView&) = delete;

  void setContent(Panel* panel, Shared* shared, PanelState* state);
  void attach(void* parentNsView);
  void detach();
  void resize(int width, int height);

  // Called with the paths of files dropped onto the editor.  The plug-in copies
  // them into its user folders and rescans.
  std::function<void(const std::vector<std::string>&)> onFilesDropped;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

} // namespace pedal
