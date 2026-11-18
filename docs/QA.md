# Pedal Suite — DSP QA and calibration

This pass measured each pedal family against expected reference behaviour and
fixed everything the measurements exposed. The automated equivalents live in
`test/test_dsp.cpp` (Pedal Suite) and `pedal-dsp/tests/test_modules.cpp`.

## Measured checks

| Area | Check | Result |
|---|---|---|
| Octave | 220 Hz input, sub only | 110.5 Hz |
| Octave | 220 Hz input, octave up only | 440.5 Hz |
| 3-Band EQ | Mid +15 dB at 800 Hz | 15.0 dB |
| 3-Band EQ | Low +15 dB shelf at 60 Hz | 13.7 dB (shelf asymptote) |
| Analog/Digital/Tape/Ping-Pong | 100 / 400 / 900 ms echo | 100.0 / 400.0 / 900.0 ms |
| Noise/Tight Gate | steady tone at threshold | no per-cycle chopping |
| FET Comp | 20 dB input increase | ≈12 dB output increase (ratio > 1) |
| Plate Reverb | Decay 2.05 s | 2.10 s at 300 Hz |
| Room Reverb | Decay 1.76 s | 1.70 s at 300 Hz |
| Spring Reverb | Decay 1.16 s | 1.10 s at 300 Hz |
| Precision Drive | 80 Hz, Attack 0 → 1 | −19.4 dB (tightened) |
| Precision Drive | Gate 90%, quiet input | 80 dB reduction |
| Wah | 700 Hz, Pedal 0 → 50% | +23.2 dB through the band-pass |
| Laser | Pitch +12, 220 Hz input | 440.0 Hz (55 ms reported latency) |
| Cab | unit IR, 220 Hz | transparent within 10% |
| IR bank | 16-bit WAV load | 1 entry, normalised |
| NAM | 220 Hz test model at 44.1/48/96 kHz | 220 Hz preserved |
| NAM | zero input with a model | DC blocked; ~0 after 1 s |
| NAM | eight blocks, 2 live | finite, third+ dry |
| All 42 pedals | 48/96 kHz, controls at 0 and 1 | finite, no NaNs |
| All 42 pedals | default settings | audible output |
| All values | display → parse round-trip | within one display step |

## Fixes made

- **Pitch Drop** now uses the full knob in every mode: Up 0…+24 st,
  Down 0…−24 st, Drop 0…−24 st in semitones, Harmony −17…+31 st,
  Detune 2…30 cents. The knob and typed entry are mode-aware.
- **Gate** detector now has a short envelope follower, so a steady note near
  the threshold is not chopped at every zero crossing.
- **Tight Gate** decay display matches its 1–200 ms DSP range.
- **FET Comp** uses a 6 dB soft knee.
- **Bass Chorus** low cut now spans 40–300 Hz as displayed.
- **3-Band EQ** mid gain now reaches the displayed ±15 dB.
- **Delay** time smoothers snap on the first configuration, so the first echo
  lands exactly at the displayed time.
- **Reverb** decay is calibrated per mode (Room/Plate/Spring) so the seconds
  readout matches the measured RT60.
- **Lo-Fi / De-Ess** kHz values parse correctly from typed input.
- **FET Release** is continuous (100 ms–1.2 s) instead of inheriting the
  stepped studio-compressor display.
- **Octave** up path is gated smoothly, removing a click on note release.

## New features completed

- Real **gain reduction** metering for FET Comp, Opto Comp, Noise Gate and
  Tight Gate. The values feed the UI GR bars, the CLAP gain-adjustment
  extension and the LV2 `slotN_gr` ports.
- **Bypass tails** for all delays and reverbs: a bypassed time pedal keeps
  ringing while its input is muted. Non-time pedals remain silent when
  bypassed.
- A full **extreme-settings sweep** at 48 and 96 kHz for every pedal.

## Signature pedals

