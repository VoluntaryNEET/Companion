#pragma once
#include <Arduino.h>

// ─── MIC PIN DEFINITION ────────────────────────────────────────────────────
#define MIC_PIN 4  // ADC input-only pin, good for analog — change if needed.

// ─── SPIKE DETECTION TUNING ────────────────────────────────────────────────
#define MIC_SAMPLE_COUNT    8      // readings per poll to find peak amplitude
#define MIC_SPIKE_THRESHOLD 220    // ADC units above baseline to count as loud spike (0-4095)
#define MIC_COOLDOWN_MS     800    // ms to ignore further spikes after one fires (prevents echo re-trigger)

// ─── STATE ─────────────────────────────────────────────────────────────────
static int           _micBaseline    = 2048; // ADC midpoint, recalibrated on init
static unsigned long _micLastSpike   = 0;

// ─── INIT ───────────────────────────────────────────────────────────────────
void listen_init() {
    delay(500);
    long sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += analogRead(MIC_PIN);
        delay(5);
    }
    _micBaseline = sum / 32;
    _micLastSpike = millis(); // block triggers during initial settle period
    Serial.print(F("Mic baseline: "));
    Serial.println(_micBaseline);
}

// ─── DETECTION ─────────────────────────────────────────────────────────────
// Returns true once when a loud spike is detected, then cooldown before next trigger
bool listen_isLoudSpike() {
    unsigned long now = millis();

    // Cooldown — ignore spikes too close together (echo, reverb)
    if (now - _micLastSpike < MIC_COOLDOWN_MS) return false;

    // Take multiple samples and find peak deviation from baseline
    int peak = 0;
    for (int i = 0; i < MIC_SAMPLE_COUNT; i++) {
        int val       = analogRead(MIC_PIN);
        int deviation = abs(val - _micBaseline);
        if (deviation > peak) peak = deviation;
    }

    if (peak >= MIC_SPIKE_THRESHOLD) {
        Serial.println(peak);
        _micLastSpike = now;
        return true;
    }

    return false;
}