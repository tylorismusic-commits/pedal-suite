# Changelog

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

