#pragma once
#include "motors.h"

// ─── TUNING ────────────────────────────────────────────────────────────────
#define IDLE_FORWARD_MIN_MS   800    // min time to drive forward during wander
#define IDLE_FORWARD_MAX_MS   2000   // max time to drive forward during wander
#define IDLE_TURN_MIN_MS      300    // min time to turn during wander
#define IDLE_TURN_MAX_MS      800    // max time to turn during wander
#define IDLE_PAUSE_MS         400    // brief pause between moves — feels more natural

#define IDLE_SPEED            140     // slow wander speed — desk safe
#define TURN_SPEED            170     // slightly faster for turns to feel snappy

#define JIGGLE_STEP_MS        200    // ms per jiggle step for happy
#define JIGGLE_STEP_MS_NEG    350    // ms per jiggle step for negative — slower/less energy
#define JIGGLE_STEPS          6      // number of jiggle alternations for happy
#define JIGGLE_STEPS_NEG      4      // fewer jiggles for negative touch

#define YELL_REVERSE_MS       800    // how long to reverse when startled

// ─── STATE ─────────────────────────────────────────────────────────────────
enum MotorRoutine {
    ROUTINE_IDLE,
    ROUTINE_HAPPY_JIGGLE,
    ROUTINE_NEG_JIGGLE,
    ROUTINE_YELL_REVERSE,
    ROUTINE_STOPPED
};

static MotorRoutine  _motorRoutine     = ROUTINE_IDLE;
static unsigned long _motorStepTimer   = 0;
static int           _motorStep        = 0;

// Idle wander sub-states
enum WanderState { WANDER_FORWARD, WANDER_TURN, WANDER_PAUSE };
static WanderState   _wanderState      = WANDER_FORWARD;
static unsigned long _wanderDuration   = 0;

// ─── INTERNAL: start a new wander move ─────────────────────────────────────
static void _nextWanderMove() {
    _motorStepTimer = millis();
    switch (_wanderState) {
        case WANDER_FORWARD:
            _wanderDuration = random(IDLE_FORWARD_MIN_MS, IDLE_FORWARD_MAX_MS);
            motors_forward(IDLE_SPEED);
            break;
        case WANDER_TURN:
            _wanderDuration = random(IDLE_TURN_MIN_MS, IDLE_TURN_MAX_MS);
            // Randomly pick left or right
            if (random(2)) motors_turnLeft(TURN_SPEED);
            else           motors_turnRight(TURN_SPEED);
            break;
        case WANDER_PAUSE:
            _wanderDuration = IDLE_PAUSE_MS;
            motors_stop();
            break;
    }
}

// ─── PUBLIC: set a routine ─────────────────────────────────────────────────
void motors_setRoutine(MotorRoutine routine) {
    _motorRoutine   = routine;
    _motorStep      = 0;
    _motorStepTimer = millis();

    // Kick off immediately
    switch (routine) {
        case ROUTINE_IDLE:
            _wanderState = WANDER_FORWARD;
            _nextWanderMove();
            break;
        case ROUTINE_HAPPY_JIGGLE:
        case ROUTINE_NEG_JIGGLE:
            motors_turnLeft(MOTOR_SPEED_HALF);
            break;
        case ROUTINE_YELL_REVERSE:
            motors_backward(MOTOR_SPEED_FULL);
            break;
        case ROUTINE_STOPPED:
            motors_stop();
            break;
    }
}

// ─── PUBLIC: call every loop ───────────────────────────────────────────────
void motors_update() {
    unsigned long now     = millis();
    unsigned long elapsed = now - _motorStepTimer;

    switch (_motorRoutine) {

        // ── IDLE: random walk wander ──────────────────────────────────────
        case ROUTINE_IDLE:
            if (elapsed >= _wanderDuration) {
                _motorStepTimer = now;
                // Cycle through forward → turn → pause → forward ...
                switch (_wanderState) {
                    case WANDER_FORWARD: _wanderState = WANDER_TURN;    break;
                    case WANDER_TURN:    _wanderState = WANDER_PAUSE;   break;
                    case WANDER_PAUSE:   _wanderState = WANDER_FORWARD; break;
                }
                _nextWanderMove();
            }
            break;

        // ── HAPPY JIGGLE: alternate CW and CCW quickly ───────────────────
        case ROUTINE_HAPPY_JIGGLE:
            if (elapsed >= JIGGLE_STEP_MS) {
                _motorStepTimer = now;
                _motorStep++;
                if (_motorStep >= JIGGLE_STEPS) {
                    // Done — return to idle wander
                    motors_setRoutine(ROUTINE_IDLE);
                } else {
                    if (_motorStep % 2 == 0) motors_turnLeft(MOTOR_SPEED_HALF);
                    else                     motors_turnRight(MOTOR_SPEED_HALF);
                }
            }
            break;

        // ── NEGATIVE JIGGLE: same but slower and fewer steps ─────────────
        case ROUTINE_NEG_JIGGLE:
            if (elapsed >= JIGGLE_STEP_MS_NEG) {
                _motorStepTimer = now;
                _motorStep++;
                if (_motorStep >= JIGGLE_STEPS_NEG) {
                    motors_setRoutine(ROUTINE_IDLE);
                } else {
                    if (_motorStep % 2 == 0) motors_turnLeft(MOTOR_SPEED_SLOW);
                    else                     motors_turnRight(MOTOR_SPEED_SLOW);
                }
            }
            break;

        // ── YELL REVERSE: back up fast then return to idle ────────────────
        case ROUTINE_YELL_REVERSE:
            if (elapsed >= YELL_REVERSE_MS) {
                motors_setRoutine(ROUTINE_IDLE);
            }
            break;

        case ROUTINE_STOPPED:
            break;
    }
}