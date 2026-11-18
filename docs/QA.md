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
| All 41 pedals | 48/96 kHz, controls at 0 and 1 | finite, no NaNs |
| All 41 pedals | default settings | audible output |
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

## Cab / IR

The Cab pedal uses zero-latency direct FIR convolution (up to 4096 taps) with
the impulse owned by the plugin's IR bank. WAVs are averaged to mono,
resampled to the host rate, normalised at 1 kHz and truncated to 4096 taps.
The picker lists the same sorted files as the engine bank, so parameter
indices and the UI agree. A unit-impulse test confirms the Cab is transparent
within 10%. The redundant VERB browser tab was removed because all reverbs
are already in TIME.

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
