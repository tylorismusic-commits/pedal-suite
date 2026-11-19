# Changelog

## 0.8.9-alpha — 2026-11-19

- LV2 now also keeps a plugin-side performance copy, so setlist and bank MIDI
  works with the editor closed.
- Added `editor_open`, performance-position input/output ports and a background
  setlist reload when the LV2 editor closes.
- CLAP and LV2 share the same performance engine and synchronize bank/entry
  position with the editor.

## 0.8.8-alpha — 2026-11-19

- CLAP now keeps its own setlist/bank performance copy, so MIDI setlist and
  bank control works with the editor closed.
  - Setlists and user presets reload when the editor closes.
  - Performance bank/entry position is published to the editor and adopted
    when it reopens.
- LV2 keeps the existing editor-driven performance control for now.

## 0.8.7-alpha — 2026-11-19

- **ADD CURRENT** now snapshots the live board instead of requiring a loaded
  preset. It works on the initial/Init board and captures unsaved edits.
- If no setlist exists, ADD CURRENT creates one automatically.
- Setlist snapshot files store an explicit value count so future parameter
  additions remain readable.

## 0.8.6-alpha — 2026-11-19

- Moved the setlist manager hint above the action buttons so it no longer
  collides with ADD CURRENT / REMOVE.

## 0.8.5-alpha — 2026-11-19

- MIDI control for setlists and banks:
  - CC 28 / 29 = previous / next setlist entry.
  - CC 30 / 31 = previous / next preset bank.
  - Program Change selects an entry in the active setlist, or a preset within
    the current bank when no setlist is active.
- CLAP consumes the events directly; LV2 transfers them to the UI through
  sequence and value ports.
- Fixed expression mode swallowing unrelated CC messages.
- MIDI panel now documents the performance mapping.

## 0.8.4-alpha — 2026-11-19

- Board preset banks: the preset browser now filters by **ALL**, each factory
  category and **USER**.
- Setlist manager: create, rename and delete setlists; add the current board;
  reorder entries; step with PREVIOUS/NEXT; and load any entry. Setlists are
  stored in `~/.config/PedalSuite/setlists`.
- Preset list header now carries the BANK and SETLIST controls.
- Setlist and bank state is kept out of the host parameter block; setlists are
  performance files, not part of a board preset.

## 0.8.3-alpha — 2026-11-19

- Restored the pedal cards to near their original size while keeping them
  clear of the input and output meters.
- Made 2- and 4-control pedal knob columns symmetrical inside each card.
- Moved TUNER and MIDI above the pedals into the page-title row.

## 0.8.2-alpha — 2026-11-19

- Narrowed the pedal cards and moved them left so the fourth pedal no longer
  hides behind the output meter.
- Moved TUNER and MIDI into a stacked utility strip on the right side, below
  the header buttons. The footer is back to just INPUT/OUTPUT.
- Updated drag/reorder hit-testing for the new board geometry.

## 0.8.1-alpha — 2026-11-19

- Reverted the board rails, screws and colour-coded patch cables; the original
  board look is back.
- Moved the MIDI button above the footer so it no longer crowds the input knob.

## 0.8.0-alpha — 2026-11-19

- Added an Auto-Wah pedal with a wider envelope sweep, independent Q and range,
  and three module presets.
- **MIDI learn:** the new MIDI panel learns a note or CC for each of the eight
  pedals. Learned mappings override the factory note/CC ranges and are saved
  with the plugin state.
- **Expression mapping:** learn a CC and route it to any control on any loaded
  pedal. The old CC 4/11 first-Wah behaviour remains when expression is off.
- LV2 learns work across the UI/plugin boundary through dedicated learn and
  learn-state control ports.
- Added twelve board presets: Classic Rock, Bluesbreaker, Country Twang,
  Shoegaze, Funk Machine, Bass DI, Synth Bass, Prog Lead, Worship, Garage,
  Doom and Acoustic DI.
- Updated the README, preset count and roadmap.

## 0.7.0-alpha — 2026-11-18

Big interaction and performance audit.

- Cab now uses **FFT partitioned convolution** (256-sample blocks, 4096 taps).
  A cab dropped from ~85% CPU to ~1.2%; two cabs are ~2%.
- Cabs are now limited to **two live instances**, like NAM. Extra cabs pass
  dry and show **2 CAB LIMIT**. Eight cabs no longer stack gain or burn the
  CPU.
- A Cab with no IR passes dry through its EQ instead of going silent.
- An IR selected before the Cab is loaded is retained through lazy prepare.
- Selecting an IR updates the reported latency (Cab adds 256 samples).
- The CLAP wrapper now queues a latency change and notifies the host on the
  main thread when NAM/Cab latency changes mid-session.
- Clicking the gap between the last preset row and the footer no longer
  selects a hidden preset.

Audit coverage added

- All 1,764 ordered pedal pairs stay finite and bounded.
- 200 random eight-slot boards stay finite with sane latency.
- State round-trips every module plus IR/NAM resource indices.
- Browser page tests for UTIL, AMP and PRESETS.
- Malformed and oversized NAM models are rejected safely.
- NAM gate regression test.

## 0.6.3-alpha — 2026-11-18

- NAM Amp now has a **Gate** control. The gate starts closed on load, mutes
  the model's captured noise floor, and stops downstream drives from
  amplifying amp hiss. The startup thump after a NAM block is gone.
- The board preset list has proper breathing room above the footer; the
  last row is no longer crammed against the buttons.

## 0.6.2-alpha — 2026-11-18

- Restored the **NAM Amp** pedal and its two-instance safety limit.
- Added an **AMP** browser page so NAM is easy to find.
- Added the **UTIL** browser page with the EQ, Lo-Fi, Tape Sat and Cab pedals.
- Moved **Cab** from DRIVE to UTIL.

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

