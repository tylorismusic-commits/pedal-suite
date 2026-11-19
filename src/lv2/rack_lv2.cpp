// Pedal Suite — LV2 plugin
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Control ports mirror the CLAP parameter ids: port = kPortFirstParam + ParamId.
// The meter ports follow the parameter block in the same order as the LV2 UI.

#include <lv2/atom/atom.h>
#include <lv2/atom/util.h>
#include <lv2/core/lv2.h>
#include <lv2/state/state.h>
#include <lv2/urid/urid.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

#include "dsp/engine.hpp"
#include "dsp/ir_bank.hpp"
#include "midi_map.hpp"
#include "modules.hpp"
#include "params.hpp"
#include "state.hpp"

namespace {

constexpr const char* kPluginUri =
    "https://github.com/tylorismusic-commits/pedal-suite";

enum Port : uint32_t {
  PORT_AUDIO_IN_L = 0,
  PORT_AUDIO_IN_R = 1,
  PORT_AUDIO_OUT_L = 2,
  PORT_AUDIO_OUT_R = 3,
  PORT_FIRST_PARAM = 4,
  PORT_IN_PEAK_L = PORT_FIRST_PARAM + pedal::P_COUNT,
  PORT_IN_PEAK_R,
  PORT_OUT_PEAK_L,
  PORT_OUT_PEAK_R,
  PORT_IN_PEAK,
  PORT_OUT_PEAK,
  PORT_SLOT0_GR,
  PORT_SLOT1_GR,
  PORT_SLOT2_GR,
  PORT_SLOT3_GR,
  PORT_SLOT4_GR,
  PORT_SLOT5_GR,
  PORT_SLOT6_GR,
  PORT_SLOT7_GR,
  PORT_LATENCY,
  PORT_MIDI_IN,
  PORT_MIDI_LEARN_SLOT,
  PORT_MIDI_LEARN_EXPR,
  PORT_MIDI_LEARN_SLOT_STATE,
  PORT_MIDI_LEARN_EXPR_STATE,
  PORT_COUNT
};

struct Plugin {
  float sampleRate = 48000.0f;
  pedal::Shared shared;
  pedal::Engine engine;
  pedal::Meters meters;

  float* audioInL = nullptr;
  float* audioInR = nullptr;
  float* audioOutL = nullptr;
  float* audioOutR = nullptr;

  float* params[pedal::P_COUNT] = {};
  bool portsSeen = false;
  float lastPort[pedal::P_COUNT] = {};

  float* inPeakL = nullptr;
  float* inPeakR = nullptr;
  float* outPeakL = nullptr;
  float* outPeakR = nullptr;
  float* inPeak = nullptr;
  float* outPeak = nullptr;
  float* slotGr[pedal::kSlotCount] = {};
  float* latency = nullptr;
  const LV2_Atom_Sequence* midiIn = nullptr;
  float* midiLearnSlotPort = nullptr;
  float* midiLearnExprPort = nullptr;
  float* midiLearnSlotStatePort = nullptr;
  float* midiLearnExprStatePort = nullptr;
  float lastLearnSlot = -2.0f;
  float lastLearnExpr = -1.0f;
  uint8_t lastCc[128] = {};
  std::shared_ptr<const pedal::IrBank> irBank;
  int lastIrIndex = -2;
  bool irDirty = true;
  std::vector<std::string> namNames;
  int lastNamIndex = -2;

