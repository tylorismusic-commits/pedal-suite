# Changelog

## 0.6.0-alpha — 2026-11-18

- Added **NAM Amp**, a Neural Amp Modeler player with Gain, Bass, Mid,
  Treble and Level.
- Load `.nam` models with the LOAD AMP button. Models are resampled to the
  host rate and run through a DC blocker.
- **Hard limit of two live NAM blocks per session.** Extra NAM slots pass
  dry and show a red "2 AMP LIMIT" warning, so eight NAM blocks cannot crash
  or exhaust memory.
- NAM adds a 64-sample block latency, reported to the host and summed across
  active NAM slots.
- Model loading is capped at 32 MB per file; missing, malformed or
  unsupported models are rejected and pass dry safely.
- Fixed the round footswitch hit area: clicks now toggle bypass instead of
  falling through to the pedal picker.
- Fixed the LV2 manifest's missing `doap:` prefix that strict hosts rejected.
- The LV2 editor lists IR and NAM files directly from disk, so the pickers
  work in hosts that run the UI in a separate process.
- NAM models are settled on load, and a DC blocker removes the constant
  offset they produce with silence.

## 0.5.2-alpha — 2026-11-18

- Tidied the About page: the UP/DOWN buttons no longer overlap the footer
  text, the SOURCE column is now a clearer TYPE column, and the repeated
  "Pedal Suite exclusive"/"Amp Suite" source labels are gone.
- Browser option subtitles now show the pedal type instead of the source.

## 0.5.1-alpha — 2026-11-18

- The Cab pedal now has a prominent **LOAD IR** button below its name.
  It turns amber, outlines in accent and shows a green dot when an IR is
  loaded.
- Pedal browser tabs are centered, and partial option rows are centered
  too (the last row no longer hangs to the left).
- The current pedal highlight is a subtle module-tinted card with an accent
  border instead of a solid accent fill, so the name stays readable.

## 0.5.0-alpha — 2026-11-18

- Added **Cab**, an impulse-response cabinet pedal with Low Cut, High Cut,
  Level and Mix. Load WAV IRs from the folder picker on the pedal card.
- IRs are resampled to the host rate, normalised at 1 kHz and convolved
  with a zero-latency 4096-tap FIR.
- Removed the redundant VERB browser tab; reverbs already live under TIME.
- The loaded IR list is shared with the engine so host and UI indices agree.

## 0.4.0-alpha — 2026-11-18

- Added Horizon Devices-style **Precision Drive** with Gate, Drive, Attack
  and Level. Attack tightens the pre-clip low end; the input gate reports GR.
- Added a classic band-pass **Wah** with Pedal, Q, Range and Mix. MIDI
  expression (CC 4 or 11) moves the first Wah in the chain.
- Added a **Laser** pitch/ring-mod pedal with Pitch, Ring, Sens, Pedal and
  Mix, including envelope-driven sweeps.
- New boards: Precision Metal, Laser Lead and Wah Rock.
- Modules are now prepared lazily per slot: idle and small boards use far
  less memory and start immediately, which clears the way for amp/cab/NAM
  models.
- Control updates are gated by a shared revision counter; modules are no
  longer reconfigured every audio block.
- Every pedal is now checked for audible output, and slot type changes are
  covered by a test.

## 0.3.4-alpha — 2026-11-18

- MIDI footswitch control in CLAP and LV2: notes 36-43 or 60-67 and
  CC 20-27 or 80-87 toggle pedals 1-8.
- LV2 now exposes all eight gain-reduction ports (slots 7-8 were missing).
- Fixed the LV2 plugin and UI URIs, which stopped the LV2 editor from
  opening in hosts; the editor now loads and draws in REAPER.
- Corrected the CLAP descriptor URL, description and feature list.
- User preset files now use the `.pedal-suite` extension; `.suite-rack`
  files from earlier alphas still load and migrate on save.

## 0.3.3-alpha — 2026-11-18

- Eight new curated board presets: Ambient Wash, Gojira Drop, Bass Growl,
  Funk Wah, Pitch Lead, Spring Surf, Lo-Fi Tape and Modern Metal.
- Factory boards now reset to empty slots and neutral input/output before
  loading, so every preset is absolute and repeatable.
- Added validation and audio tests for every factory board.

