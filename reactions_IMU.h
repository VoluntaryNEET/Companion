#pragma once
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// ─── SHAKE DETECTION TUNING ────────────────────────────────────────────────
#define SHAKE_THRESHOLD      7.0f  // m/s² delta to count as a spike
#define SHAKE_COUNT_TRIGGER  6     // spikes needed within window to confirm shake
#define SHAKE_WINDOW_MS      600   // ms window for spike accumulation

// ─── STATE ─────────────────────────────────────────────────────────────────
static float         _lastAccelMag     = 0.0f;
static int           _shakeSpikes      = 0;
static unsigned long _shakeWindowStart = 0;

// ─── SHAKE DIRECTION ───────────────────────────────────────────────────────
enum ShakeDirection {
    SHAKE_NONE = 0,
    SHAKE_HORIZONTAL,
    SHAKE_VERTICAL
};

// ─── INIT ───────────────────────────────────────────────────────────────────
void imu_init(Adafruit_MPU6050 &mpu) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    _lastAccelMag     = sqrt(a.acceleration.x * a.acceleration.x +
                             a.acceleration.y * a.acceleration.y +
                             a.acceleration.z * a.acceleration.z);
    _shakeWindowStart = millis();
}

// ─── DETECTION ─────────────────────────────────────────────────────────────
// Returns shake direction once confirmed, then resets — reactions.h decides what to do
ShakeDirection imu_getShakeDirection(Adafruit_MPU6050 &mpu) {
    unsigned long now = millis();

    sensors_event_t a, g, temp;
    if (!mpu.getEvent(&a, &g, &temp)) return SHAKE_NONE;

    float mag = sqrt(a.acceleration.x * a.acceleration.x +
                     a.acceleration.y * a.acceleration.y +
                     a.acceleration.z * a.acceleration.z);

    float delta = fabs(mag - _lastAccelMag);
    _lastAccelMag = mag;

    if (now - _shakeWindowStart > SHAKE_WINDOW_MS) {
        _shakeSpikes      = 0;
        _shakeWindowStart = now;
    }

    if (delta > SHAKE_THRESHOLD) _shakeSpikes++;

    if (_shakeSpikes >= SHAKE_COUNT_TRIGGER) {
        _shakeSpikes = 0;

        // Determine dominant axis — remove gravity bias on Z
        float dx = fabs(a.acceleration.x);
        float dy = fabs(a.acceleration.y);
        float dz = fabs(a.acceleration.z - 9.81f);
        float maxAxis = max({dx, dy, dz});

        return (maxAxis == dx || maxAxis == dy) ? SHAKE_HORIZONTAL : SHAKE_VERTICAL;
    }

    return SHAKE_NONE;
}
