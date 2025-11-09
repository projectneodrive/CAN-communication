/*
  CAN_Sender.ino
  Works with MCP2515 + TJA1050 boards using MCP_CAN_lib (Cory J. Fowler)

  Wiring (Pro Micro example):
    MCP2515  ->  Pro Micro
    VCC      ->  5V
    GND      ->  GND
    CS       ->  D10        (change CS_PIN below if you wire differently)
    SI(MOSI) ->  16 (MOSI)
    SO(MISO) ->  14 (MISO)
    SCK      ->  15 (SCK)

  Wiring (Uno example):
    CS -> D10, MOSI->11, MISO->12, SCK->13

  Bus:
    Connect CANH↔CANH, CANL↔CANL between nodes.
    Ensure exactly two 120 Ω terminators on the entire bus (short the two J1 on the can converters)
*/

#include <SPI.h>
#include <mcp_can.h>

// --- CONFIG ---
const uint32_t CAN_BITRATE = CAN_500KBPS;
const int CS_PIN = 10;              // <-- change if your CS is on another pin
const uint32_t STD_ID = 0x012;      // 11-bit ID (the ID of the message) lower = more priority
const uint32_t EXT_ID = 0x00ABCDEF; // 29-bit ID
// ---------------

MCP_CAN CAN(CS_PIN); //initializes a MCP_CAN class, the CS_PIN is used to talk between the arduino and the CAN controller (ISP communication)

bool startCAN()
{
  // Try 8 MHz first (most blue MCP2515 boards use 8MHz crystals)
  if (CAN_OK == CAN.begin(MCP_STDEXT, CAN_BITRATE, MCP_8MHZ)) { //initializes the CAN communication, MCP_ANY means accepting standard and extended IDs but still applying filters
    return true;
  }
  // Fall back to 16 MHz
  if (CAN_OK == CAN.begin(MCP_STDEXT, CAN_BITRATE, MCP_16MHZ)) {
    return true;
  }
  return false;
}

void setup() {
  Serial.begin(115200); //everything that is serial is only useful for printing in the arduino serial monitor
  while (!Serial) { /* wait for USB */ }

  Serial.println(F("MCP2515 CAN Sender init..."));
  if (!startCAN()) {
    Serial.println(F("CAN init failed. Check wiring, CS pin, power, and crystal speed."));
    while (1) { delay(1000); }
  }

  // Normal mode (not loopback/sleep)
  CAN.setMode(MCP_NORMAL);

  // Optional: set all masks/filters to allow any ID, not strictly needed for the sender
  CAN.init_Mask(0, 0, 0);
  CAN.init_Mask(1, 0, 0);

  Serial.println(F("CAN init OK. Sending frames every second..."));
}

void loop() {
  // 1) Send a standard 11-bit frame with 5 ASCII bytes
  byte hello[] = { 'h','e','l','l','o' };
  //in order : CAN identifier message identifier, 0->standard frame type (11bits) 1->extended frame type (29 bits), size of message to send, pointer to data array
  byte rc = CAN.sendMsgBuf(STD_ID, 0 /*std*/, sizeof(hello), hello); //we put it in a variable to check the message did go through
  if (rc == CAN_OK) Serial.println(F("STD 0x012: 'hello' sent"));
  else              Serial.println(F("STD send failed"));

  delay(1000);

  // 2) Send an extended 29-bit frame with 5 ASCII bytes
  byte world[] = { 'w','o','r','l','d' };
  rc = CAN.sendMsgBuf(EXT_ID, 1 /*ext*/, sizeof(world), world);
  if (rc == CAN_OK) Serial.println(F("EXT 0x00ABCDEF: 'world' sent"));
  else              Serial.println(F("EXT send failed"));

  delay(1000);
}
