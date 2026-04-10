# Autonomous Line Follower Robot - ZEM 2026

![Competition](https://img.shields.io/badge/Competition-Zilele_Educației_Mecanice-orange?style=flat-square)
![National](https://img.shields.io/badge/Level-Național-red?style=flat-square)
![Altium](https://img.shields.io/badge/PCB-Altium_Designer-A5915F?style=flat-square)
![ESP32-S3](https://img.shields.io/badge/MCU-ESP32--S3_WROOM--1_N8R8-blueviolet?style=flat-square)
![DRV8833](https://img.shields.io/badge/Driver-DRV8833-green?style=flat-square)
![Custom PCB](https://img.shields.io/badge/Hardware-Custom_PCB-blue?style=flat-square)
![SolidWorks](https://img.shields.io/badge/CAD-SolidWorks-red?style=flat-square)

Fully autonomous line follower robot designed and built from scratch for **Zilele Educației Mecanice (ZEM)** - a national mechatronics competition between Romanian engineering faculties. Everything is custom: PCB designed in Altium Designer, mechanical structure modeled in CAD, firmware written from scratch, and assembled and tuned by hand.

---

## 🏆 Competition

**Zilele Educației Mecanice (ZEM)** is a national-level competition organized between Mechatronics faculties across Romania. Teams design and build autonomous robots from scratch, then compete live - requiring real-time hardware and software adjustments on the competition floor under pressure.

---

## ⚙️ Hardware Architecture

### 🧠 Microcontroller
- **ESP32-S3 WROOM-1 N8R8** - dual-core 240MHz, 8MB flash, 8MB PSRAM, WiFi + BLE for wireless tuning and telemetry

### 🔌 Motor Driver
- **DRV8833** - dual H-bridge motor driver integrated directly on the PCB, controlling two N20 motors

### 🚗 Motors
- **N20 micro gear motors with encoders** - closed-loop speed control via encoder feedback for precise differential drive

### 📡 Sensor Array
- **QTR-8A** - 8-channel reflectance sensor bar for line detection and position calculation
- **QTR-1A** - individual sensors for specific detection zones

### 🔍 Additional Sensors
- **Hall effect sensor** - magnetic field detection
- **Distance sensor** - obstacle detection and course adaptation

### 🖨️ PCB
- Fully custom PCB designed in **Altium Designer**
- All drivers, power regulation, sensor connectors, and MCU integrated on a single board
- Designed for minimal footprint and optimal weight distribution on the chassis

---

## 💻 Firmware

- Written from scratch for ESP32-S3
- **PID control loop** for line following using QTR-8A weighted centroid position
- **Encoder feedback** for closed-loop motor speed control
- Real-time sensor calibration routines

---

## 🔩 Mechanical Design

- Full robot chassis modeled in SolidWorks
- Designed for low center of gravity and sensor bar alignment
- Custom mounts for PCB, motors, and sensor array
- Parts manufactured via 3D printing
  
---

## 🛠️ Tech Stack

| Category | Details |
|----------|---------|
| PCB Design | Altium Designer |
| MCU | ESP32-S3 WROOM-1 N8R8 |
| Motor driver | DRV8833 (on-board) |
| Motors | N20 with encoders |
| Line sensor | QTR-8A (array) |
| Extra sensors | Hall effect, distance, QTR-1A |
| CAD | SolidWorks, Fusion 360 |
| Manufacturing | Custom PCB + 3D printed chassis |
| Firmware | C++ |

---

## 📁 What's in This Repo

- `pcb/` — Altium Designer project files (schematic + layout)
- `firmware/` — ESP32-S3 source code (PID, encoder, sensor routines)
- `cad/` — SolidWorks chassis files
- `photos/` — PCB, assembled robot, competition photos

---

## 🧠 What I Learned

- Full PCB design cycle: schematic → layout → fabrication → assembly → test
- SMD soldering and board bring-up from scratch
- PID tuning on a physical system under real competition conditions
- Integrating multiple sensor types into a single control loop
- Making fast hardware and software decisions under competition time pressure
- Designing for manufacturability and repairability in a competition context

---

## 📅 Ongoing

**2026** | National competition — Zilele Educației Mecanice | UNSTPB Bucharest
