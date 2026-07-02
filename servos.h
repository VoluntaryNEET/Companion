#pragma once
#include <ESP32Servo.h>
#include "globals.h"

// ─── PIN DEFINITIONS ───────────────────────────────────────────────────────
#define SERVO_LEFT_PIN  22
#define SERVO_RIGHT_PIN 15

// ─── SERVO POSITIONS ───────────────────────────────────────────────────────
#define SERVO_UP      170   // arm raised
#define SERVO_MID      90   // arm horizontal
#define SERVO_DOWN     10   // arm drooped
#define SERVO_IDLE_HI  95   // idle sway high
#define SERVO_IDLE_LO  85   // idle sway low — barely moves

// ─── STATE ─────────────────────────────────────────────────────────────────
static Servo _servoLeft;
static Servo _servoRight;

// Arm animation state machine
enum ArmState {
    ARM_IDLE,
    ARM_HAPPY,
    ARM_ANGRY,
    ARM_STARTLED,
    ARM_TIRED
};

static ArmState      _armState        = ARM_IDLE;
static unsigned long _armStepTimer    = 0;
static int           _armStep         = 0;
static bool          _armStateDone    = false;

// ─── INIT ───────────────────────────────────────────────────────────────────
void servos_init() {
    _servoLeft.attach(SERVO_LEFT_PIN);
    _servoRight.attach(SERVO_RIGHT_PIN);
    _servoLeft.write(SERVO_MID);
    _servoRight.write(SERVO_MID);
    Serial.println(F("Servos initialized"));
}

// ─── INTERNAL: set both arms ───────────────────────────────────────────────
static void _setArms(int leftPos, int rightPos) {
    _servoLeft.write(leftPos);
    _servoRight.write(rightPos);
}

// ─── PUBLIC: trigger a reaction ────────────────────────────────────────────
void servos_setReaction(ArmState state) {
    _armState     = state;
    _armStep      = 0;
    _armStateDone = false;
    _armStepTimer = millis();
}

// ─── PUBLIC: call every loop ───────────────────────────────────────────────
void servos_update() {
    unsigned long now = millis();

    switch (_armState) {

        // ── IDLE: slow gentle sway, barely moving ─────────────────────────
        case ARM_IDLE:
            // Sway left and right arm alternately every 1200ms
            if (now - _armStepTimer >= 1200) {
                _armStepTimer = now;
                _armStep      = !_armStep;
                if (_armStep) {
                    _setArms(SERVO_IDLE_HI, SERVO_IDLE_LO);
                } else {
                    _setArms(SERVO_IDLE_LO, SERVO_IDLE_HI);
                }
            }
            break;

        // ── HAPPY: arms alternate up and down cheerfully ──────────────────
        // Left up → right up left down → left up right down → repeat
        case ARM_HAPPY:
            if (now - _armStepTimer >= 300) {
                _armStepTimer = now;
                switch (_armStep) {
                    case 0: _setArms(SERVO_UP,   SERVO_DOWN); break;
                    case 1: _setArms(SERVO_DOWN,  SERVO_UP);  break;
                    case 2: _setArms(SERVO_UP,   SERVO_DOWN); break;
                    case 3: _setArms(SERVO_DOWN,  SERVO_UP);  break;
                    case 4: _setArms(SERVO_MID,   SERVO_MID);
                            _armState = ARM_IDLE; break;
                }
                _armStep++;
            }
            break;

        // ── ANGRY: both slam up then slam down together ────────────────────
        case ARM_ANGRY:
            if (now - _armStepTimer >= 400) {
                _armStepTimer = now;
                switch (_armStep) {
                    case 0: _setArms(SERVO_UP,   SERVO_UP);   break;
                    case 1: _setArms(SERVO_DOWN,  SERVO_DOWN); break;
                    case 2: _setArms(SERVO_UP,   SERVO_UP);   break;
                    case 3: _setArms(SERVO_DOWN,  SERVO_DOWN);
                            _armState = ARM_IDLE; break;
                }
                _armStep++;
            }
            break;

        // ── STARTLED: both shoot up, hold 2 seconds, then come down ───────
        case ARM_STARTLED:
            switch (_armStep) {
                case 0:
                    _setArms(SERVO_UP, SERVO_UP);
                    _armStep++;
                    _armStepTimer = now;
                    break;
                case 1:
                    if (now - _armStepTimer >= 2000) {
                        _setArms(SERVO_MID, SERVO_MID);
                        _armState = ARM_IDLE;
                    }
                    break;
            }
            break;

        // ── TIRED: arms droop down slowly over 1 second ───────────────────
        case ARM_TIRED: {
            if (_armStep == 0) {
                _armStep++;
                _armStepTimer = now;
            }
            unsigned long elapsed = now - _armStepTimer;
            if (elapsed <= 1000) {
                // Interpolate from MID to DOWN over 1000ms
                float t   = (float)elapsed / 1000.0f;
                int   pos = SERVO_MID + (int)(t * (SERVO_DOWN - SERVO_MID));
                _setArms(pos, pos);
            } else {
                _setArms(SERVO_DOWN, SERVO_DOWN);
                _armState = ARM_IDLE;
            }
            break;
        }
    }
}
