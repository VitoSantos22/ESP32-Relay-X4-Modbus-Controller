#include <ModbusRTU.h>

ModbusRTU mb;

// RS485
#define RS485_RX 18
#define RS485_TX 19
#define RS485_DE 32

// Relay outputs (inverted)
const int relayPins[4] = {23, 5, 4, 13};

// Digital inputs (inverted)
const int inputPins[4] = {25, 26, 27, 33};

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("=== MODBUS X4 START ===");

  // Relays OFF at boot (logic inverted)
  for (int i = 0; i < 4; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], HIGH);
  }

  // Inputs
  for (int i = 0; i < 4; i++) {
    pinMode(inputPins[i], INPUT_PULLUP);
  }

  // RS485 UART
  Serial2.begin(9600, SERIAL_8N1, RS485_RX, RS485_TX);

  // txEnablePin = GPIO32
  mb.begin(&Serial2, RS485_DE, true);
  mb.slave(3);

  // Holding registers 0..3 = relay commands
  for (int i = 0; i < 4; i++) {
    mb.addHreg(i, 0);
  }

  // Input registers 0..3 = relay states
  for (int i = 0; i < 4; i++) {
    mb.addIreg(i, 0);
  }

  // Input registers 10..13 = digital inputs IN1..IN4
  for (int i = 0; i < 4; i++) {
    mb.addIreg(10 + i, 0);
  }

  Serial.println("Slave ID 3 ready @ 9600 8N1");
}

void loop() {
  mb.task();

  // Relay control + relay state mirror
  for (int i = 0; i < 4; i++) {
    bool on = (mb.Hreg(i) != 0);
    digitalWrite(relayPins[i], on ? LOW : HIGH);   // inverted
    mb.Ireg(i, on ? 1 : 0);
  }

  // Inputs
  for (int i = 0; i < 4; i++) {
    bool active = (digitalRead(inputPins[i]) == LOW);  // inverted
    mb.Ireg(10 + i, active ? 1 : 0);
  }
}