## 0.3.2-alpha — 2026-11-18

- Added a built-in monophonic tuner with a YIN pitch detector.
- The tuner shows note, octave, cents, frequency and confidence, and reads
  the dry input before the pedalboard.
- Open it from the TUNER button below the input meter.

## 0.3.1-alpha — 2026-11-18

- QA pass across all 37 pedals with measurement-based tests.
- Pitch Drop modes now use the full knob and show the effective pitch.
- FET Comp soft knee; gate envelope follower; calibrated reverb decay.
- Delay timing is sample-accurate on the first echo.
- Real gain-reduction meters for the compressor and gate pedals.
- Bypass tails for the delays and reverbs.
- Typed kHz values, FET release and EQ mid gain now parse/format correctly.

## 0.3.0-alpha — 2026-11-18

- Completed the 37-pedal catalog.
- Added FET Comp and Opto Comp.
- Added Envelope Filter, Noise Gate and Tight Gate.
- Added 3-Band EQ, Bass EQ, Lo-Fi and Tape Sat.
- Added Bass Chorus, Vibrato and Octave.
- Added Tape Echo, Digital Delay, Ping-Pong, Plate Reverb and Room Reverb.
- New factory presets for every pedal; dynamics, gate and tone tests added.

## 0.2.0-alpha — 2026-11-18

- Added Analog Delay and Spring Reverb, bringing the board to 21 pedal
  types — the public-alpha pedal target.
- Analog Delay uses the hearth-dsp BBD-voiced delay with Time, Feedback,
  Tone and Mix.
- Spring Reverb uses the hearth-dsp spring tank with Decay, Tone and Mix.
- Factory presets for both, plus delay/reverb tail tests.

## 0.1.6-alpha — 2026-11-18

- Added Chorus, Flanger, Phaser and Tremolo.
- Restored the Pitch Drop Tone control and added a 3+2 knob layout for
  five-control pedals so the footswitch stays clear.
- Added factory presets for the modulation pedals and a pitch preview.

## 0.1.5-alpha — 2026-11-18

- Added Pitch Drop with Whammy Up, Whammy Down, Drop, Harmony and Detune
  modes on a Rubber Band Live low-latency engine.
- Added Retune with semitone, cent and low-cut controls for step tuning.
- Both pitch pedals report their latency to the host.
- Parameter text entry understands semitones, cents and Hz for the new
  controls, and the semitone knob snaps to 25 steps.
- Save/rename text fields ignore duplicate key events within 60 ms.

## 0.1.4-alpha — 2026-11-18

- Added Tone Bender, Bass Drive and Bass Fuzz.
- Bass pedals keep a clean low band while blending the driven high band.
- Added factory presets for all three.
- P3 drive/fuzz family is now complete.

## 0.1.3-alpha — 2026-11-18

- Added the first drive/fuzz family: Treble Boost, Overdrive, Blues Drive,
  Distortion, Rat, Big Fuzz and Fuzz Face.
- Each pedal has its own faceplate color, 3–4 controls and footswitch.
- Added factory pedal presets for the new family.
- Added a full pedal-module finite sweep.
- Fixed Fuzz Face bias so the ADAA clipper advances once per sample.

## 0.1.2-alpha — 2026-11-18

- Matched the Amp Suite pedal card shape: 236×380 vertical stompboxes with
  gradient faceplates, stage-size knobs and accurate 808 green / boost gold.
- Replaced the bypass bar with a circular footswitch, metal cap and LED.
- Moved MIX and GAIN below each pedal, with MIX under the pedal body.
- Added patch cables and side jacks between pedals.
- Added page switching while dragging for page-1 ↔ page-2 reordering.
- Page 2 carries its own pedals and the same card layout.

## 0.1.1-alpha — 2026-11-18

- Link pedal-dsp into the LV2 module so 808 Drive resolves at load time.
- Rebuild the release bundle.

## 0.1.0-alpha — 2026-11-18

- Initial Pedal Suite container: eight slots across two pages of four.
- Boost pedal with gain, tone and level.
- 808 Drive pedal: retuned Amp Suite TS9/808 with drive, tone and level.
- Browser, page switching, footswitch bypass, drag reorder.
- Factory boards, user board/pedal presets, typed values, About.
- CLAP and LV2 builds, state recall, automation naming and latency plumbing.