The Precision Drive follows the documented Horizon Devices topology: an input
gate, a pre-clip high-pass driven by **Attack**, a 720 Hz mid bump, soft
clipping and a 9 kHz output low-pass. It was tuned against the measured
target response of a tight high-gain boost, not against the proprietary
Tone 3K or Amp Locker models (those cannot be loaded into the test suite).
At full Attack it removes 19 dB at 80 Hz while keeping the mid push, which is
the behaviour players use in front of high-gain amps, cabs and NAM profiles.

The Laser follows the Misha Mansoor X design: an expression-swept pitch
shifter with ring modulation and an envelope follower. Pedal sweeps ±12
semitones around the Pitch setting, Sens adds envelope movement, and Ring
blends in the metallic carrier. It reports its Rubber Band latency.

The Wah is a band-pass state-variable filter with a 1.8–3 octave sweep and
Q up to 12. MIDI CC 4 or 11 moves the first Wah in the chain.

## NAM amp

The official NeuralAmpModelerCore runs WaveNet, ConvNet, LSTM, Linear,
Sequential and Slimmable architectures. Models are loaded from
`~/.local/share/pedal-suite/nam`, validated against a 32 MB cap, and run at
their training rate with linear resampling at other host rates. The pedal
buffers 64 samples, reports that latency, and passes dry if a model is
missing. A DC blocker removes the offset models produce with silence, and
only two NAM blocks are ever live (tested with eight).

## NAM gate

The NAM pedal now gates the model output from the dry input. With a quiet
input the model's noise floor is muted, and with a downstream Drive or
Distortion the residual noise drops from ~-80 dB to essentially zero after
the first 50 ms. The gate starts closed so the model-load thump cannot reach
later gain blocks.

## Cab / IR

The Cab pedal uses 256-sample FFT partitioned convolution (4096 taps) with
overlap-add. A 4-cab IR bank measured 1.2% CPU for one cab, 2.0% for two and
1.7% for eight slots (only two live). The IR is owned by the plugin's bank;
an IR selected before lazy Cab preparation is retained. The picker lists the
same sorted files as the engine bank. A unit-impulse test confirms the Cab is
transparent within 10%. Cab adds 256 samples of reported latency (0 with no
IR) and is limited to two live instances; extra cabs pass dry.

## Interaction audit

- 1,764 ordered pedal pairs: finite, worst peak 2.61.
- 200 random eight-slot boards: finite, worst peak 13.1, sane latency.
- State round-trips for every module and both resource indices.
- Browser pages UTIL/AMP/PRESETS and the preset-gap click are covered.
- Malformed and oversized NAM models are rejected; the NAM gate is tested.

## NAM amp

The official NeuralAmpModelerCore runs WaveNet, ConvNet, LSTM, Linear,
Sequential and Slimmable architectures. Models are loaded from
`~/.local/share/pedal-suite/nam`, validated against a 32 MB cap, and run at
their training rate with linear resampling at other host rates. The pedal
buffers 64 samples so the model processes blocks, reports that latency, and
passes dry if a model is missing or fails. A DC blocker removes the constant
offset many models produce with silence, and **only two NAM blocks are ever
live**; the rest pass dry and show a limit warning. Eight NAM blocks were
tested at 48 kHz without dropouts or non-finite output.

## NAM / amp / cab preparation

- **Lazy module preparation:** only modules selected by a slot are prepared.
  A heavy eight-pedal board dropped from 84.6 MB RSS / 102 ms prepare to
  15.3 MB / 0.1 ms; empty boards are nearly free. This leaves room for large
  amp, cab and NAM models per slot.
- **Revision-gated controls:** the plugins only push controls to the engine
  when a parameter actually changes, instead of rebuilding every module
  every block.
- **Coverage:** every module is checked for audible output, and changing a
  slot from Off to Delay to Wah is tested so lazy preparation cannot silence
  a newly selected module.
- The state blob is versioned and size-checked; trailing bytes are tolerated
  so a future resource section (IR/NAM file paths) can be appended without
  breaking older parameter layouts.

## Known limitations

- Octave is monophonic, like the analogue pedals it models.
- Pitch Drop adds Rubber Band latency (reported to the host).
- Reverb decay is calibrated for the default tone; heavy tone settings damp
  the tail earlier, as a real tank would.
- Bypass tails are always on; there is no per-pedal trails switch yet.
