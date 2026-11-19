// Pedal Suite — LV2 UI (X11 + Cairo)
// SPDX-License-Identifier: GPL-3.0-or-later
//
// The host owns the widget size. On every allocation we letterbox the
// 1040x720 panel into whatever we are given and redraw. We never call
// XResizeWindow or the host's ui_resize in response to a configure event:
// suil turns those into another GTK allocation, which arrives back as another
// configure event, and the window flickers and eventually wedges Ardour.
//
// Redraws are throttled to ~30 Hz from the idle interface. Hosts call idle
// from a GTK idle handler, which can run thousands of times a second; drawing
// every call starves the host's main loop.

#include <lv2/core/lv2.h>
#include <lv2/ui/ui.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <cairo-xlib.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <string>

#include "params.hpp"
#include "ui/geometry.hpp"
#include "ui/panel.hpp"

namespace {

constexpr const char* kUiUri =
    "https://github.com/tylorismusic-commits/pedal-suite#ui";
constexpr uint32_t kFirstParam = 4;
constexpr uint32_t kMeterBase = kFirstParam + pedal::P_COUNT;
constexpr uint32_t kMidiInPort = kMeterBase + 15;
constexpr uint32_t kLearnSlotPort = kMidiInPort + 1;
constexpr uint32_t kLearnExprPort = kMidiInPort + 2;
constexpr uint32_t kLearnSlotStatePort = kMidiInPort + 3;
constexpr uint32_t kLearnExprStatePort = kMidiInPort + 4;
constexpr uint32_t kSetlistNextPort = kMidiInPort + 5;
constexpr uint32_t kSetlistPrevPort = kMidiInPort + 6;
constexpr uint32_t kSetlistPcSeqPort = kMidiInPort + 7;
constexpr uint32_t kSetlistPcValuePort = kMidiInPort + 8;
constexpr uint32_t kBankNextPort = kMidiInPort + 9;
constexpr uint32_t kBankPrevPort = kMidiInPort + 10;
constexpr auto kFrameInterval = std::chrono::milliseconds(33);

struct Ui {
  LV2UI_Write_Function write = nullptr;
  LV2UI_Controller controller = nullptr;
  const LV2UI_Resize* hostResize = nullptr;

  pedal::Shared shared;
  pedal::Panel panel;
  pedal::PanelState state;

  Display* dpy = nullptr;
  Window win = 0;
  KeyCode lastKeyCode = 0;
  std::chrono::steady_clock::time_point lastKeyTime{};
  cairo_surface_t* windowSurface = nullptr;
  cairo_surface_t* backing = nullptr;
  cairo_t* cr = nullptr;
  cairo_format_t format = CAIRO_FORMAT_RGB24;

  uint32_t width = pedal::kPanelWidth;
  uint32_t height = pedal::kPanelHeight;

  bool dirty = true;
  int lastLearnSlotSent = -2;
  bool lastLearnExprSent = false;
  std::chrono::steady_clock::time_point lastFrame{};

  double layoutScale() const {
    const double sx = static_cast<double>(width) / pedal::kPanelWidth;
    const double sy = static_cast<double>(height) / pedal::kPanelHeight;
    return std::min(sx, sy);
  }

  double offsetX() const {
    return (static_cast<double>(width) - pedal::kPanelWidth * layoutScale()) * 0.5;
  }

  double offsetY() const {
    return (static_cast<double>(height) - pedal::kPanelHeight * layoutScale()) *
           0.5;
  }

  void rebuildSurfaces() {
    if (cr) {
      cairo_destroy(cr);
      cr = nullptr;
    }
    if (backing) {
      cairo_surface_destroy(backing);
      backing = nullptr;
    }
    backing = cairo_image_surface_create(format, static_cast<int>(width),
                                         static_cast<int>(height));
    cr = cairo_create(backing);
    if (windowSurface) {
      cairo_xlib_surface_set_size(windowSurface, width, height);
    }
  }

  void configure(uint32_t w, uint32_t h) {
    if (w == 0 || h == 0 || (w == width && h == height)) {
      return;
    }
    width = w;
    height = h;
    rebuildSurfaces();
    dirty = true;
  }

