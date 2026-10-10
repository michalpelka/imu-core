/*
 * GPS-disciplined timestamps from PPS + NMEA, independent of the board.
 *
 * The board provides a free-running 64-bit cycle count (e.g. the DWT cycle
 * counter extended in software) and reports:
 *   - gps_time_on_pps(cycles)      the cycle count of each PPS edge
 *   - gps_time_set_nmea(posix_us)  the time from each valid RMC
 *     (nmea_parser.c does this from raw bytes)
 *   - gps_time_tick_1ms()          every millisecond (SysTick)
 *
 * gps_time_to_us() then converts any cycle count to Unix time in µs, using the
 * CPU clock rate measured between pulses (crystal drift correction), and keeps
 * running at the last measured rate if PPS stops (holdover). Until the first
 * PPS it returns µs since cycle 0.
 *
 * Which second an RMC refers to depends on the source; set
 * NMEA_TIME_IS_NEXT_PULSE in the project's config.h:
 *   0 (default)  RMC names the PPS edge before it (u-blox receivers)
 *   1            RMC names the PPS edge after it (mandeye fake_pps)
 *
 * Needs the project's main.h for CMSIS (__get_PRIMASK etc.).
 */
#ifndef GPS_TIME_H
#define GPS_TIME_H

#include <stdint.h>

// gps_status bits
#define GPS_TIME_VALID  0x01  // locked to PPS at least once: timestamps are Unix time
#define GPS_PPS_OK      0x02  // last PPS accepted < 1.1 s ago; otherwise holding over
#define GPS_NMEA_OK     0x04  // valid RMC time received < 1 s ago
#define GPS_RATE_CAL    0x08  // CPU clock rate measured against PPS at least once

void     gps_time_init(uint32_t nominal_cycles_per_sec);
void     gps_time_on_pps(uint64_t cycles);
void     gps_time_set_nmea(uint64_t posix_us);
void     gps_time_tick_1ms(void);
uint64_t gps_time_to_us(uint64_t cycles);
uint8_t  gps_time_status(void);

// Diagnostics
uint32_t gps_time_pps_count(void);       // accepted pulses
uint32_t gps_time_pps_rejected(void);    // edges rejected (no recent RMC, or < 900 ms after the last)
uint32_t gps_time_cycles_per_sec(void);  // current measured rate

#endif /* GPS_TIME_H */
