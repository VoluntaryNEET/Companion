#pragma once
#include <Adafruit_SSD1306.h>
#include "globals.h"

// ─── DISPLAY DEFINITIONS ───────────────────────────────────────────────────
#define INFO_SCREEN_WIDTH   128
#define INFO_SCREEN_HEIGHT  64
#define INFO_OLED_RESET     -1
#define INFO_OLED_ADDRESS   0x3C

// ─── BATTERY (commented out until voltage divider is wired) ────────────────
// #define BATTERY_PIN         35
// #define BATTERY_MAX_V       4.2f
// #define BATTERY_MIN_V       3.0f
// #define BATTERY_DIVIDER     2.0f  // voltage divider halves the input

// ─── STATE ─────────────────────────────────────────────────────────────────
static Adafruit_SSD1306 infoDisplay(INFO_SCREEN_WIDTH, INFO_SCREEN_HEIGHT, &Wire, INFO_OLED_RESET);
static unsigned long    _infoLastUpdate = 0;
#define INFO_UPDATE_MS  500  // refresh rate — no need to redraw every frame

// ─── INTERNAL: test if a pixel is inside the heart shape ──────────────────
// Uses heart curve equation: (x²+y²-1)³ - x²y³ ≤ 0
// px, py are pixel coords normalized relative to heart center and size
static bool _inHeart(int px, int py, int cx, int cy, int sz) {
    // Normalize to -1..1 range, flip y so heart points down
    float x =  (float)(px - cx) / (float)sz;
    float y = -(float)(py - cy) / (float)sz;
    // Shift up slightly so heart is centered better visually
    y += 0.3f;
    float val = (x*x + y*y - 1.0f);
    return (val * val * val) - (x * x * y * y * y) <= 0.0f;
}

// ─── INTERNAL: draw heart outline only ────────────────────────────────────
static void _drawHeartOutline(int cx, int cy, int sz, uint16_t color) {
    int bound = sz + 2;
    for (int py = cy - bound; py <= cy + bound; py++) {
        for (int px = cx - bound; px <= cx + bound; px++) {
            // Draw pixel if it's on the edge — inside but neighbour is outside
            if (_inHeart(px, py, cx, cy, sz)) {
                bool edge = !_inHeart(px-1, py, cx, cy, sz) ||
                            !_inHeart(px+1, py, cx, cy, sz) ||
                            !_inHeart(px, py-1, cx, cy, sz) ||
                            !_inHeart(px, py+1, cx, cy, sz);
                if (edge) infoDisplay.drawPixel(px, py, color);
            }
        }
    }
}

// ─── INTERNAL: draw heart filled up to a fill line ────────────────────────
static void _drawHeartFilled(int cx, int cy, int sz, int fillFromY, uint16_t color) {
    int bound = sz + 2;
    for (int py = cy - bound; py <= cy + bound; py++) {
        for (int px = cx - bound; px <= cx + bound; px++) {
            if (_inHeart(px, py, cx, cy, sz)) {
                if (py >= fillFromY) {
                    infoDisplay.drawPixel(px, py, color); // filled region
                }
            }
        }
    }
}

// ─── INTERNAL: draw zigzag crack inside heart ─────────────────────────────
static void _drawCrack(int cx, int cy, int sz) {
    // Jagged lightning bolt style crack down the center of the heart
    // Points defined relative to heart center
    int pts[][2] = {
        { 0, -sz + 2},
        {-3, -sz/2  },
        { 2,  0     },
        {-2,  sz/2  },
        { 0,  sz - 2}
    };
    for (int i = 0; i < 4; i++) {
        infoDisplay.drawLine(
            cx + pts[i][0],   cy + pts[i][1],
            cx + pts[i+1][0], cy + pts[i+1][1],
            SSD1306_WHITE
        );
    }
}

// ─── INTERNAL: draw heart fill based on relationship ───────────────────────
static void _drawRelationshipHeart() {
    int cx = 64;  // center of screen horizontally
    int cy = 24;  // center vertically
    int sz = 14;  // heart size — controls overall scale

    int heartTop    = cy - sz;
    int heartBottom = cy + sz;
    int heartHeight = heartBottom - heartTop;

    if (relationshipMeter >= 0) {
        float pct    = (float)relationshipMeter / 100.0f;
        int fillFrom = heartBottom - (int)(pct * heartHeight);

        // Draw filled region clipped to heart shape
        _drawHeartFilled(cx, cy, sz, fillFrom, SSD1306_WHITE);
        // Draw outline on top to keep it crisp
        _drawHeartOutline(cx, cy, sz, SSD1306_WHITE);

        // Percentage below
        infoDisplay.setTextSize(1.5);
        infoDisplay.setTextColor(SSD1306_WHITE);
        infoDisplay.setCursor(cx + sz + 6, cy - 6);
        infoDisplay.print(relationshipMeter);infoDisplay.print("%");

    } else {
        // Negative — outline heart with zigzag crack
        _drawHeartOutline(cx, cy, sz, SSD1306_WHITE);
        _drawCrack(cx, cy, sz);

        // Negative number below
        infoDisplay.setTextSize(1.5);
        infoDisplay.setTextColor(SSD1306_WHITE);
        infoDisplay.setCursor(cx + sz + 6, cy - 6);
        infoDisplay.print(relationshipMeter);infoDisplay.print("%");
    }
}

// ─── INTERNAL: draw battery (commented until wired) ────────────────────────
static void _drawBattery() {
    // ── Uncomment when voltage divider is wired to BATTERY_PIN ──
    //
    // int raw     = analogRead(BATTERY_PIN);
    // float v     = (raw / 4095.0f) * 3.3f * BATTERY_DIVIDER;
    // float pct   = ((v - BATTERY_MIN_V) / (BATTERY_MAX_V - BATTERY_MIN_V)) * 100.0f;
    // pct         = constrain(pct, 0.0f, 100.0f);
    //
    // // Lightning bolt at left
    // infoDisplay.drawLine(10, 48, 7,  56, SSD1306_WHITE);
    // infoDisplay.drawLine(7,  56, 12, 56, SSD1306_WHITE);
    // infoDisplay.drawLine(12, 56, 9,  64, SSD1306_WHITE);
    //
    // // Percentage text
    // infoDisplay.setTextSize(1);
    // infoDisplay.setTextColor(SSD1306_WHITE);
    // infoDisplay.setCursor(18, 54);
    // infoDisplay.print((int)pct);
    // infoDisplay.print("%");

    // Placeholder until battery is wired
    infoDisplay.setTextSize(1);
    infoDisplay.setTextColor(SSD1306_WHITE);
    infoDisplay.setCursor(4, 54);
    infoDisplay.print("BAT: ---%");
}

// ─── PUBLIC API ────────────────────────────────────────────────────────────
void infoDisplay_init() {
    if (!infoDisplay.begin(SSD1306_SWITCHCAPVCC, INFO_OLED_ADDRESS)) {
        Serial.println(F("Info display failed"));
        return;
    }
    infoDisplay.clearDisplay();
    infoDisplay.display();
    Serial.println(F("Info display ready"));
}

void infoDisplay_update() {
    unsigned long now = millis();
    if (now - _infoLastUpdate < INFO_UPDATE_MS) return;
    _infoLastUpdate = now;

    infoDisplay.clearDisplay();
    _drawRelationshipHeart();
    _drawBattery();
    infoDisplay.display();
}