  void handleEvent(const XEvent& ev) {
    const double s = layoutScale();
    const double ox = offsetX();
    const double oy = offsetY();
    auto toLogicalX = [&](int x) { return (x - ox) / s; };
    auto toLogicalY = [&](int y) { return (y - oy) / s; };

    switch (ev.type) {
      case ConfigureNotify:
        configure(static_cast<uint32_t>(ev.xconfigure.width),
                  static_cast<uint32_t>(ev.xconfigure.height));
        break;
      case Expose:
        if (ev.xexpose.count == 0) {
          draw();
        }
        break;
      case ButtonPress: {
        if (dpy && win) {
          XSetInputFocus(dpy, win, RevertToParent, CurrentTime);
          XFlush(dpy);
        }
        const bool fine = (ev.xbutton.state & ShiftMask) != 0;
        if (ev.xbutton.button == Button4) {
          panel.wheel(shared, state, toLogicalX(ev.xbutton.x),
                      toLogicalY(ev.xbutton.y), 1.0);
        } else if (ev.xbutton.button == Button5) {
          panel.wheel(shared, state, toLogicalX(ev.xbutton.x),
                      toLogicalY(ev.xbutton.y), -1.0);
        } else if (ev.xbutton.button == Button1) {
          panel.pointerDown(shared, state, toLogicalX(ev.xbutton.x),
                            toLogicalY(ev.xbutton.y), fine);
        } else if (ev.xbutton.button == Button3) {
          panel.beginTextEntry(shared, state, toLogicalX(ev.xbutton.x),
                               toLogicalY(ev.xbutton.y));
        }
        dirty = true;
        break;
      }
      case ButtonRelease:
        panel.pointerUp(shared, state);
        dirty = true;
        break;
      case KeyPress: {
        char buf[8] = {};
        KeySym sym = 0;
        XKeyEvent key = ev.xkey;
        const int n = XLookupString(&key, buf, sizeof(buf), &sym, nullptr);
        if (sym == XK_Return || sym == XK_KP_Enter) {
          panel.textKey(shared, state, pedal::Panel::kTextEnter, 0);
        } else if (sym == XK_Escape) {
          panel.textKey(shared, state, pedal::Panel::kTextEscape, 0);
        } else if (sym == XK_BackSpace) {
          panel.textKey(shared, state, pedal::Panel::kTextBackspace, 0);
        } else if (n > 0) {
          const auto now = std::chrono::steady_clock::now();
          if (ev.xkey.keycode == lastKeyCode &&
              now - lastKeyTime < std::chrono::milliseconds(60)) {
            break;
          }
          lastKeyCode = ev.xkey.keycode;
          lastKeyTime = now;
          panel.textKey(shared, state, 0, buf[0]);
        }
        dirty = true;
        break;
      }
      case MotionNotify: {
        const bool fine = (ev.xmotion.state & ShiftMask) != 0;
        panel.pointerMove(shared, state, toLogicalX(ev.xmotion.x),
                          toLogicalY(ev.xmotion.y), fine);
        dirty = true;
        break;
      }
      default:
        break;
    }
  }

  void draw() {
    if (!cr || !windowSurface || width < 8 || height < 8) {
      return;
    }

    for (uint32_t i = 0; i < pedal::P_COUNT; ++i) {
      if (!shared.takeDirty(i)) {
        continue;
      }
      const float v = static_cast<float>(shared.get(i));
      if (write) {
        write(controller, kFirstParam + i, sizeof(float), 0, &v);
      }
    }

    cairo_save(cr);
    cairo_set_source_rgb(cr, 0.043, 0.047, 0.051);
    cairo_paint(cr);
    cairo_translate(cr, offsetX(), offsetY());
    cairo_scale(cr, layoutScale(), layoutScale());
    cairo_rectangle(cr, 0, 0, pedal::kPanelWidth, pedal::kPanelHeight);
    cairo_clip(cr);
    panel.tick(shared, state);
    panel.draw(cr, shared, state);
    cairo_restore(cr);
    cairo_surface_flush(backing);

    cairo_t* xcr = cairo_create(windowSurface);
    cairo_set_source_surface(xcr, backing, 0, 0);
    cairo_paint(xcr);
    cairo_destroy(xcr);
    cairo_surface_flush(windowSurface);
    XFlush(dpy);
  }