  const LV2_URID_Map* map = nullptr;
  LV2_URID stateKey = 0;
  LV2_URID legacyStateKey = 0;
  LV2_URID chunkType = 0;
  LV2_URID midiEventType = 0;
};

LV2_Handle instantiate(const LV2_Descriptor*, double rate, const char*,
                       const LV2_Feature* const* features) {
  auto* p = new Plugin();
  p->sampleRate = static_cast<float>(rate);
  for (const LV2_Feature* const* f = features; f && *f; ++f) {
    if (!std::strcmp((*f)->URI, LV2_URID__map) && (*f)->data) {
      p->map =
          static_cast<const LV2_URID_Map*>(static_cast<const void*>((*f)->data));
    }
  }
  if (p->map) {
    p->stateKey = p->map->map(
        p->map->handle,
        "https://github.com/tylorismusic-commits/pedal-suite#state");
    p->legacyStateKey = p->map->map(
        p->map->handle,
        "https://github.com/tylorismusic-commits/suite-rack#state");
    p->chunkType = p->map->map(p->map->handle, LV2_ATOM__Chunk);
    p->midiEventType = p->map->map(
        p->map->handle, "http://lv2plug.in/ns/ext/midi#MidiEvent");
  }
  p->engine.prepare(p->sampleRate);
  pedal::initializeRack(p->shared);
  p->irBank = pedal::loadIrBank(
      pedal::irDir(), static_cast<int>(std::lround(p->sampleRate)));
  p->namNames = pedal::listNamFiles(pedal::namDir());
  p->shared.namBankCount.store(
      static_cast<int>(p->namNames.size()), std::memory_order_relaxed);
  p->lastNamIndex = -2;
  p->shared.irBankCount.store(
      p->irBank ? static_cast<int>(p->irBank->entries.size()) : 0,
      std::memory_order_relaxed);
  p->engine.setControls(pedal::controlsFrom(p->shared), p->shared.revision());
  return p;
}

void connectPort(LV2_Handle handle, uint32_t port, void* data) {
  auto* p = static_cast<Plugin*>(handle);
  switch (port) {
    case PORT_AUDIO_IN_L: p->audioInL = static_cast<float*>(data); break;
    case PORT_AUDIO_IN_R: p->audioInR = static_cast<float*>(data); break;
    case PORT_AUDIO_OUT_L: p->audioOutL = static_cast<float*>(data); break;
    case PORT_AUDIO_OUT_R: p->audioOutR = static_cast<float*>(data); break;
    case PORT_IN_PEAK_L: p->inPeakL = static_cast<float*>(data); break;
    case PORT_IN_PEAK_R: p->inPeakR = static_cast<float*>(data); break;
    case PORT_OUT_PEAK_L: p->outPeakL = static_cast<float*>(data); break;
    case PORT_OUT_PEAK_R: p->outPeakR = static_cast<float*>(data); break;
    case PORT_IN_PEAK: p->inPeak = static_cast<float*>(data); break;
    case PORT_OUT_PEAK: p->outPeak = static_cast<float*>(data); break;
    case PORT_SLOT0_GR: p->slotGr[0] = static_cast<float*>(data); break;
    case PORT_SLOT1_GR: p->slotGr[1] = static_cast<float*>(data); break;
    case PORT_SLOT2_GR: p->slotGr[2] = static_cast<float*>(data); break;
    case PORT_SLOT3_GR: p->slotGr[3] = static_cast<float*>(data); break;
    case PORT_SLOT4_GR: p->slotGr[4] = static_cast<float*>(data); break;
    case PORT_SLOT5_GR: p->slotGr[5] = static_cast<float*>(data); break;
    case PORT_SLOT6_GR: p->slotGr[6] = static_cast<float*>(data); break;
    case PORT_SLOT7_GR: p->slotGr[7] = static_cast<float*>(data); break;
    case PORT_LATENCY: p->latency = static_cast<float*>(data); break;
    case PORT_MIDI_IN:
      p->midiIn = static_cast<const LV2_Atom_Sequence*>(data);
      break;
    case PORT_MIDI_LEARN_SLOT:
      p->midiLearnSlotPort = static_cast<float*>(data);
      break;
    case PORT_MIDI_LEARN_EXPR:
      p->midiLearnExprPort = static_cast<float*>(data);
      break;
    case PORT_MIDI_LEARN_SLOT_STATE:
      p->midiLearnSlotStatePort = static_cast<float*>(data);
      break;
    case PORT_MIDI_LEARN_EXPR_STATE:
      p->midiLearnExprStatePort = static_cast<float*>(data);
      break;
    default:
      if (port >= PORT_FIRST_PARAM &&
          port < PORT_FIRST_PARAM + pedal::P_COUNT) {
        p->params[port - PORT_FIRST_PARAM] = static_cast<float*>(data);
      }
      break;
  }
}

void activate(LV2_Handle handle) {
  auto* p = static_cast<Plugin*>(handle);
  p->engine.reset();
  p->irBank = pedal::loadIrBank(
      pedal::irDir(), static_cast<int>(std::lround(p->sampleRate)));
  p->namNames = pedal::listNamFiles(pedal::namDir());
  p->shared.namBankCount.store(
      static_cast<int>(p->namNames.size()), std::memory_order_relaxed);
  p->lastNamIndex = -2;
  p->shared.irBankCount.store(
      p->irBank ? static_cast<int>(p->irBank->entries.size()) : 0,
      std::memory_order_relaxed);
  p->irDirty = true;
  p->engine.setControls(pedal::controlsFrom(p->shared), p->shared.revision());
  if (p->latency) {
    *p->latency = static_cast<float>(p->engine.latencySamples());
  }
}

void deactivate(LV2_Handle) {}

void run(LV2_Handle handle, uint32_t frames) {
  auto* p = static_cast<Plugin*>(handle);
  if (!p->audioInL || !p->audioOutL) return;
  if (!p->audioInR) p->audioInR = p->audioInL;
  if (!p->audioOutR) p->audioOutR = p->audioOutL;

  for (uint32_t i = 0; i < pedal::P_COUNT; ++i) {
    float* v = p->params[i];
    if (!v) continue;
    if (!p->portsSeen || *v != p->lastPort[i]) {
      p->shared.setFromHost(i, static_cast<double>(*v));
      p->lastPort[i] = *v;
    }
  }
  p->portsSeen = true;

  if (p->midiLearnSlotPort && *p->midiLearnSlotPort != p->lastLearnSlot) {
    p->lastLearnSlot = *p->midiLearnSlotPort;
    p->shared.midiLearnSlot.store(
        static_cast<int>(std::lround(*p->midiLearnSlotPort)));
  }
  if (p->midiLearnExprPort && *p->midiLearnExprPort != p->lastLearnExpr) {
    p->lastLearnExpr = *p->midiLearnExprPort;
    p->shared.midiLearnExpr.store(*p->midiLearnExprPort > 0.5f);
  }

  if (p->midiIn && p->midiEventType) {
    LV2_ATOM_SEQUENCE_FOREACH(p->midiIn, ev) {
      if (!ev || ev->body.type != p->midiEventType || ev->body.size < 3) {
        continue;
      }
      const uint8_t* msg = reinterpret_cast<const uint8_t*>(ev + 1);
      const uint8_t status = msg[0] & 0xF0;
      if (status == 0x90 && msg[2] > 0) {
        const int note = msg[1] & 0x7F;
        const int learnSlot = p->shared.midiLearnSlot.load();
        if (learnSlot >= 0) {
          pedal::assignNoteMapping(p->shared, learnSlot, note);
          p->shared.midiLearnSlot.store(-1);
          p->shared.midiLearnSlotState.store(-1);
          continue;
        }
        const int slot = pedal::mappedNoteSlot(p->shared, note);
        if (slot >= 0) pedal::toggleSlotBypass(p->shared, slot);
      } else if (status == 0xB0) {
        const int cc = msg[1] & 0x7F;
        const int learnSlot = p->shared.midiLearnSlot.load();
        if (learnSlot >= 0) {
          pedal::assignCcMapping(p->shared, learnSlot, cc);
          p->shared.midiLearnSlot.store(-1);
          p->shared.midiLearnSlotState.store(-1);
          continue;
        }
        if (p->shared.midiLearnExpr.exchange(false)) {
          p->shared.set(pedal::P_MIDI_EXPR_CC, static_cast<double>(cc));
          p->shared.set(pedal::P_MIDI_EXPR_ON, 1.0);
          p->shared.midiLearnExprState.store(false);
          continue;
        }
        if (pedal::expressionEnabled(p->shared)) {
          if (cc == pedal::expressionCc(p->shared)) {
            pedal::applyExpression(p->shared, (msg[2] & 0x7F) / 127.0f);
          }
          continue;
        }
        if (cc == 11 || cc == 4) {
          pedal::applyExpression(p->shared, (msg[2] & 0x7F) / 127.0f);
          continue;
        }
        const bool now = (msg[2] & 0x7F) >= 64;
        const bool was = p->lastCc[cc] >= 64;
        p->lastCc[cc] = msg[2] & 0x7F;
        if (now && !was) {
          const int slot = pedal::mappedCcSlot(p->shared, cc);
          if (slot >= 0) pedal::toggleSlotBypass(p->shared, slot);
        }
      }
    }
  }

  if (p->midiLearnSlotStatePort) {
    *p->midiLearnSlotStatePort =
        static_cast<float>(p->shared.midiLearnSlotState.load());
  }
  if (p->midiLearnExprStatePort) {
    *p->midiLearnExprStatePort =
        p->shared.midiLearnExprState.load() ? 1.0f : 0.0f;
  }

  p->engine.setControls(pedal::controlsFrom(p->shared), p->shared.revision());

  const int irIndex =
      static_cast<int>(std::lround(p->shared.get(pedal::P_CAB_IR)));
  if (irIndex != p->lastIrIndex || p->irDirty) {
    p->lastIrIndex = irIndex;
    p->irDirty = false;
    const pedal::IrEntry* entry = nullptr;
    if (p->irBank && irIndex > 0 &&
        irIndex <= static_cast<int>(p->irBank->entries.size())) {
      entry = &p->irBank->entries[static_cast<size_t>(irIndex - 1)];
    }
    if (entry && !entry->h.empty()) {
      p->engine.setCabIr(entry->h.data(), static_cast<int>(entry->h.size()));
    } else {
      p->engine.setCabIr(nullptr, 0);
    }
  }

  const int namIndex =
      static_cast<int>(std::lround(p->shared.get(pedal::P_NAM_MODEL)));
  if (namIndex != p->lastNamIndex) {
    p->lastNamIndex = namIndex;
    std::string namPath;
    if (namIndex > 0 &&
        namIndex <= static_cast<int>(p->namNames.size())) {
      namPath = pedal::namDir() + "/" +
                p->namNames[static_cast<size_t>(namIndex - 1)];
    }
    p->engine.setNamModel(namPath);
  }

  if (p->audioOutL != p->audioInL) {
    std::memcpy(p->audioOutL, p->audioInL, frames * sizeof(float));
  }
  if (p->audioOutR != p->audioInR) {
    std::memcpy(p->audioOutR, p->audioInR, frames * sizeof(float));
  }

  p->engine.process(p->audioOutL, p->audioOutR, static_cast<int>(frames),
                    p->meters);
  p->shared.tunerHz.store(p->engine.tuner().frequency(),
                          std::memory_order_relaxed);
  p->shared.tunerCents.store(p->engine.tuner().cents(),
                             std::memory_order_relaxed);
  p->shared.tunerConfidence.store(p->engine.tuner().confidence(),
                                  std::memory_order_relaxed);
  p->shared.tunerNote.store(p->engine.tuner().midiNote(),
                            std::memory_order_relaxed);

  if (p->inPeakL) *p->inPeakL = p->meters.inPeakLDb;
  if (p->inPeakR) *p->inPeakR = p->meters.inPeakRDb;
  if (p->outPeakL) *p->outPeakL = p->meters.outPeakLDb;
  if (p->outPeakR) *p->outPeakR = p->meters.outPeakRDb;
  if (p->inPeak) *p->inPeak = p->meters.inPeakDb;
  if (p->outPeak) *p->outPeak = p->meters.outPeakDb;
  for (int i = 0; i < pedal::kSlotCount; ++i) {
    if (p->slotGr[i]) *p->slotGr[i] = p->meters.slotGrDb[i];
  }
  if (p->latency) {
    *p->latency = static_cast<float>(p->engine.latencySamples());
  }

  p->shared.inPeakDb.store(p->meters.inPeakDb, std::memory_order_relaxed);
  p->shared.outPeakDb.store(p->meters.outPeakDb, std::memory_order_relaxed);
  p->shared.inPeakLDb.store(p->meters.inPeakLDb, std::memory_order_relaxed);
  p->shared.inPeakRDb.store(p->meters.inPeakRDb, std::memory_order_relaxed);
  p->shared.outPeakLDb.store(p->meters.outPeakLDb, std::memory_order_relaxed);
  p->shared.outPeakRDb.store(p->meters.outPeakRDb, std::memory_order_relaxed);
  for (int i = 0; i < pedal::kSlotCount; ++i) {
    p->shared.slotGrDb[i].store(p->meters.slotGrDb[i],
                                std::memory_order_relaxed);
  }
}

LV2_State_Status stateSave(LV2_Handle handle,
                           LV2_State_Store_Function store,
                           LV2_State_Handle state, uint32_t,
                           const LV2_Feature* const*) {
  auto* p = static_cast<Plugin*>(handle);
  if (!p->chunkType) return LV2_STATE_ERR_NO_FEATURE;
  const std::vector<uint8_t> blob = pedal::serializeState(p->shared, 1.0);
  store(state, p->stateKey, blob.data(), blob.size(), p->chunkType,
        LV2_STATE_IS_POD | LV2_STATE_IS_PORTABLE);
  return LV2_STATE_SUCCESS;
}

LV2_State_Status stateRestore(LV2_Handle handle,
                              LV2_State_Retrieve_Function retrieve,
                              LV2_State_Handle state, uint32_t,
                              const LV2_Feature* const*) {
  auto* p = static_cast<Plugin*>(handle);
  uint32_t type = 0, flags = 0;
  size_t size = 0;
  const void* data = retrieve(state, p->stateKey, &size, &type, &flags);
  if (!data && p->legacyStateKey) {
    data = retrieve(state, p->legacyStateKey, &size, &type, &flags);
  }
  if (!data) return LV2_STATE_SUCCESS;
  if (type != p->chunkType) return LV2_STATE_ERR_BAD_TYPE;
  if (!pedal::deserializeState(data, size, p->shared)) {
    return LV2_STATE_ERR_BAD_TYPE;
  }
  // Force the next run() to push the restored values through the engine even
  // if the host has not refreshed the control ports yet.
  p->portsSeen = false;
  return LV2_STATE_SUCCESS;
}

const LV2_State_Interface kStateInterface = {stateSave, stateRestore};

const void* extensionData(const char* uri) {
  if (uri && !std::strcmp(uri, LV2_STATE__interface)) {
    return &kStateInterface;
  }
  return nullptr;
}

void cleanup(LV2_Handle handle) { delete static_cast<Plugin*>(handle); }

const LV2_Descriptor kDescriptor = {kPluginUri,      instantiate, connectPort,
                                    activate,        run,         deactivate,
                                    cleanup,         extensionData};

} // namespace

LV2_SYMBOL_EXPORT const LV2_Descriptor* lv2_descriptor(uint32_t index) {
  return index == 0 ? &kDescriptor : nullptr;
}
