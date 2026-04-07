#include <ModbusRTU.h>
#include <Preferences.h>
#include <esp_system.h>

ModbusRTU mb;
Preferences prefs;

// =========================
// Hardware mapping
// =========================
#define RS485_RX 18
#define RS485_TX 19
#define RS485_DE 32

const int relayPins[4] = {23, 5, 4, 13};
const int inputPins[4] = {25, 26, 27, 33};

// =========================
// Firmware info
// =========================
#define FW_VERSION 110   // v1.1.0 -> 110

// =========================
// Config defaults
// =========================
#define DEFAULT_SLAVE_ID 3
#define DEFAULT_BAUD_CODE 0   // 9600
#define DEFAULT_BOOT_MODE 0   // restore last state

// =========================
// Config/runtime variables
// =========================
uint8_t currentSlaveId = DEFAULT_SLAVE_ID;
uint8_t currentBaudCode = DEFAULT_BAUD_CODE;
uint8_t currentBootMode = DEFAULT_BOOT_MODE;

uint8_t lastRelayMask = 0xFF;

// =========================
// Helpers
// =========================
uint32_t baudFromCode(uint8_t code) {
  switch (code) {
    case 0: return 9600;
    case 1: return 19200;
    case 2: return 38400;
    case 3: return 57600;
    case 4: return 115200;
    default: return 9600;
  }
}

bool validSlaveId(uint16_t id) {
  return id >= 1 && id <= 247;
}

bool validBaudCode(uint16_t code) {
  return code <= 4;
}

bool validBootMode(uint16_t mode) {
  return mode <= 2;
}

uint8_t makeRelayMaskFromHregs() {
  uint8_t mask = 0;
  for (int i = 0; i < 4; i++) {
    if (mb.Hreg(i) != 0) {
      mask |= (1 << i);
    }
  }
  return mask;
}

void applyRelayMask(uint8_t mask) {
  for (int i = 0; i < 4; i++) {
    bool on = (mask >> i) & 0x01;
    digitalWrite(relayPins[i], on ? LOW : HIGH);   // active LOW
    mb.Hreg(i, on ? 1 : 0);
    mb.Ireg(i, on ? 1 : 0);
  }
}

uint8_t getBootRelayMask(uint8_t bootMode) {
  switch (bootMode) {
    case 1: return 0x00; // all OFF
    case 2: return 0x0F; // all ON
    case 0:
    default:
      return prefs.getUChar("relay_mask", 0x00);
  }
}

void saveRelayMask(uint8_t mask) {
  prefs.putUChar("relay_mask", mask);
}

void loadConfig() {
  currentSlaveId = prefs.getUChar("slave_id", DEFAULT_SLAVE_ID);
  currentBaudCode = prefs.getUChar("baud_code", DEFAULT_BAUD_CODE);
  currentBootMode = prefs.getUChar("boot_mode", DEFAULT_BOOT_MODE);

  if (!validSlaveId(currentSlaveId)) currentSlaveId = DEFAULT_SLAVE_ID;
  if (!validBaudCode(currentBaudCode)) currentBaudCode = DEFAULT_BAUD_CODE;
  if (!validBootMode(currentBootMode)) currentBootMode = DEFAULT_BOOT_MODE;
}

void saveConfig(uint8_t slaveId, uint8_t baudCode, uint8_t bootMode) {
  prefs.putUChar("slave_id", slaveId);
  prefs.putUChar("baud_code", baudCode);
  prefs.putUChar("boot_mode", bootMode);
}

void factoryResetConfig() {
  prefs.putUChar("slave_id", DEFAULT_SLAVE_ID);
  prefs.putUChar("baud_code", DEFAULT_BAUD_CODE);
  prefs.putUChar("boot_mode", DEFAULT_BOOT_MODE);
  prefs.putUChar("relay_mask", 0x00);
}

void updateDiagnosticRegisters() {
  mb.Ireg(100, currentSlaveId);
  mb.Ireg(101, currentBaudCode);
  mb.Ireg(102, currentBootMode);
  mb.Ireg(103, FW_VERSION);
}

