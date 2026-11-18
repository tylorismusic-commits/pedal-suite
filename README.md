# Pedal Suite

**Status: 0.1.5-alpha — drive/fuzz family plus pitch.**

A virtual guitar and bass pedalboard in the Suite visual language: eight pedal
slots across two pages of four, with a shared preset browser, typed values,
drag reordering and host automation.

This is the first P1 slice. It currently ships **Boost** and **808 Drive**
(the retuned Amp Suite TS9/808). The rest of the pedal catalog, pitch engine
and full board visuals land in later milestones.

![Pedal Suite — page 1](docs/pedal-board.png)

![Pedal Suite — page 2](docs/pedal-board-page2.png)

## Current slice

- 1040×720 panel with power, preset bar, page buttons and utility pills.
- Four wide pedal slots per page; **PAGE 1** = pedals 1–4, **PAGE 2** = 5–8.
- Amp Suite-style pedal cards: gradient faceplates, footswitch, LED, preset button.
- MIX and GAIN sit below each pedal body.
- Drag a pedal name or card to reorder, including page-to-page moves.
- Browser with **ALL / DRIVE / MOD / DYN / TIME / VERB / PRESETS**.
- **Boost** — Gain, Tone, Level.
- **808 Drive** — Drive, Tone, Level; retuned TS9/808 from Amp Suite.
- **Treble Boost** — Boost, Range, Level.
- **Overdrive** — Drive, Tone, Level, Mix.
- **Blues Drive** — Drive, Tone, Level.
- **Distortion** — Dist, Filter, Level.
- **Rat** — Dist, Filter, Level.
- **Big Fuzz** — Sustain, Tone, Level.
- **Fuzz Face** — Fuzz, Bias, Volume.
- **Tone Bender** — Attack, Tone, Level.
- **Bass Drive** — Drive, Blend, Tone, Level.
- **Bass Fuzz** — Fuzz, Blend, Tone, Level.
- **Pitch Drop** — Gojira-style Whammy Up, Whammy Down, Drop, Harmony
  and Detune modes with Pitch, Mode, Glide, Tone and Mix.
- **Retune** — Low-latency semitone/cent tuning with Semitones, Cents,
  Low Cut and Mix.
- Factory board presets: Clean Platform, 808 Stack, Lead Lift.
- User board and pedal presets: save, rename and delete.
- Double-click resets a knob; right-click types an exact value.
- CLAP and LV2 builds share state, automation, latency and GR plumbing.

## Repository layout

```
pedal-suite
├── src/                    plugin container, UI, engine and metadata
├── third_party/hearth-dsp  shared DSP primitives
├── third_party/pedal-dsp   pedal-specific DSP
├── tools/gen_ttl.cpp       LV2 metadata generator
└── test/                   DSP, state and panel tests
```

## Build

```sh
git submodule update --init --recursive
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
```

## Install

CLAP:

```sh
cp build/clap/PedalSuite.clap ~/.clap/
cp -r build/clap/PedalSuite.resources ~/.clap/
```

LV2:

```sh
cp -r build/lv2/pedal-suite.lv2 ~/.lv2/
```

## Roadmap

1. Pedalboard visuals: LEDs, footswitch caps, patch cables.
2. Pitch Drop and low-latency Retune.
3. Drive/fuzz family, then modulation, time, reverb, dynamics.
4. Full preset catalog and public alpha.

GPL-3.0-or-later.
