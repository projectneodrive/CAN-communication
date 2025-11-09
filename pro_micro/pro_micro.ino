#include <SPI.h>
#include <mcp_can.h>

// --- CONFIG -------------------------------------------------
const uint32_t CAN_BITRATE = CAN_500KBPS;
const int      CS_PIN      = 10;          // MCP2515 CS
// If your MCP2515 is 16 MHz, change MCP_8MHZ -> MCP_16MHZ below.
// ------------------------------------------------------------

// MCP_CAN instance
MCP_CAN CAN(CS_PIN);

// CAN IDs
const uint32_t ID_CMD_TO_PROMICRO = 0x100;
const uint32_t ID_CMD_TO_UNO      = 0x101; // ignored
const uint32_t ID_CMD_WHO_ARE_YOU = 0x1FF;

// Replies
const uint32_t ID_REPLY_PROMICRO_WHO  = 0x200; // reply to 1FF
const uint32_t ID_REPLY_PROMICRO_CMD  = 0x210; // reply to 100

const uint8_t LED_PIN = LED_BUILTIN;

void setup()
{
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.begin(115200);
  unsigned long start = millis();
  while (!Serial && (millis() - start < 3000)) { ; }

  Serial.println(F("Pro Micro CAN node starting..."));

  if (CAN.begin(MCP_ANY, CAN_BITRATE, MCP_8MHZ) == CAN_OK) {
    Serial.println(F("CAN init OK"));
  } else {
    Serial.println(F("CAN init FAILED"));
    while (true) {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      delay(200);
    }
  }

  CAN.setMode(MCP_NORMAL);
  Serial.println(F("CAN in NORMAL mode, ready."));
}

void loop()
{
  unsigned long rxId;
  byte len = 0;
  byte ext = 0;
  byte data[8];

  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    byte rc = CAN.readMsgBuf(&rxId, &ext, &len, data);
    if (rc == CAN_OK) {
      handleFrame(rxId, len, data);
    } else {
      Serial.print(F("readMsgBuf error rc="));
      Serial.println(rc);
    }
  }
}

void handleFrame(unsigned long id, uint8_t len, uint8_t *data)
{
  switch (id) {
    case ID_CMD_TO_PROMICRO:
      Serial.print(F("Pro Micro got CMD (0x100), len="));
      Serial.print(len);
      Serial.print(F(" data: "));
      printData(len, data);
      Serial.println();

      digitalWrite(LED_PIN, !digitalRead(LED_PIN));

      // reply on the bus so you see it in candump
      sendCmdReply();
      break;

    case ID_CMD_TO_UNO:
      // not for us
      break;

    case ID_CMD_WHO_ARE_YOU:
      Serial.println(F("Pro Micro: got WHO_ARE_YOU (0x1FF), replying..."));
      sendIdentity();
      break;

    default:
      break;
  }
}

void sendIdentity()
{
  const char txt[] = "PROMICRO";   // 8 bytes
  byte buf[8];
  memcpy(buf, txt, 8);

  byte rc = CAN.sendMsgBuf(ID_REPLY_PROMICRO_WHO, 0 /*std*/, 8, buf);
  if (rc == CAN_OK) {
    Serial.println(F("Pro Micro: identity frame sent (0x200, 'PROMICRO')"));
  } else {
    Serial.print(F("Pro Micro: identity send failed, rc="));
    Serial.println(rc);
  }
}

void sendCmdReply()
{
  // Just say "P-CMD" when we get ID 0x100
  const char txt[] = "P-CMD";
  byte buf[5];
  memcpy(buf, txt, 5);

  byte rc = CAN.sendMsgBuf(ID_REPLY_PROMICRO_CMD, 0 /*std*/, 5, buf);
  if (rc == CAN_OK) {
    Serial.println(F("Pro Micro: CMD reply sent (0x210, 'P-CMD')"));
  } else {
    Serial.print(F("Pro Micro: CMD reply failed, rc="));
    Serial.println(rc);
  }
}

void printData(uint8_t len, uint8_t *data)
{
  for (uint8_t i = 0; i < len; i++) {
    Serial.print("0x");
    if (data[i] < 0x10) Serial.print('0');
    Serial.print(data[i], HEX);
    Serial.print(' ');
  }
}
