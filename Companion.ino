#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <FluxGarage_RoboEyes.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Preferences.h>
#include "globals.h"
#include "wakeup_anim.h"
#include "info_display.h"
#include "motors.h"
#include "motor_routines.h"
#include "servos.h"
//#include "ir_sensors.h"

bool InReaction = false;
int  relationshipMeter = 0;

#include "reactions.h"

// ─── OLED DEFINITIONS ──────────────────────────────────────────────────────
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_ADDRESS  0x3D  // Try 0x3C if alternative needed

// ─── IMU DEFINITIONS ───────────────────────────────────────────────────────
#define MPU6050_ADDRESS 0x68  // 0x68 if AD0 is low, 0x69 if HIGH

// ─── PIN DECLARATIONS ──────────────────────────────────────────────────────
#define SDA_PIN 18
#define SCL_PIN 21

// ─── GLOBALS ───────────────────────────────────────────────────────────────
Adafruit_MPU6050             mpu;
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
RoboEyes<Adafruit_SH1106G>   roboEyes(display);
Preferences                  prefs;

// ─── RELATIONSHIP ──────────────────────────────────────────────────────────
void relationship_add(int amount) {
    relationshipMeter = constrain(relationshipMeter + amount, -100, 100);
    prefs.putInt("rel", relationshipMeter);
    Serial.print(F("Relationship: "));
    Serial.println(relationshipMeter);
}

// ─── INIT FUNCTIONS ────────────────────────────────────────────────────────
void init_OLED() {
    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(100000); // Keep at 100kHz — MPU and OLED conflict at higher speeds. Needs 4.7k pull-up resistors.
    if (!display.begin(OLED_ADDRESS, true)) {
        Serial.println(F("SSD1106 allocation failed"));
        for (;;);
    }
    display.clearDisplay();
    display.display();
}

void init_RoboEyes() {
    // Refer to example i2c_SSD1306_Basics for more initialization options
    roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 100);
    // roboEyes.setWidth(36, 36);
    // roboEyes.setHeight(36, 36);
    // roboEyes.setBorderradius(8, 8);
    // roboEyes.setSpacebetween(10);
    roboEyes.setMood(DEFAULT);
    roboEyes.setPosition(DEFAULT);
    roboEyes.setCuriosity(OFF);
}

void init_MPU6050() {
    if (!mpu.begin(MPU6050_ADDRESS)) {
        Serial.println(F("MPU6050 allocation failed"));
        for (;;);
    }
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    Serial.println(F("MPU6050 initialized"));
}

void init_Relationship() {
    prefs.begin("companion", false);
    relationshipMeter = prefs.getInt("rel", 0); // 0 = default on first boot
    Serial.print(F("Relationship loaded: "));
    Serial.println(relationshipMeter);
}

void relationship_reset(int defaultValue = 0) {//testing
    relationshipMeter = constrain(defaultValue, -100, 100);
    prefs.putInt("rel", relationshipMeter);

    Serial.print(F("Relationship reset to: "));
    Serial.println(relationshipMeter);
}
// ─── SETUP ─────────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    init_OLED();
    init_RoboEyes();
    init_MPU6050();
    init_Relationship();
    //relationship_reset();
    infoDisplay_init();
    motors_init();
    motors_setRoutine(ROUTINE_IDLE);
    servos_init();
    //ir_init();
    reactions_init(mpu, roboEyes);
}

// ─── LOOP ──────────────────────────────────────────────────────────────────
void loop() {
    // Play wakeup animation on first boot, then hand off to normal loop
    if (wakeup_update(display, roboEyes)) return;

    roboEyes.update();
    infoDisplay_update();
    servos_update();
    //bool irBusy = ir_update(); // IR overrides motors when avoiding

    static unsigned long lastReactionPoll = 0;
    if (millis() - lastReactionPoll >= 20) {
        lastReactionPoll = millis();
        //if (!irBusy) motors_update(); // only wander when IR isn't avoiding
        reactions_update(mpu, roboEyes);
    }
}
