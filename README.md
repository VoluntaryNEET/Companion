Got it, here's everything for the GitHub page:

---

# Goobo Sr. — Version 3
### Autonomous Desktop Companion Robot | ESP32

> A small emotionally expressive autonomous robot that reacts to touch, shaking, and loud sounds with animated eyes, arm movements, and audio. Features a persistent relationship meter that evolves over time based on how you treat it.

---

## Demo
*(Upload a video or gif here)*

---

## Features

- 🤖 **Animated OLED eyes** — RoboEyes-powered expressions including happy, angry, tired, and default idle with autoblink and wander
- 💪 **Servo arms** — Two MG90S servos act as arms, expressing emotions with cheers, slams, startled raises, and tired droops
- ❤️ **Relationship meter** — Integer value from -100 to 100 persisted in ESP32 NVS flash. Same input produces different reactions depending on current relationship state
- 🔊 **Audio reactions** — DFPlayer Mini MP3 module plays contextual sounds for each reaction
- 👆 **Touch sensor** — TTP223 capacitive sensor using hardware interrupts
- 📳 **Shake detection** — MPU6050 IMU detects violent shaking with horizontal/vertical direction awareness
- 🎙️ **Sound detection** — MAX9814 microphone detects loud spikes like claps or sudden sounds
- 🚗 **Autonomous movement** — Random walk idle behaviour via TB6612FNG motor driver
- 🚧 **Cliff and wall avoidance** — 6x MH Flying Fish IR sensors prevent the robot from falling off surfaces or hitting walls
- 📺 **Secondary display** — SSD1306 OLED shows a relationship heart (fills when positive, cracks when negative) and battery status

---

## Hardware

| Component | Model | Interface |
|---|---|---|
| Microcontroller | ESP32 WROOM-32 | — |
| Eye Display | SH1106 128x64 OLED | I2C 0x3D |
| Info Display | SSD1306 128x64 OLED | I2C 0x3C |
| IMU | MPU6050 / GY-521 | I2C 0x68 |
| Touch Sensor | TTP223 | GPIO23 Interrupt |
| Microphone | MAX9814 | ADC GPIO2 |
| Motor Driver | TB6612FNG | PWM GPIO32/26 |
| DC Motors | Gear motors x2 | Via TB6612FNG |
| Arm Servos | MG90S x2 | PWM GPIO15/22 |
| IR Sensors | MH Flying Fish x6 | GPIO 4,5,13,35,36,39 |
| Audio | DFPlayer Mini | UART2 GPIO16/17 |
| Speaker | 8W | Via DFPlayer |

---

## Pin Map

| GPIO | Component | Notes |
|---|---|---|
| 18 | I2C SDA | Shared — both OLEDs + MPU6050 |
| 21 | I2C SCL | Shared |
| 16 | DFPlayer RX2 | UART2 |
| 17 | DFPlayer TX2 | UART2 — via 1kΩ resistor |
| 2 | MAX9814 Out | Analog ADC |
| 23 | TTP223 Out | Hardware interrupt |
| 32 | PWMA | Motor A speed |
| 33 | AIN1 | Motor A direction |
| 25 | AIN2 | Motor A direction |
| 26 | PWMB | Motor B speed |
| 27 | BIN1 | Motor B direction |
| 14 | BIN2 | Motor B direction |
| 15 | Servo Left | PWM |
| 22 | Servo Right | PWM |
| 36 | IR Down Left | Input-only |
| 39 | IR Down Right | Input-only |
| 35 | IR Down Front L | Input-only |
| 4 | IR Down Front R | — |
| 5 | IR Wall Front | — |
| 13 | IR Wall Back | — |
| 34 | Battery ADC | Input-only, voltage divider |

---

## SD Card Setup (DFPlayer)

Format as FAT32. Files must follow this exact structure:

```
SD:/
  01/
    001.mp3   — Happy sound
    002.mp3   — Angry sound
    003.mp3   — Laugh / touched
    004.mp3   — Shutdown
    005.mp3   — Wakeup
    006.mp3   — Yell / startled
```

---

## Software Architecture

```
Companion.ino          — Main sketch
globals.h              — Shared extern declarations
reactions.h            — Reaction orchestrator (priority system)
reactions_IMU.h        — Shake detection → ShakeDirection
reactions_TOUCH.h      — Hardware interrupt touch → TouchResult
reactions_LISTEN.h     — Mic spike detection → bool
motors.h               — TB6612FNG primitives
motor_routines.h       — Idle wander, jiggle, reverse routines
servos.h               — Arm expressions
ir_sensors.h           — Cliff and wall avoidance
sound_PLAY.h           — DFPlayer sound ID mapping
wakeup_anim.h          — Boot animation sequence
info_display.h         — Secondary display heart + battery
```

---

## Reaction System

All reactions are interrupt-style — they fire instantly, run for a fixed duration, then return to idle. Shake has the highest priority and can preempt any other active reaction.

| Trigger | Relationship ≥ 0 | Relationship < 0 |
|---|---|---|
| Shake | TIRED mood + scared sound | ANGRY mood + angry sound |
| Touch | HAPPY + laugh + ARM_HAPPY | Confused + ARM_TIRED |
| Loud sound | TIRED + flicker + ARM_STARTLED | TIRED + flicker + ARM_STARTLED |

Relationship changes per event:
- Shake: **-10**
- Touch positive: **+5**
- Touch negative: **+2**
- Loud sound: **-5**

---

## Libraries Required

Install via Arduino Library Manager:

- `Adafruit SSD1306`
- `Adafruit GFX Library`
- `Adafruit SH110X`
- `Adafruit MPU6050`
- `Adafruit Unified Sensor`
- `ESP32Servo`
- `DFRobotDFPlayerMini`

Install manually (not on Library Manager):

- [FluxGarage RoboEyes](https://github.com/FluxGarage/RoboEyes) — download ZIP and add via Sketch → Include Library → Add .ZIP

Board support: Add ESP32 to Arduino IDE via Boards Manager URL:
```
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

---

## Board Settings

| Setting | Value |
|---|---|
| Board | ESP32 Dev Module |
| Upload Speed | 921600 |
| Flash Size | 4MB |
| Partition Scheme | Default 4MB with spiffs |

---

## Notes

- I2C runs at **100kHz** — do not increase. Higher speeds cause conflicts between the OLED and MPU6050. Requires **4.7kΩ pull-up resistors** on SDA and SCL.
- DFPlayer RX line requires a **1kΩ series resistor** from ESP32 TX.
- DFPlayer `begin()` uses `false` flag to disable ACK handshake — required for clone modules.
- GPIO 34, 35, 36, 39 are **input-only** on ESP32 — do not use for outputs.
- GPIO 12 is a strapping pin — keep LOW at boot.

---

## Credits

Built by:
- FA24-BCE-089 — Shafay Mahmood
- FA24-BCE-090 — Shayan Arshad
- FA24-BCE-084 — Rana Zakaria Samad

Eye animation engine by [FluxGarage RoboEyes](https://github.com/FluxGarage/RoboEyes)

---

*Goobo Sr. Version 3 — 2025*
