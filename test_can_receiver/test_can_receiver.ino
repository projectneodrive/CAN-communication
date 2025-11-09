/*
  CAN_Receiver.ino
  Prints any frames received on the bus.

  Wiring: same as sender (match your board’s SPI pins and CS).
*/

#include <SPI.h>
#include <mcp_can.h>

// --- CONFIG ---
const uint32_t CAN_BITRATE = CAN_500KBPS;
const int CS_PIN = 10;  // <-- set to your wiring
const byte INT_PIN = 2;    // MCP2515 INT → D2
// ---------------

MCP_CAN CAN(CS_PIN);

volatile bool canInterruptFlag = false;

bool startCAN()
{
  if (CAN_OK == CAN.begin(MCP_STDEXT, CAN_BITRATE, MCP_8MHZ)) return true;
  if (CAN_OK == CAN.begin(MCP_STDEXT, CAN_BITRATE, MCP_16MHZ)) return true;
  return false;
}


//ISR when the CAN controller gives an interrupt
void canISR() {
  canInterruptFlag = true;
}

void printFrame(unsigned long id, byte ext, byte rtr, byte len, const byte *buf)
{
  // ID
  if (ext) {
    Serial.print(F("EXT ID 0x"));
  } else {
    Serial.print(F("STD ID 0x"));
  }
  Serial.print(id, HEX);

  // RTR/Data
  if (rtr) Serial.print(F(" RTR"));
  Serial.print(F("  ["));
  Serial.print(len);
  Serial.print(F("]  "));

  // Data in HEX
  for (byte i = 0; i < len; i++) {
    if (buf[i] < 16) Serial.print('0');
    Serial.print(buf[i], HEX);
    Serial.print(' ');
  }

  // Separator
  Serial.print(F(" | "));

  // Data in ASCII (print '.' for non-printables)
  for (byte i = 0; i < len; i++) {
    char c = (char)buf[i];
    Serial.print(isPrintable(c) ? c : '.');
  }

  Serial.println();
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { /* wait */ }

  Serial.println(F("MCP2515 CAN Receiver init..."));
  if (!startCAN()) {
    Serial.println(F("CAN init failed. Check wiring, CS pin, power, and crystal speed."));
    while (1) { delay(1000); }
  }

  //enables interrupts
  pinMode(INT_PIN, INPUT_PULLUP); // INT is active-low
  attachInterrupt(digitalPinToInterrupt(INT_PIN), canISR, FALLING); //we call canISR on a falling edge from the interrupt

  Serial.println(F("Ready, using interrupts"));

  CAN.setMode(MCP_NORMAL);
  // Accept all IDs
  CAN.init_Mask(0, 0, 0);
  CAN.init_Mask(1, 0, 0);

  Serial.println(F("CAN init OK. Listening..."));
}

void loop() {
  if (canInterruptFlag) {
    canInterruptFlag = false;

    // There might be more than one frame queued
    while (CAN_MSGAVAIL == CAN.checkReceive()) {

      long unsigned int id;
      byte len;
      byte buf[8];
      byte ext = 0, rtr = 0;

      CAN.readMsgBuf(&id, &len, buf);

      // Prefer library flags if available; fallback heuristic:
      // isolates the correct id and ext and rtr
      #ifdef MCP2515_PUT_FLAGS_IN_ID
        ext = (id & 0x80000000UL) != 0;
        rtr = (id & 0x40000000UL) != 0;
        id &= 0x1FFFFFFFUL; // clear flag bits if they’re embedded
      #else
        ext = (id > 0x7FF); // heuristic
      #endif

      printFrame(id, ext, rtr, len, buf);
    }

    // you can do other stuff here; no busy waiting needed
  }
}
