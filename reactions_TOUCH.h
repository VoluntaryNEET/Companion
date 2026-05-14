#pragma once
#include <Arduino.h>

// ─── TOUCH SENSOR PIN DEFINITIONS ──────────────────────────────────────────
#define TOUCH_PIN_LEFT  23

// ─── TAP / HOLD TUNING ─────────────────────────────────────────────────────
#define TOUCH_DEBOUNCE_MS  50    // ms to confirm real press vs noise
#define TOUCH_HOLD_MS      150   // ms of continuous contact to count as hold vs tap

// ─── TOUCH RESULT TYPE ─────────────────────────────────────────────────────
enum TouchSide  { TOUCH_SIDE_LEFT, TOUCH_SIDE_RIGHT, TOUCH_SIDE_NONE };
enum TouchEvent { TOUCH_EVENT_NONE, TOUCH_EVENT_TAP, TOUCH_EVENT_HOLD };

struct TouchResult {
  TouchEvent event;
  TouchSide  side;
};

// ─── ISR STATE ─────────────────────────────────────────────────────────────
// volatile — compiler must never cache these, ISR and main loop both touch them
struct _TouchISRState {
  volatile bool          pressed;      // true = finger currently down
  volatile unsigned long pressStart;   // millis() when finger landed
  volatile bool          holdFired;    // true = hold event already dispatched
  volatile bool          eventPending; // true = ISR flagged a change for getEvent()
  volatile bool          risingEdge;   // true = just pressed, false = just released
};

static _TouchISRState _isr[1]; // index 0 = left, 1 = right

// ─── ISRs ──────────────────────────────────────────────────────────────────
// IRAM_ATTR keeps ISR in fast IRAM — required on ESP32 to avoid cache miss crashes
void IRAM_ATTR _isr_touchLeft() {
  _isr[0].risingEdge   = (digitalRead(TOUCH_PIN_LEFT) == HIGH);
  _isr[0].pressStart   = millis();
  _isr[0].eventPending = true;
}

// ─── INIT ───────────────────────────────────────────────────────────────────
void touch_init() {
  pinMode(TOUCH_PIN_LEFT,  INPUT);

  _isr[0] = { false, 0, false, false, false };

  // CHANGE fires on both press and release so we catch the full tap/hold cycle
  attachInterrupt(digitalPinToInterrupt(TOUCH_PIN_LEFT),  _isr_touchLeft,  CHANGE);
}
void touch_cancelActive() {
    _isr[0].pressed = false;
    _isr[0].eventPending = false;
}
// ─── DETECTION ─────────────────────────────────────────────────────────────
// Call from reactions_update() — processes pending ISR flags into tap/hold events
// All tap/hold logic lives here intentionally, not in the ISR
TouchResult touch_getEvent() {
  const TouchSide sides[1] = { TOUCH_SIDE_LEFT};
  unsigned long now = millis();

  // Process any pending interrupt flags
  for (int i = 0; i < 1; i++) {
    if (!_isr[i].eventPending) continue;
    _isr[i].eventPending = false; // clear flag immediately

    if (_isr[i].risingEdge) {
      // Finger just landed — start tracking, nothing to return yet
      _isr[i].pressed   = true;
      _isr[i].holdFired = false;
    } else {
      // Finger just lifted
      if (!_isr[i].pressed) continue;
      _isr[i].pressed = false;  // clear immediately

      unsigned long heldFor = now - _isr[i].pressStart;
      if (heldFor < TOUCH_DEBOUNCE_MS) continue;

      if (!_isr[i].holdFired && heldFor >= TOUCH_HOLD_MS) {
        _isr[i].holdFired = true;  // mark as fired so continuous loop can't also fire
        return { TOUCH_EVENT_HOLD, sides[i] };
      } else if (!_isr[i].holdFired) {
        _isr[i].holdFired = true;  // same here
        return { TOUCH_EVENT_TAP, sides[i] };
      }
    }
  }

  // Finger still down — keep firing hold every poll cycle past threshold
  for (int i = 0; i < 1; i++) {
      if (_isr[i].pressed && !_isr[i].holdFired) {
          if (now - _isr[i].pressStart >= TOUCH_HOLD_MS) {
              // Don't set holdFired here — let it keep refreshing while held
              return { TOUCH_EVENT_HOLD, sides[i] };
          }
      }
  }

  return { TOUCH_EVENT_NONE, TOUCH_SIDE_NONE };
}
inline bool touch_isPressed() {
    return _isr[0].pressed;
}
