#pragma once
#include <Arduino.h>

// ─── PIN DEFINITIONS ───────────────────────────────────────────────────────
#define MOTOR_PWMA  32
#define MOTOR_AIN1  33
#define MOTOR_AIN2  25
#define MOTOR_PWMB  26
#define MOTOR_BIN1  27
#define MOTOR_BIN2  14

// ─── PWM CONFIG ────────────────────────────────────────────────────────────
#define MOTOR_PWM_FREQ      1000
#define MOTOR_PWM_RES       8

// ─── SPEED PRESETS ─────────────────────────────────────────────────────────
#define MOTOR_SPEED_FULL    255
#define MOTOR_SPEED_HALF    128
#define MOTOR_SPEED_SLOW    80
#define MOTOR_SPEED_STOP    0

void motors_stop() {
    digitalWrite(MOTOR_AIN1, LOW);
    digitalWrite(MOTOR_AIN2, LOW);
    digitalWrite(MOTOR_BIN1, LOW);
    digitalWrite(MOTOR_BIN2, LOW);

    ledcWrite(MOTOR_PWMA, MOTOR_SPEED_STOP);
    ledcWrite(MOTOR_PWMB, MOTOR_SPEED_STOP);
}

// ─── INIT ───────────────────────────────────────────────────────────────────
void motors_init() {
    pinMode(MOTOR_AIN1, OUTPUT);
    pinMode(MOTOR_AIN2, OUTPUT);
    pinMode(MOTOR_BIN1, OUTPUT);
    pinMode(MOTOR_BIN2, OUTPUT);

    ledcAttach(MOTOR_PWMA, MOTOR_PWM_FREQ, MOTOR_PWM_RES);
    ledcAttach(MOTOR_PWMB, MOTOR_PWM_FREQ, MOTOR_PWM_RES);

    motors_stop();
    Serial.println(F("Motors initialized"));
}

// ─── PRIMITIVES ────────────────────────────────────────────────────────────

void motors_forward(uint8_t speed = MOTOR_SPEED_HALF) {
    digitalWrite(MOTOR_AIN1, HIGH);
    digitalWrite(MOTOR_AIN2, LOW);
    digitalWrite(MOTOR_BIN1, HIGH);
    digitalWrite(MOTOR_BIN2, LOW);

    ledcWrite(MOTOR_PWMA, speed);
    ledcWrite(MOTOR_PWMB, speed);
}

void motors_backward(uint8_t speed = MOTOR_SPEED_HALF) {
    digitalWrite(MOTOR_AIN1, LOW);
    digitalWrite(MOTOR_AIN2, HIGH);
    digitalWrite(MOTOR_BIN1, LOW);
    digitalWrite(MOTOR_BIN2, HIGH);

    ledcWrite(MOTOR_PWMA, speed);
    ledcWrite(MOTOR_PWMB, speed);
}

void motors_turnLeft(uint8_t speed = MOTOR_SPEED_HALF) {
    digitalWrite(MOTOR_AIN1, LOW);
    digitalWrite(MOTOR_AIN2, HIGH);
    digitalWrite(MOTOR_BIN1, HIGH);
    digitalWrite(MOTOR_BIN2, LOW);

    ledcWrite(MOTOR_PWMA, speed);
    ledcWrite(MOTOR_PWMB, speed);
}

void motors_turnRight(uint8_t speed = MOTOR_SPEED_HALF) {
    digitalWrite(MOTOR_AIN1, HIGH);
    digitalWrite(MOTOR_AIN2, LOW);
    digitalWrite(MOTOR_BIN1, LOW);
    digitalWrite(MOTOR_BIN2, HIGH);

    ledcWrite(MOTOR_PWMA, speed);
    ledcWrite(MOTOR_PWMB, speed);
}

void motors_curve(uint8_t speedLeft, uint8_t speedRight) {
    digitalWrite(MOTOR_AIN1, HIGH);
    digitalWrite(MOTOR_AIN2, LOW);
    digitalWrite(MOTOR_BIN1, HIGH);
    digitalWrite(MOTOR_BIN2, LOW);

    ledcWrite(MOTOR_PWMA, speedLeft);
    ledcWrite(MOTOR_PWMB, speedRight);
}