  int idle() {
    if (write && controller) {
      const int learnSlot = shared.midiLearnSlot.load();
      if (learnSlot != lastLearnSlotSent) {
        lastLearnSlotSent = learnSlot;
        const float v = static_cast<float>(learnSlot);
        write(controller, kLearnSlotPort, sizeof(float), 0, &v);
      }
      const bool learnExpr = shared.midiLearnExpr.load();
      if (learnExpr != lastLearnExprSent) {
        lastLearnExprSent = learnExpr;
        const float v = learnExpr ? 1.0f : 0.0f;
        write(controller, kLearnExprPort, sizeof(float), 0, &v);
      }
    }
    while (dpy && XPending(dpy) > 0) {
      XEvent ev;
      XNextEvent(dpy, &ev);
      handleEvent(ev);
    }
    const auto now = std::chrono::steady_clock::now();
    if (dirty || now - lastFrame >= kFrameInterval) {
      draw();
      dirty = false;
      lastFrame = now;
    }
    return 0;
  }
};

LV2UI_Handle uiInstantiate(const LV2UI_Descriptor*, const char* /*pluginUri*/,
                           const char* bundlePath, LV2UI_Write_Function write,
                           LV2UI_Controller controller, LV2UI_Widget* widget,
                           const LV2_Feature* const* features) {
  void* parent = nullptr;
  const LV2UI_Resize* hostResize = nullptr;
  const float* scaleHint = nullptr;
  for (const LV2_Feature* const* f = features; f && *f; ++f) {
    if (!std::strcmp((*f)->URI, LV2_UI__parent)) {
      parent = (*f)->data;
    } else if (!std::strcmp((*f)->URI, LV2_UI__resize)) {
      hostResize = static_cast<const LV2UI_Resize*>(f[0]->data);
    } else if (!std::strcmp((*f)->URI, LV2_UI__scaleFactor)) {
      scaleHint = static_cast<const float*>(f[0]->data);
    }
  }

  auto* ui = new Ui();
  ui->write = write;
  ui->controller = controller;
  ui->hostResize = hostResize;

  ui->dpy = XOpenDisplay(nullptr);
  if (!ui->dpy) {
    delete ui;
    return nullptr;
  }

  const int screen = DefaultScreen(ui->dpy);
  const double initialScale =
      scaleHint && *scaleHint > 0.0f ? pedal::clampScale(*scaleHint) : 1.0;
  const pedal::UiSize initial = pedal::sizeForScale(initialScale);
  ui->width = initial.width;
  ui->height = initial.height;

  const Window parentWindow =
      parent ? static_cast<Window>(reinterpret_cast<uintptr_t>(parent))
             : RootWindow(ui->dpy, screen);

  XSetWindowAttributes attrs{};
  attrs.background_pixel = BlackPixel(ui->dpy, screen);
  attrs.event_mask = ExposureMask | ButtonPressMask | ButtonReleaseMask |
                     PointerMotionMask | Button1MotionMask | ButtonMotionMask |
                     KeyPressMask | StructureNotifyMask;
  ui->win = XCreateWindow(ui->dpy, parentWindow, 0, 0, ui->width, ui->height, 0,
                          CopyFromParent, InputOutput, CopyFromParent,
                          CWBackPixel | CWEventMask, &attrs);
  XStoreName(ui->dpy, ui->win, "Pedal Suite");
  XMapRaised(ui->dpy, ui->win);

  {
    const pedal::UiSize minSize = pedal::sizeForScale(pedal::kMinUiScale);
    const pedal::UiSize maxSize = pedal::sizeForScale(pedal::kMaxUiScale);
    XSizeHints hints{};
    hints.flags = PMinSize | PMaxSize | PBaseSize | PWinGravity;
    hints.min_width = static_cast<int>(minSize.width);
    hints.min_height = static_cast<int>(minSize.height);
    hints.max_width = static_cast<int>(maxSize.width);
    hints.max_height = static_cast<int>(maxSize.height);
    hints.base_width = pedal::kPanelWidth;
    hints.base_height = pedal::kPanelHeight;
    hints.win_gravity = NorthWestGravity;
    XSetWMNormalHints(ui->dpy, ui->win, &hints);
  }

  ui->format = DefaultDepth(ui->dpy, screen) == 32 ? CAIRO_FORMAT_ARGB32
                                                   : CAIRO_FORMAT_RGB24;
  ui->windowSurface = cairo_xlib_surface_create(
      ui->dpy, ui->win, DefaultVisual(ui->dpy, screen), ui->width, ui->height);
  ui->rebuildSurfaces();
  XFlush(ui->dpy);

  std::string dir = bundlePath ? bundlePath : "";
  ui->panel.loadFont(dir);

  if (ui->hostResize && ui->hostResize->ui_resize) {
    ui->hostResize->ui_resize(ui->hostResize->handle,
                              static_cast<int>(ui->width),
                              static_cast<int>(ui->height));
  }

  *widget = reinterpret_cast<LV2UI_Widget>(static_cast<uintptr_t>(ui->win));
  return ui;
}

void uiCleanup(LV2UI_Handle handle) {
  auto* ui = static_cast<Ui*>(handle);
  if (ui->cr) {
    cairo_destroy(ui->cr);
  }
  if (ui->backing) {
    cairo_surface_destroy(ui->backing);
  }
  if (ui->windowSurface) {
    cairo_surface_destroy(ui->windowSurface);
  }
  if (ui->dpy) {
    XDestroyWindow(ui->dpy, ui->win);
    XCloseDisplay(ui->dpy);
  }
  delete ui;
}

void uiPortEvent(LV2UI_Handle handle, uint32_t port, uint32_t size,
                 uint32_t format, const void* buffer) {
  auto* ui = static_cast<Ui*>(handle);
  if (format != 0 || size != sizeof(float) || !buffer) {
    return;
  }
  const float v = *static_cast<const float*>(buffer);
  if (port >= kFirstParam && port < kFirstParam + pedal::P_COUNT) {
    ui->shared.setFromHost(port - kFirstParam, static_cast<double>(v));
    ui->dirty = true;
    return;
  }
  if (port == kLearnSlotStatePort) {
    const int slot = static_cast<int>(std::lround(v));
    ui->shared.midiLearnSlotState.store(slot);
    if (slot < 0) ui->shared.midiLearnSlot.store(-1);
    ui->dirty = true;
    return;
  }
  if (port == kLearnExprStatePort) {
    const bool on = v > 0.5f;
    ui->shared.midiLearnExprState.store(on);
    if (!on) ui->shared.midiLearnExpr.store(false);
    ui->dirty = true;
    return;
  }
  if (port == kSetlistNextPort) {
    ui->shared.midiSetlistNextSeq.store(
        static_cast<uint32_t>(std::lround(v)));
    ui->dirty = true;
    return;
  }
  if (port == kSetlistPrevPort) {
    ui->shared.midiSetlistPrevSeq.store(
        static_cast<uint32_t>(std::lround(v)));
    ui->dirty = true;
    return;
  }
  if (port == kSetlistPcSeqPort) {
    ui->shared.midiSetlistPcSeq.store(
        static_cast<uint32_t>(std::lround(v)));
    ui->dirty = true;
    return;
  }
  if (port == kSetlistPcValuePort) {
    ui->shared.midiSetlistPcValue.store(static_cast<int>(std::lround(v)));
    ui->dirty = true;
    return;
  }
  if (port == kBankNextPort) {
    ui->shared.midiBankNextSeq.store(
        static_cast<uint32_t>(std::lround(v)));
    ui->dirty = true;
    return;
  }
  if (port == kBankPrevPort) {
    ui->shared.midiBankPrevSeq.store(
        static_cast<uint32_t>(std::lround(v)));
    ui->dirty = true;
    return;
  }
  switch (port) {
    case kMeterBase: ui->shared.inPeakLDb.store(v); break;
    case kMeterBase + 1: ui->shared.inPeakRDb.store(v); break;
    case kMeterBase + 2: ui->shared.outPeakLDb.store(v); break;
    case kMeterBase + 3: ui->shared.outPeakRDb.store(v); break;
    case kMeterBase + 4: ui->shared.inPeakDb.store(v); break;
    case kMeterBase + 5: ui->shared.outPeakDb.store(v); break;
    case kMeterBase + 6:
    case kMeterBase + 7:
    case kMeterBase + 8:
    case kMeterBase + 9:
    case kMeterBase + 10:
    case kMeterBase + 11:
    case kMeterBase + 12:
    case kMeterBase + 13:
      ui->shared.slotGrDb[port - (kMeterBase + 6)].store(v);
      break;
    default:
      break;
  }
}

int uiIdle(LV2UI_Handle handle) { return static_cast<Ui*>(handle)->idle(); }

const LV2UI_Idle_Interface kIdleInterface = {uiIdle};

const void* uiExtensionData(const char* uri) {
  if (uri && !std::strcmp(uri, LV2_UI__idleInterface)) {
    return &kIdleInterface;
  }
  return nullptr;
}

const LV2UI_Descriptor kUiDescriptor = {kUiUri, uiInstantiate, uiCleanup,
                                        uiPortEvent, uiExtensionData};

} // namespace

LV2_SYMBOL_EXPORT const LV2UI_Descriptor* lv2ui_descriptor(uint32_t index) {
  return index == 0 ? &kUiDescriptor : nullptr;
}
