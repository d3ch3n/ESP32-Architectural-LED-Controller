# Ripado Architecture

**Project:** Ripado

**Subtitle:** ESP32 Architectural LED Controller

Version: 0.1.0-alpha

---

# Philosophy

Ripado is designed around three principles:

- Reliability
- Maintainability
- Expandability

Every module must have a single responsibility.

Modules communicate through well-defined interfaces.

No module should directly depend on implementation details of another module.

---

# System Architecture

                +------------------+
                |   Web Interface  |
                +------------------+
                          |
                          |
                +------------------+
                | Command Manager  |
                +------------------+
                          |
         +----------------+----------------+
         |                |                |
         |                |                |
+----------------+ +----------------+ +----------------+
| Config Manager | | AnimationEngine| | Diagnostics    |
+----------------+ +----------------+ +----------------+
                          |
                          |
                +------------------+
                |  Led Controller  |
                +------------------+
                          |
                     FastLED Driver
                          |
                    Physical LEDs

Future integrations:

- Matter
- MQTT
- eWeLink Cube
- REST API

All integrations must communicate ONLY with CommandManager.

---

# Modules

## Core

Responsible for system initialization.

Responsibilities:

- Boot
- Setup
- Loop
- Scheduler

---

## CommandManager

Responsible for interpreting commands.

Examples:

Power ON

Power OFF

Set Color

Set Brightness

Set Effect

The CommandManager NEVER manipulates LEDs directly.

---

## Animation Engine

Responsible for generating animation frames.

It never receives HTTP requests.

It never communicates with Alexa.

It never accesses Preferences.

Its only responsibility is animation.

---

## Led Controller

Responsible for physical LEDs.

Responsibilities:

- FastLED
- Brightness
- Rendering
- GPIO

---

## Config Manager

Persistent configuration.

Uses:

Preferences

LittleFS

JSON

---

## Web Service

Provides:

Dashboard

Configuration

Commissioning

OTA

Diagnostics

About

---

## Storage

Responsible for:

LittleFS

Preferences

Configuration backup

Restore

---

## Diagnostics

Provides:

Heap

RSSI

FPS

Animation Time

Firmware Version

Uptime

Temperature

---

# Future Modules

Matter

MQTT

eWeLink Cube

REST API

Developer Mode

---

# Coding Standard

Language:

English

Variables:

camelCase

Classes:

PascalCase

Constants:

UPPER_CASE

No delay()

No blocking code

Every class must have a single responsibility.

---

# State Machine

BOOT

↓

CONNECTING_WIFI

↓

READY

↓

POWERING_ON

↓

ON

↓

CHANGING_COLOR

↓

POWERING_OFF

↓

OFF

↓

OTA

↓

ERROR

---

# Development Rules

Never break existing functionality.

Every feature must be documented.

Every release must have a changelog.

Every module must contain comments.
