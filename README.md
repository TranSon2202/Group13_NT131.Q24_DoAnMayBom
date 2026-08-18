# 🌱 Smart Plant Irrigation System – ESP32

## 📌 Project Overview

This project is a **Smart Plant Irrigation System** that automatically controls a water pump based on soil moisture levels.

The system uses an **ESP32** connected to a soil moisture sensor and a relay-controlled water pump. The ESP32 communicates with a cloud-based REST API hosted on **Microsoft Azure** to:

- Send soil moisture data to the server.
- Retrieve configured moisture thresholds.
- Receive manual pump control commands.
- Automatically control the pump based on soil moisture.
- Prevent automatic watering during configured time periods.

The goal of the project is to reduce manual watering and provide a simple IoT-based solution for plant irrigation.

---

## 🏗️ System Architecture

```text
                  ┌──────────────────────┐
                  │      Mobile/Web App  │
                  └──────────┬───────────┘
                             │
                             │ REST API
                             ▼
                  ┌──────────────────────┐
                  │    Azure Web API     │
                  │                      │
                  │  Pump Status API     │
                  │  Humidity API        │
                  │  Device Config API   │
                  └──────────┬───────────┘
                             │
                           Wi-Fi
                             │
                             ▼
                  ┌──────────────────────┐
                  │        ESP32         │
                  │                      │
                  │  ┌────────────────┐  │
                  │  │ Moisture Sensor│  │
                  │  └───────┬────────┘  │
                  │          │           │
                  │  ┌───────▼────────┐  │
                  │  │ Control Logic   │  │
                  │  └───────┬────────┘  │
                  │          │           │
                  │  ┌───────▼────────┐  │
                  │  │     Relay      │  │
                  │  └───────┬────────┘  │
                  └──────────┼───────────┘
                             │
                             ▼
                       ┌──────────┐
                       │ Water    │
                       │ Pump     │
                       └──────────┘
```

---

## 🔧 Hardware Components

| Component | Description |
|---|---|
| ESP32 | Main microcontroller |
| Soil Moisture Sensor | Measures soil moisture |
| Relay Module | Controls the water pump |
| DC Water Pump | Pumps water to the plants |
| Power Supply | Provides power for the ESP32 and pump |
| Water Pipe | Transfers water to the plants |

### Pin Configuration

| Component | ESP32 Pin |
|---|---|
| Soil Moisture Sensor | GPIO 35 |
| Relay Module | GPIO 17 |

---

## 💻 Software & Technologies

- **ESP32**
- **Arduino IDE**
- **C/C++**
- **ArduinoJson**
- **Wi-Fi**
- **HTTP/REST API**
- **JSON**
- **Microsoft Azure**
- **NTP (Network Time Protocol)**

---

# ⚙️ System Features

## 1. Soil Moisture Monitoring

The ESP32 reads the analog value from the soil moisture sensor:

```cpp
int value = analogRead(SENSOR_PIN);
```

The ESP32 ADC provides a value between approximately `0` and `4095`.

The sensor value is converted into a moisture percentage:

```cpp
int humidityPercent = map(value, 4095, 2000, 0, 100);
humidityPercent = constrain(humidityPercent, 0, 100);
```

The result is constrained to a value between `0%` and `100%`.

---

## 2. Automatic Irrigation Mode

In automatic mode, the ESP32 compares the current soil moisture with two configurable thresholds.

For example:

```text
Lower Threshold = 30%
Upper Threshold = 80%

Moisture <= 30%  → Pump ON
Moisture >= 80%  → Pump OFF
```

Using two thresholds provides hysteresis and helps prevent the relay from switching ON/OFF too frequently.

---

# 🎮 Manual Pump Control

The system can also receive pump commands from the server.

The ESP32 sends a `GET` request to the Pump Status API and receives JSON data such as:

```json
{
    "pumpOnOrOff": true
}
```

If:

```json
"pumpOnOrOff": true
```

the ESP32 turns the pump ON.

If:

```json
"pumpOnOrOff": false
```

the ESP32 turns the pump OFF.

---

# ☁️ Cloud API Communication

The ESP32 communicates with REST APIs hosted on Microsoft Azure.

### Pump Status

```text
GET /api/Pump/Status
```

Used to retrieve the current pump command.

### Upload Soil Moisture

```text
POST /api/Humidity
```

Example JSON:

```json
{
    "value": 45,
    "deviceName": "ESP32"
}
```

### Get Device Configuration

```text
GET /api/Device/config?deviceName=ESP32
```

Example response:

```json
{
    "lowerThreshold": 30,
    "upperThreshold": 80
}
```

---

# ⏰ Watering Schedule

The system uses **NTP** to obtain the current time.

Timezone:

```text
GMT+7
```

Automatic watering is blocked during:

```text
10:00 → 15:00
18:00 → 06:00
```

Therefore, automatic watering is allowed during:

