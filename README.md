# Teensy DJ FX Box

A hardware effects unit for DJing, built on a **Teensy 4.1** with the
**PJRC Audio Shield**. Line audio comes in, runs through a filter, echo, and
reverb, and goes back out (all driven by four knobs and two buttons on the
front panel).

## What it does

Signal path: **line in → filter → echo → reverb → volume → line out.**

- **Filter** (one knob) center is bypass. Turn left to sweep a low-pass
  (cut the highs), right to sweep a high-pass (cut the lows). Small dead-zone
  in the middle so "flat" is easy to find.
- **Echo** a ~375 ms delay with feedback so the repeats ring out.
  On/off button + amount knob.
- **Reverb** a room reverb. On/off button + amount knob.
- **Volume** master fader with a squared taper so it feels natural by ear.

## How the code was built — the PJRC Audio Design Tool

I didn't wire the audio graph by hand. The Teensy Audio Library has a
browser tool — the **[Audio System Design Tool](https://www.pjrc.com/teensy/gui/)** —
where you drag audio blocks (input, mixers, filter, delay, reverb, output)
onto a canvas and connect them visually. Hitting **Export** spits out the
boilerplate: all the `AudioMixer4`, `AudioConnection patchCord…` lines.
That's the whole block between `// GUItool: begin` and `// GUItool: end`
in the sketch.

Everything after that block is the hand-written part: reading the knobs and
buttons, mapping the filter knob to a logarithmic frequency sweep, toggling
echo/reverb on and off, and smoothing the controls.

So the workflow was: **design the audio chain visually → export the wiring
code → write the control logic around it.**

<img width="934" height="483" alt="Bildschirmfoto 2026-09-09 um 18 27 19" src="https://github.com/user-attachments/assets/e9e120f8-e783-424a-a6d2-fc5668be987f" />


## Hardware

- Teensy 4.1 + PJRC Audio Shield (SGTL5000 codec)
- 4 potentiometers (filter, echo, reverb, volume) → A0–A3
- 2 push-buttons (echo, reverb) → pins 2 and 4

## Files

- `dj_fx.ino` the full sketch (audio graph + control code)

## Building it

Open `dj_fx.ino` in the Arduino IDE with
[Teensyduino](https://www.pjrc.com/teensy/td_download.html) installed,
select your Teensy 4.1, and upload.
