# ESP32 Relay X4 Modbus

![License](https://img.shields.io/badge/license-MIT-green)
![Platform](https://img.shields.io/badge/platform-ESP32-blue)
![Protocol](https://img.shields.io/badge/protocol-Modbus%20RTU-orange)
![Status](https://img.shields.io/badge/status-stable-brightgreen)

Modbus RTU firmware for ESP32 4-channel relay boards with RS485.

This project provides a robust Modbus slave implementation with persistent configuration and relay state restore.

---

## 📸 Hardware

![Board](docs/board.jpg)

Tested on:

* ESP32 Relay X4 Modbus board (v1.3) https://it.aliexpress.com/item/1005010277681749.html
* USB-C (CH340)
* RS485 onboard transceiver

---

## 📦 Features

* 4x Relay outputs
* 4x Digital inputs (optoisolated)
* RS485 Modbus RTU
* Relay state persistence (restore after reboot)
* Configurable Slave ID
* Configurable Baudrate
* Configurable boot behavior
* Factory reset support

---

## 🔌 Wiring (RS485)

```text
ESP32 Board        RS485 Master
-----------        --------------
A  (D+)   -------- A
B  (D-)   -------- B
GND       -------- GND (recommended)
```

---

## ⚙️ Modbus Configuration (Default)

| Parameter | Value |
| --------- | ----- |
| Slave ID  | 3     |
| Baudrate  | 9600  |
| Frame     | 8N1   |

---

## 📊 Register Map

### Relay Control (Holding Registers)

| Register | Description |
| -------- | ----------- |
| 0        | Relay 1     |
| 1        | Relay 2     |
| 2        | Relay 3     |
| 3        | Relay 4     |

Values:

* `0` = OFF
* `1` = ON

---

### Relay State (Input Registers)

| Register | Description   |
| -------- | ------------- |
| 0        | Relay 1 State |
| 1        | Relay 2 State |
| 2        | Relay 3 State |
| 3        | Relay 4 State |

---

### Digital Inputs (Input Registers)

| Register | Description |
| -------- | ----------- |
| 10       | IN1         |
| 11       | IN2         |
| 12       | IN3         |
| 13       | IN4         |

---

## ⚙️ Configuration Registers

### Holding Registers

| Register | Description   |
| -------- | ------------- |
| 100      | Slave ID      |
| 101      | Baudrate      |
| 102      | Boot Mode     |
| 103      | Apply Command |

---

### Input Registers (Diagnostics)

| Register | Description       |
| -------- | ----------------- |
| 100      | Current Slave ID  |
| 101      | Current Baudrate  |
| 102      | Current Boot Mode |
| 103      | Firmware Version  |

---

## 🔧 Configuration Values

### Slave ID (`Hreg 100`)

* Range: `1..247`

---

### Baudrate (`Hreg 101`)

| Value | Baudrate |
| ----- | -------- |
| 0     | 9600     |
| 1     | 19200    |
| 2     | 38400    |
| 3     | 57600    |
| 4     | 115200   |

---

### Boot Mode (`Hreg 102`)

| Value | Behavior           |
| ----- | ------------------ |
| 0     | Restore last state |
| 1     | All relays OFF     |
| 2     | All relays ON      |

---

### Apply Command (`Hreg 103`)

| Value | Action                 |
| ----- | ---------------------- |
| 0     | No action              |
| 1     | Save configuration     |
| 2     | Save + reboot          |
| 3     | Factory reset + reboot |

---

## 🔧 Usage Examples

### Turn ON Relay 1

* Write Holding Register `0 = 1`

---

### Change Slave ID

* Write `Hreg 100 = 7`
* Write `Hreg 103 = 2`

---

### Change Baudrate to 19200

* Write `Hreg 101 = 1`
* Write `Hreg 103 = 2`

---

### Set Boot Mode (All OFF)

* Write `Hreg 102 = 1`
* Write `Hreg 103 = 1`

---

### Factory Reset

* Write `Hreg 103 = 3`

---

## 🔄 Restore State

Relay states are automatically saved and restored after reboot.

---

## 🧪 Tested With

* QModMaster
* Modbus Poll
* BoneIO Black

---

## 🔌 Integrations

### BoneIO Black (Experimental)

Optional controller configuration:

```text
integrations/boneio/controller.json
```

Category:

```text
Other
```

---

## 🔧 Pin Mapping

| Function | GPIO |
| -------- | ---- |
| Relay 1  | 23   |
| Relay 2  | 5    |
| Relay 3  | 4    |
| Relay 4  | 13   |
| Input 1  | 25   |
| Input 2  | 26   |
| Input 3  | 27   |
| Input 4  | 33   |
| RS485 RX | 18   |
| RS485 TX | 19   |
| RS485 DE | 32   |

---

## 📦 Project Structure

```text
.
├── firmware/
│   └── esp32_relay_x4_modbus.ino
├── integrations/
│   └── boneio/
│       └── controller.json
├── docs/
│   └── board.jpg
├── README.md
└── LICENSE
```

---

## ⚠️ Notes

* Relays are **active LOW**
* Inputs are **active LOW**
* GPIO13 may toggle at boot
* RS485 requires DE control (GPIO32)

---

## 📦 Roadmap

* [x] Modbus RTU firmware
* [x] Relay control
* [x] Input reading
* [x] Restore state
* [x] Configurable parameters
* [ ] BoneIO official integration
* [ ] MQTT bridge

---

## 🤝 Contributing

Contributions are welcome!

---

## 📜 License

MIT License
