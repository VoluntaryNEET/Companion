#pragma once
#include <DFRobotDFPlayerMini.h>
#include <HardwareSerial.h>

// ─── PIN DEFINITIONS ───────────────────────────────────────────────────────
#define DFPLAYER_RX 16
#define DFPLAYER_TX 17

// ─── SOUND IDs ─────────────────────────────────────────────────────────────
#define SOUND_HAPPY         1   // 001.mp3 — positive touch when relationship < 0
#define SOUND_ANGRY         2   // 002.mp3 — shake when relationship < 0
#define SOUND_LAUGH_TOUCHED 3   // 003.mp3 — touch when relationship >= 0
#define SOUND_WAKEUP        5   // 005.mp3 — wakeup animation
#define SOUND_YELL          6   // 006.mp3 — microphone startled

// ─── STATE ─────────────────────────────────────────────────────────────────
static HardwareSerial    _dfSerial(2);
static DFRobotDFPlayerMini _player;

// ─── INIT ───────────────────────────────────────────────────────────────────
void sound_init() {
    _dfSerial.begin(9600, SERIAL_8N1, DFPLAYER_RX, DFPLAYER_TX);
    delay(1000); // give DFPlayer time to boot and read SD card
    if (!_player.begin(_dfSerial, false)) {
        Serial.println(F("DFPlayer failed — check wiring and SD card"));
        return;
    }
    _player.volume(30); // 0-30, adjust to taste
    Serial.println(F("DFPlayer ready"));
}

// ─── PLAY ──────────────────────────────────────────────────────────────────
void sound_play(uint8_t soundID) {
    _player.play(soundID);
}