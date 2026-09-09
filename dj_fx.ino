#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// wiring block

// ---------- what each mixer is ----------
// mixer1 = filter-mix  (in0 dry, in1 low-pass, in2 high-pass)
// mixer2 = echo-in     (in0 signal, in1 delay feedback) -> delay1
// mixer3 = echo-out    (in0 dry, in1 delayed/wet)
// mixer4 = reverb-mix  (in0 dry, in1 reverb/wet)
// mixer5 = master      (in0 signal * volume)

// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=101,303
AudioMixer4              mixer1;         //xy=242.85715866088867,370.04758071899414 -- filter-mix (dry / LP / HP)
AudioFilterStateVariable filter1;        //xy=243.85713958740234,232.4285888671875
AudioEffectDelay         delay1;         //xy=431.38095474243164,559.6666641235352
AudioMixer4              mixer3;         //xy=439.7618637084961,235.9523639678955 -- echo-out (dry + wet)
AudioMixer4              mixer2;         //xy=441.5237846374512,364.2380676269531 -- echo-in (dry + feedback -> delay)
AudioEffectFreeverb      freeverb1;      //xy=618.7619247436523,290.76189041137695
AudioMixer4              mixer5;         //xy=797.4285049438477,370.1904716491699 -- master volume
AudioMixer4              mixer4;         //xy=802.2380981445312,244.3809471130371 -- reverb-mix (dry + wet)
AudioOutputI2S           i2s2;           //xy=979.9999809265137,306.80952548980713
AudioConnection          patchCord1(i2s1, 0, filter1, 0);
AudioConnection          patchCord2(i2s1, 0, mixer1, 0);
AudioConnection          patchCord3(mixer1, 0, mixer2, 0);
AudioConnection          patchCord4(mixer1, 0, mixer3, 0);
AudioConnection          patchCord5(filter1, 0, mixer1, 1);
AudioConnection          patchCord6(filter1, 2, mixer1, 2);
AudioConnection          patchCord7(delay1, 0, mixer3, 1);
AudioConnection          patchCord8(delay1, 0, mixer2, 1);
AudioConnection          patchCord9(mixer3, freeverb1);
AudioConnection          patchCord10(mixer3, 0, mixer4, 0);
AudioConnection          patchCord11(mixer2, delay1);
AudioConnection          patchCord12(freeverb1, 0, mixer4, 1);
AudioConnection          patchCord13(mixer5, 0, i2s2, 0);
AudioConnection          patchCord14(mixer4, 0, mixer5, 0);
AudioControlSGTL5000     sgtl5000_1;     //xy=884.6666641235352,517.7143115997314
// GUItool: end automatically generated code


// ---------- Front-panel controls ----------
const int PIN_FILTER = A0;   // filter knob
const int PIN_ECHO   = A1;   // echo amount knob
const int PIN_REVERB = A2;   // reverb amount knob
const int PIN_VOLUME = A3;   // master volume fader
const int PIN_BTN_ECHO   = 2; // echo on/off button
const int PIN_BTN_REVERB = 4; // reverb on/off button

// ---------- Tunable settings ----------
const float LP_MAX_HZ = 15000.0f, LP_MIN_HZ = 60.0f;   // low-pass sweep range
const float HP_MIN_HZ = 30.0f,    HP_MAX_HZ = 12000.0f; // high-pass sweep range
const float FILTER_Q  = 1.2f;     // filter resonance / bite
const float DEADZONE  = 0.06f;    // center detent where filter is off
const int   ECHO_TIME_MS  = 375;  // delay time in ms
const float ECHO_FEEDBACK = 0.38f;// how long echoes ring out
const float REVERB_ROOM = 0.7f, REVERB_DAMP = 0.5f;    // reverb character

// ---------- State ----------
bool echoOn = false, reverbOn = false;
bool lastBtnEcho = HIGH, lastBtnReverb = HIGH;
float sFilter = 0, sEcho = 0, sReverb = 0, sVolume = 0;
float smooth(float p, float n) { return p + 0.15f * (n - p); }

void setup() { // switches the audio chip on, picks the line input
AudioMemory(400);
analogReadResolution(12);

sgtl5000_1.enable();
sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);
sgtl5000_1.lineInLevel(5);
sgtl5000_1.lineOutLevel(13);
sgtl5000_1.volume(0.6);

filter1.resonance(FILTER_Q);
delay1.delay(0, ECHO_TIME_MS);
mixer2.gain(1, ECHO_FEEDBACK);   // feedback into the delay
freeverb1.roomsize(REVERB_ROOM);
freeverb1.damping(REVERB_DAMP);

pinMode(PIN_BTN_ECHO, INPUT_PULLUP);
pinMode(PIN_BTN_REVERB, INPUT_PULLUP);

setFilter(0.5f);   // start bypassed
setEcho(0.0f);     // start dry
setReverb(0.0f);   // start dry
}

void loop() { // reads the four knobs and smooths them

  // translate a knob position
  sFilter = smooth(sFilter, analogRead(PIN_FILTER) / 4095.0f);
  sEcho   = smooth(sEcho,   analogRead(PIN_ECHO)   / 4095.0f);
  sReverb = smooth(sReverb, analogRead(PIN_REVERB) / 4095.0f);
  sVolume = smooth(sVolume, analogRead(PIN_VOLUME) / 4095.0f);

setFilter(sFilter);
setVolume(sVolume);
setEcho(echoOn     ? sEcho   : 0.0f);
setReverb(reverbOn ? sReverb : 0.0f);

bool be = digitalRead(PIN_BTN_ECHO);
if (be == LOW && lastBtnEcho == HIGH) { echoOn = !echoOn; delay(20); }
  lastBtnEcho = be;
bool br = digitalRead(PIN_BTN_REVERB);
if (br == LOW && lastBtnReverb == HIGH) { reverbOn = !reverbOn; delay(20); }
  lastBtnReverb = br;
}

// FILTER: center = bypass, left = kill highs, right = kill lows
void setFilter(float k) {
float x = (k - 0.5f) * 2.0f;
float dry, lp = 0, hp = 0, hz;
if (fabs(x) < DEADZONE) {
    dry = 1;
} else if (x < 0) {                              // low-pass
float a = (-x - DEADZONE) / (1.0f - DEADZONE);
    hz = LP_MAX_HZ * powf(LP_MIN_HZ / LP_MAX_HZ, a);
filter1.frequency(hz);
    dry = 1 - a; lp = a;
} else {                                         // high-pass
float a = (x - DEADZONE) / (1.0f - DEADZONE);
    hz = HP_MIN_HZ * powf(HP_MAX_HZ / HP_MIN_HZ, a);
filter1.frequency(hz);
    dry = 1 - a; hp = a;
}
mixer1.gain(0, dry); mixer1.gain(1, lp); mixer1.gain(2, hp);
}


void setEcho(float wet)   { mixer3.gain(0, 1.0f); mixer3.gain(1, wet); }
void setReverb(float wet) { mixer4.gain(0, 1.0f); mixer4.gain(1, wet); }
void setVolume(float v)   { mixer5.gain(0, v * v); }