```text
06:00 → 10:00
15:00 → 18:00
```

If the system cannot obtain the current time, watering is allowed by default.

---

# 🔄 Operating Modes

The system has two main control modes.

## AUTO Mode

The ESP32 controls the pump based on:

- Soil moisture.
- Lower threshold.
- Upper threshold.
- Watering schedule.

```text
             AUTO MODE
                 │
                 ▼
        Read Soil Moisture
                 │
                 ▼
        Check Watering Time
          /              \
       Block             Allow
         │                 │
       Pump OFF       Check Moisture
                           │
                    ┌──────┴──────┐
                    ▼             ▼
               Too Dry        Moist Enough
                    │             │
                 Pump ON       Pump OFF
```

## Manual/API Mode

When a pump command is received from the API, the ESP32 temporarily switches from automatic control to API control.

After the configured timeout expires, the system returns to AUTO mode.

Current timeout:

```cpp
const long apiTimeout = 60000;
```

This corresponds to **60 seconds**.

---

# 📡 Data Flow

The main loop performs the following operations:

```text
1. Get current time
        ↓
2. Read soil moisture
        ↓
3. Get moisture thresholds from API
        ↓
4. Send moisture data to API
        ↓
5. Get pump status from API
        ↓
6. Process manual pump command
        ↓
7. Check API timeout
        ↓
8. Run AUTO mode if necessary
        ↓
9. Wait 2 seconds
        ↓
10. Repeat
```

---

# 📦 Required Arduino Libraries

The project requires:

```cpp
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
```

Install **ArduinoJson** through the Arduino IDE Library Manager if it is not already installed.

---

# 🚀 Installation & Setup

## 1. Install Arduino IDE

Install Arduino IDE and configure it for ESP32 development.

## 2. Install ESP32 Board Package

Add ESP32 support to Arduino IDE and select the appropriate ESP32 board.

## 3. Install ArduinoJson

Open:

```text
Tools → Manage Libraries
```

Search for:

```text
ArduinoJson
```

and install it.

## 4. Configure Wi-Fi

Update the following variables:

```cpp
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
```

## 5. Configure API Endpoints

Set the API URLs:

```cpp
const char* serverName = "YOUR_PUMP_STATUS_API";
const char* PostDoam = "YOUR_HUMIDITY_API";
const char* GetDoam = "YOUR_DEVICE_CONFIG_API";
```

## 6. Upload the Program

Connect the ESP32 to the computer using USB and upload the program.

Open Serial Monitor at:

```text
115200 baud
```

---

# 🖥️ Serial Monitor

The ESP32 prints useful information for debugging.

Example:

```text
Connecting....
192.168.1.10

Time: 08:30
2500
Do am theo %: 76

AUTO: OFF
```

When the soil becomes dry:

```text
Do am theo %: 25
AUTO: ON
```

When the soil becomes sufficiently wet:

```text
Do am theo %: 82
AUTO: OFF
```

---

# 🔐 Security Note

Do **not** commit real Wi-Fi passwords, API keys, tokens, or other credentials to a public GitHub repository.

Instead of:

```cpp
const char* password = "YOUR_REAL_PASSWORD";
```

use a separate configuration file or another secure configuration mechanism.

---

# 📁 Suggested Project Structure

```text
Smart-Irrigation-ESP32/
│
├── Smart-Irrigation-ESP32.ino
├── README.md
│
├── docs/
│   ├── architecture.png
│   └── circuit-diagram.png
│
└── screenshots/
    └── serial-monitor.png
```

---

# 🎯 Project Objectives

The main objectives of this project are:

- Build an IoT-based automatic irrigation system.
- Monitor soil moisture in real time.
- Automatically control a water pump.
- Allow remote pump control through a REST API.
- Store and manage irrigation configuration through a cloud backend.
- Prevent watering during inappropriate time periods.
- Connect embedded hardware with a cloud-based application.

---

# 🔮 Future Improvements

Possible improvements include:

- Develop a mobile application for controlling the system.
- Add real-time moisture charts.
- Add notifications when the soil becomes too dry.
- Support multiple ESP32 devices.
- Add API authentication using JWT or API keys.
- Store historical moisture data in a database.
- Integrate weather information to avoid unnecessary watering.
- Add battery and solar-power support.
- Use machine learning to optimize watering schedules.

---

# 👨‍💻 Conclusion

This project demonstrates how an **ESP32 IoT device** can communicate with a **cloud-based REST API** to build an intelligent irrigation system.

The combination of:

```text
ESP32
  +
Soil Moisture Sensor
  +
Relay & Water Pump
  +
Wi-Fi
  +
REST API
  +
Microsoft Azure
```

creates a system capable of both **automatic irrigation** and **remote pump control**.

The project can also serve as a foundation for developing a larger **IoT + Cloud Computing** system with multiple devices, centralized monitoring, data storage, and intelligent irrigation control.
