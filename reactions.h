#pragma once
#include <FluxGarage_RoboEyes.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_MPU6050.h>
#include "globals.h"
#include "reactions_IMU.h"
#include "reactions_TOUCH.h"
#include "reactions_LISTEN.h"
#include "sound_PLAY.h"
#include "servos.h"
#include "motor_routines.h"

extern void relationship_add(int amount);

// ─── REACTION DURATIONS ────────────────────────────────────────────────────
#define ANGRY_DURATION_MS  1500
#define HAPPY_DURATION_MS  1500
#define TIRED_DURATION_MS  2000

// ─── INTERNAL STATE ────────────────────────────────────────────────────────
static unsigned long reactionEndTime = 0;

// ─── INTERNAL HELPERS ──────────────────────────────────────────────────────
static void startReaction(
    RoboEyes<Adafruit_SH1106G> &eyes,
    uint8_t mood,
    unsigned long duration_ms,
    bool doLaugh    = false,
    bool doConfused = false
) {
    eyes.setMood(mood);
    if (doLaugh)    eyes.anim_laugh();
    if (doConfused) eyes.anim_confused();
    InReaction    = true;
    reactionEndTime = millis() + duration_ms;
}

void reactions_init(Adafruit_MPU6050 &mpu, RoboEyes<Adafruit_SH1106G> &eyes) {
    imu_init(mpu);
    touch_init();
    listen_init();
    sound_init();
    InReaction = false;
}

bool reactions_update(Adafruit_MPU6050 &mpu, RoboEyes<Adafruit_SH1106G> &eyes) {
    unsigned long now = millis();

    // 1. End current reaction when timer expires
    if (InReaction && now >= reactionEndTime) {
        InReaction = false;
        eyes.setMood(DEFAULT);
        eyes.setHFlicker(OFF, 0);
        eyes.setVFlicker(OFF, 0);
        eyes.setIdleMode(true, 2, 2);
        eyes.idleAnimationTimer = now + 800 + random(1200);
        return true;
    }

    // 2. Shake check runs ALWAYS — even mid reaction, so it can preempt touch
    ShakeDirection dir = imu_getShakeDirection(mpu);
    if (dir != SHAKE_NONE) {
        touch_cancelActive();         // kill touch state
        InReaction      = true;
        reactionEndTime = now + 1800;
        // shake:
        servos_setReaction(ARM_ANGRY);
        if (relationshipMeter >= 0) {
            sound_play(SOUND_HAPPY); // scared/worried sound for positive relationship
            eyes.setMood(TIRED);
        } else {
            sound_play(SOUND_ANGRY); // angry sound for negative relationship
            eyes.setMood(ANGRY);
        }

        relationship_add(-10);

        if (dir == SHAKE_HORIZONTAL) {
            eyes.setHFlicker(ON, 7);
            eyes.setVFlicker(OFF, 0);
        } else {
            eyes.setVFlicker(ON, 7);
            eyes.setHFlicker(OFF, 0);
        }
        return true;
    }

    // 3. Touch and future sensors only run when not already in a reaction
    if (InReaction) return true;

    TouchResult touch = touch_getEvent();
    if (touch.event != TOUCH_EVENT_NONE) {
        // Verify finger is actually still down before committing to reaction
        if (!_isr[0].pressed && touch.event == TOUCH_EVENT_HOLD) return false;
        
        InReaction      = true;
        reactionEndTime = now + 1500;
        if (relationshipMeter >= 0) {
            sound_play(SOUND_LAUGH_TOUCHED);
            servos_setReaction(ARM_HAPPY);
            eyes.setMood(HAPPY);
            eyes.anim_laugh();
            relationship_add(5);
        } else {
            sound_play(SOUND_HAPPY);
            servos_setReaction(ARM_TIRED);
            eyes.setMood(DEFAULT);
            eyes.anim_confused();
            relationship_add(2);
        }
        return true;
    }
    // 4. Loud sound — shocked/startled
    if (listen_isLoudSpike()) {
        touch_cancelActive();
        InReaction = true;
        reactionEndTime = now + 1800;
        // mic startled:
        sound_play(SOUND_YELL);
        servos_setReaction(ARM_STARTLED);
        // mic startled:
        motors_setRoutine(ROUTINE_YELL_REVERSE);
        eyes.setMood(TIRED);
        eyes.setHFlicker(ON, 7);
        eyes.setVFlicker(OFF, 0);
        relationship_add(-5); // startled but less punishing than being shaken
        return true;
    }
    // 5. Future sensors slot in here
    return false;
}