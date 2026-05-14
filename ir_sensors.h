#pragma once
#include <Arduino.h>
#include "motor_routines.h"
#include "sound_PLAY.h"
#include "globals.h"

// ─── PIN DEFINITIONS ───────────────────────────────────────────────────────
#define IR_DOWN_LEFT     36   // input-only. AKA Pin VP
#define IR_DOWN_RIGHT    39   // input-only AKA Pin VN
#define IR_DOWN_FRONT_L  35   // input-only
#define IR_DOWN_FRONT_R  4
#define IR_WALL_FRONT    5
#define IR_WALL_BACK     13

// ─── SENSOR LOGIC ──────────────────────────────────────────────────────────
// Down sensors: normally HIGH (surface detected), LOW = cliff
// Wall sensors: normally LOW (nothing detected), HIGH = wall

// ─── TUNING ────────────────────────────────────────────────────────────────
#define IR_AVOID_DURATION_MS  1000   // how long to run avoidance move
#define IR_COOLDOWN_MS        1500   // ms before IR can trigger again after avoidance

// ─── STATE ─────────────────────────────────────────────────────────────────
static bool          _irAvoiding    = false;
static unsigned long _irAvoidUntil  = 0;
static unsigned long _irLastTrigger = 0;

static int _irAvoidStage = 0; // 0 = spinning, 1 = reversing
static int _irAvoidDir   = 0; // stores which direction to spin

// ─── INIT ───────────────────────────────────────────────────────────────────
void ir_init() {
    pinMode(IR_DOWN_LEFT,    INPUT);
    pinMode(IR_DOWN_RIGHT,   INPUT);
    pinMode(IR_DOWN_FRONT_L, INPUT);
    pinMode(IR_DOWN_FRONT_R, INPUT);
    pinMode(IR_WALL_FRONT,   INPUT);
    pinMode(IR_WALL_BACK,    INPUT);
    Serial.println(F("IR sensors initialized"));
}

// ─── PUBLIC: call every loop ───────────────────────────────────────────────
// Returns true if currently avoiding — motor_routines should not run idle during this
bool ir_update() {
    unsigned long now = millis();

    if (now - _irLastTrigger < IR_COOLDOWN_MS) return _irAvoiding;

    // ── Handle active avoidance stages ──
    if (_irAvoiding && now >= _irAvoidUntil) {
        if (_irAvoidStage == 0) {
            // Spin done — now reverse
            _irAvoidStage = 1;
            _irAvoidUntil = now + IR_AVOID_DURATION_MS;
            motors_backward(MOTOR_SPEED_HALF);
        } else {
            // Reverse done — back to idle
            _irAvoiding = false;
            _irAvoidStage = 0;
            motors_setRoutine(ROUTINE_IDLE);
        }
        return _irAvoiding;
    }

    if (_irAvoiding) return true;

    // ── Read all sensors ──
    bool cliffLeft   = digitalRead(IR_DOWN_LEFT)    == LOW;
    bool cliffRight  = digitalRead(IR_DOWN_RIGHT)   == LOW;
    bool cliffFrontL = digitalRead(IR_DOWN_FRONT_L) == LOW;
    bool cliffFrontR = digitalRead(IR_DOWN_FRONT_R) == LOW;
    bool wallFront   = digitalRead(IR_WALL_FRONT)   == HIGH;
    bool wallBack    = digitalRead(IR_WALL_BACK)    == HIGH;

    // ── Internal helper to start avoidance ──
    auto startAvoid = [&](int spinDir) {
        _irAvoiding    = true;
        _irAvoidStage  = 0;
        _irAvoidUntil  = now + 500; // spin for 500ms
        _irLastTrigger = now;
        sound_play(SOUND_YELL);
        if      (spinDir == -1) motors_turnLeft(TURN_SPEED);
        else if (spinDir ==  1) motors_turnRight(TURN_SPEED);
        else                    motors_stop(); // front/both — just pause before reverse
    };

    if (cliffFrontL || cliffFrontR)    { startAvoid(0);  return true; }
    if (cliffLeft  && !cliffRight)     { startAvoid(-1); return true; } // spin left toward cliff
    if (cliffRight && !cliffLeft)      { startAvoid(1);  return true; } // spin right toward cliff
    if (cliffLeft  && cliffRight)      { startAvoid(0);  return true; }
    if (wallFront) { sound_play(SOUND_YELL); startAvoid(0); return true; }
    if (wallBack)  {
        // Wall behind — move forward instead of reverse in stage 1
        _irAvoiding    = true;
        _irAvoidStage  = 1; // skip spin, go straight to moving away
        _irAvoidUntil  = now + IR_AVOID_DURATION_MS;
        _irLastTrigger = now;
        sound_play(SOUND_YELL);
        motors_forward(MOTOR_SPEED_HALF);
        return true;
    }

    return false;
}