void setupModbusMap() {
  // Relay commands: Hreg 0..3
  for (int i = 0; i < 4; i++) {
    mb.addHreg(i, 0);
  }

  // Relay states: Ireg 0..3
  for (int i = 0; i < 4; i++) {
    mb.addIreg(i, 0);
  }

  // Digital inputs: Ireg 10..13
  for (int i = 0; i < 4; i++) {
    mb.addIreg(10 + i, 0);
  }

  // Config: Hreg 100..103
  mb.addHreg(100, currentSlaveId);
  mb.addHreg(101, currentBaudCode);
  mb.addHreg(102, currentBootMode);
  mb.addHreg(103, 0);

  // Diagnostics: Ireg 100..103
  mb.addIreg(100, currentSlaveId);
  mb.addIreg(101, currentBaudCode);
  mb.addIreg(102, currentBootMode);
  mb.addIreg(103, FW_VERSION);
}

void handleConfigCommands() {
  uint16_t newSlaveId = mb.Hreg(100);
  uint16_t newBaudCode = mb.Hreg(101);
  uint16_t newBootMode = mb.Hreg(102);
  uint16_t applyCmd = mb.Hreg(103);

  // Keep config registers aligned to valid current values if invalid writes happen
  if (!validSlaveId(newSlaveId)) {
    mb.Hreg(100, currentSlaveId);
    newSlaveId = currentSlaveId;
  }

  if (!validBaudCode(newBaudCode)) {
    mb.Hreg(101, currentBaudCode);
    newBaudCode = currentBaudCode;
  }

  if (!validBootMode(newBootMode)) {
    mb.Hreg(102, currentBootMode);
    newBootMode = currentBootMode;
  }

  if (applyCmd == 1) {
    saveConfig((uint8_t)newSlaveId, (uint8_t)newBaudCode, (uint8_t)newBootMode);
    currentSlaveId = (uint8_t)newSlaveId;
    currentBaudCode = (uint8_t)newBaudCode;
    currentBootMode = (uint8_t)newBootMode;
    mb.Hreg(103, 0);
    updateDiagnosticRegisters();
    Serial.println("Configuration saved");
  }
  else if (applyCmd == 2) {
    saveConfig((uint8_t)newSlaveId, (uint8_t)newBaudCode, (uint8_t)newBootMode);
    mb.Hreg(103, 0);
    Serial.println("Configuration saved, rebooting...");
    delay(200);
    ESP.restart();
  }
  else if (applyCmd == 3) {
    factoryResetConfig();
    mb.Hreg(103, 0);
    Serial.println("Factory reset, rebooting...");
    delay(200);
    ESP.restart();
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("=== MODBUS X4 START ===");

  prefs.begin("relaycfg", false);
  loadConfig();

  for (int i = 0; i < 4; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], HIGH); // OFF, active LOW
  }

  for (int i = 0; i < 4; i++) {
    pinMode(inputPins[i], INPUT_PULLUP);
  }

  Serial2.begin(baudFromCode(currentBaudCode), SERIAL_8N1, RS485_RX, RS485_TX);

  mb.begin(&Serial2, RS485_DE, true);
  mb.slave(currentSlaveId);

  setupModbusMap();

  uint8_t bootMask = getBootRelayMask(currentBootMode);
  applyRelayMask(bootMask);
  lastRelayMask = bootMask;

  updateDiagnosticRegisters();

  Serial.print("Slave ID: ");
  Serial.println(currentSlaveId);
  Serial.print("Baud: ");
  Serial.println(baudFromCode(currentBaudCode));
  Serial.print("Boot mode: ");
  Serial.println(currentBootMode);
  Serial.print("Restored relay mask: ");
  Serial.println(bootMask, BIN);
}

void loop() {
  mb.task();

  // Relay control + persistence
  uint8_t currentMask = makeRelayMaskFromHregs();
  if (currentMask != lastRelayMask) {
    applyRelayMask(currentMask);
    saveRelayMask(currentMask);
    lastRelayMask = currentMask;
  }

  // Input update
  for (int i = 0; i < 4; i++) {
    bool active = (digitalRead(inputPins[i]) == LOW); // active LOW
    mb.Ireg(10 + i, active ? 1 : 0);
  }

  // Config/diagnostics
  handleConfigCommands();
  updateDiagnosticRegisters();
}