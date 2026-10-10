/*
 * IMU sample packet sent to the host: 39 bytes, packed little-endian
 * (`<BQ3i3iiBB` in Python). See README.md.
 */
#ifndef IMU_PACKET_H
#define IMU_PACKET_H

#include <stddef.h>
#include <stdint.h>

#define IMU_PACKET_SYNC 0xAA

typedef struct __attribute__((packed)) {
  uint8_t  sync;          // IMU_PACKET_SYNC
  uint64_t timestamp_us;  // Unix µs once GPS-locked, otherwise µs since boot
  int32_t  rate[3];       // X, Y, Z; 1600 LSB per °/s
  int32_t  acc[3];        // X, Y, Z; 3200 LSB per m/s²
  int32_t  temp;          // raw sensor value
  uint8_t  gps_status;    // GPS_* flags (gps_time.h)
  uint8_t  crc;           // CRC-8 (poly 0x07, init 0) over the bytes before it
} imu_pkt_t;

_Static_assert(sizeof(imu_pkt_t) == 39, "imu_pkt_t must be 39 bytes");

void    imu_packet_init(void);                          // builds the CRC table
uint8_t imu_crc8(const uint8_t *data, size_t len);
void    imu_packet_seal(imu_pkt_t *pkt);                // sets sync and crc

#endif /* IMU_PACKET_H */
