#include "imu_packet.h"

// Table-driven CRC-8: the bitwise version took ~60 µs per packet on an F103.
static uint8_t crc8_table[256];

void imu_packet_init(void)
{
  for (int i = 0; i < 256; i++) {
    uint8_t crc = (uint8_t)i;
    for (int j = 0; j < 8; j++)
      crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x07) : (uint8_t)(crc << 1);
    crc8_table[i] = crc;
  }
}

uint8_t imu_crc8(const uint8_t *data, size_t len)
{
  uint8_t crc = 0;
  for (size_t i = 0; i < len; i++)
    crc = crc8_table[crc ^ data[i]];
  return crc;
}

void imu_packet_seal(imu_pkt_t *pkt)
{
  pkt->sync = IMU_PACKET_SYNC;
  pkt->crc  = imu_crc8((const uint8_t *)pkt, offsetof(imu_pkt_t, crc));
}
