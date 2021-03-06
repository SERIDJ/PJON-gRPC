#define PJON_INCLUDE_PACKET_ID true
#define TSA_RESPONSE_TIME_OUT 100000
#define PJON_INCLUDE_TSA true
#define PJON_INCLUDE_SWBB true
#include <PJON.h>

const uint8_t DEVICE_ID = 11;

PJON<ThroughSerialAsync> busA(DEVICE_ID);
PJON<SoftwareBitBang> busB(DEVICE_ID);

// All packets to devices listed here will be forwarded to bus B
const uint8_t device_id_ranges_on_B_side[] = {21, 50};


bool is_device_on_B_side(uint8_t device_id) {
  // Check if in one of the B ranges
  for (uint8_t i = 0; i < sizeof device_id_ranges_on_B_side - 1; i += 2)
    if (device_id_ranges_on_B_side[i] <= device_id && device_id <= device_id_ranges_on_B_side[i+1])
      return true;
  return false;
}

void receiver_functionB(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info) {
  // Forward packet to A segment of local bus
  if (!is_device_on_B_side(packet_info.receiver_id)) {
    busB.strategy.send_response(PJON_ACK);
    busA.send_from_id(
      packet_info.sender_id,
      packet_info.sender_bus_id,
      packet_info.receiver_id,
      packet_info.receiver_bus_id,
      (char *)payload,
      length,
      packet_info.header,
      packet_info.id,
      packet_info.port
    );
  }
}

void loop() {
  busB.receive();
  busA.update();
};

void setup() {
  Serial.begin(9600);

  // PRi
  busA.strategy.set_serial(&Serial);
  busA.set_synchronous_acknowledge(true);
  busA.set_crc_32(true);
  busA.set_router(true);
  busA.set_packet_id(true);
  busA.begin();

  // Devices via SoftwareBitBang
  busB.strategy.set_pin(12);
  busB.set_receiver(receiver_functionB);
  busB.set_synchronous_acknowledge(true);
  busB.set_crc_32(true);
  busB.set_router(true);
  busB.set_packet_id(true);
  busB.begin();
}
