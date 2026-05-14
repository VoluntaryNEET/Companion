#pragma once
#include <Adafruit_SH110X.h>
#include <FluxGarage_RoboEyes.h>

#include "sound_PLAY.h"

// ─── ANIMATION TUNING ──────────────────────────────────────────────────────
#define WAKE_S0_DURATION  600   // dark screen, flat line eyes appear
#define WAKE_S1_DURATION  400   // flat line eyes blink once
#define WAKE_S2_DURATION  600   // eyes peek open slightly
#define WAKE_S3_DURATION  900   // mouth opens vertically, eyes roll up
#define WAKE_S4_DURATION  700   // mouth shrinks, eyes come back down
#define WAKE_S5_DURATION  700   // two blinks then done

// Mouth position — centered horizontally, below the eyes
#define MOUTH_X           64    // horizontal center of 128px screen
#define MOUTH_Y           50    // vertical position below eyes
#define MOUTH_MAX_RX      5     // narrow horizontal radius — stays slim
#define MOUTH_MAX_RY      12    // tall vertical radius — stretches for yawn

// ─── STATE ─────────────────────────────────────────────────────────────────
static bool          _wakeAnimDone   = false;
static int           _wakeStage      = 0;
static unsigned long _wakeStageStart = 0;
static bool          _wakeStarted    = false;

// ─── INTERNAL: draw vertical yawn ellipse ──────────────────────────────────
static void _drawMouth(Adafruit_SH1106G &display, int rx, int ry) {
    if (rx <= 0 || ry <= 0) return;
    display.fillEllipse(MOUTH_X, MOUTH_Y, rx, ry, SH110X_WHITE);
    display.display();
}

// ─── PUBLIC API ────────────────────────────────────────────────────────────
// Returns true while animation playing, false when complete
// Call every loop() — owns the loop until it returns false
bool wakeup_update(Adafruit_SH1106G &display, RoboEyes<Adafruit_SH1106G> &eyes) {
    if (_wakeAnimDone) return false;

    // First call init — screen starts dark, eyes closed flat
    if (!_wakeStarted) {
        _wakeStarted    = true;
        _wakeStageStart = millis();
        eyes.close();
        eyes.setMood(DEFAULT);
        eyes.setPosition(DEFAULT);
        eyes.setAutoblinker(OFF, 0, 0);
        eyes.setIdleMode(OFF, 0, 0);
        sound_play(SOUND_WAKEUP);
    }

    unsigned long now     = millis();
    unsigned long elapsed = now - _wakeStageStart;

    eyes.update(); // always runs — mouth drawn on top after

    switch (_wakeStage) {

        // ── Stage 0: screen dark, flat closed eyes just sitting there ─────
        case 0:
            if (elapsed >= WAKE_S0_DURATION) {
                _wakeStage      = 1;
                _wakeStageStart = now;
            }
            break;

        // ── Stage 1: flat line eyes do a single slow blink ────────────────
        case 1:
            eyes.blink();
            if (elapsed >= WAKE_S1_DURATION) {
                _wakeStage      = 2;
                _wakeStageStart = now;
            }
            break;

        // ── Stage 2: eyes peek open — very low height, barely awake ───────
        case 2:
            eyes.open();
            eyes.setHeight(5, 5); // barely open
            eyes.setPosition(DEFAULT);
            if (elapsed >= WAKE_S2_DURATION) {
                _wakeStage      = 3;
                _wakeStageStart = now;
            }
            break;

        // ── Stage 3: mouth stretches open vertically, eyes roll up ─────────
        case 3: {
            float progress = (float)elapsed / WAKE_S3_DURATION;
            float mouthP   = min(progress * 2.0f, 1.0f); // full open by halfway
            int rx = (int)(MOUTH_MAX_RX * mouthP);
            int ry = (int)(MOUTH_MAX_RY * mouthP);
            _drawMouth(display, rx, ry);
            eyes.setHeight(10, 10);  // open a bit more for the ‾0‾ look
            eyes.setPosition(N);     // eyes roll upward
            if (elapsed >= WAKE_S3_DURATION) {
                _wakeStage      = 4;
                _wakeStageStart = now;
            }
            break;
        }

        // ── Stage 4: mouth shrinks back to nothing, eyes drift back down ───
        case 4: {
            float progress = (float)elapsed / WAKE_S4_DURATION;
            float inv      = 1.0f - progress;
            int rx = (int)(MOUTH_MAX_RX * inv);
            int ry = (int)(MOUTH_MAX_RY * inv);
            _drawMouth(display, rx, ry);
            eyes.setPosition(DEFAULT);
            eyes.setHeight(36, 36); // restore normal eye height
            if (elapsed >= WAKE_S4_DURATION) {
                _wakeStage      = 5;
                _wakeStageStart = now;
            }
            break;
        }

        // ── Stage 5: two blinks then hand off to idle ─────────────────────
        case 5:
            // Fire two blinks spaced apart within the stage window
            if (elapsed == 0 || (elapsed >= 250 && elapsed < 260)) {
                eyes.blink();
            }
            if (elapsed >= WAKE_S5_DURATION) {
                _wakeAnimDone = true;
                eyes.setHeight(36, 36);
                eyes.setPosition(DEFAULT);
                eyes.setMood(DEFAULT);
                eyes.setAutoblinker(ON, 3, 2);
                eyes.setIdleMode(ON, 2, 2);
            }
            break;
    }

    return !_wakeAnimDone;
}