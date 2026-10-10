#include "gps_time.h"

#include "config.h"
#include "main.h"  // CMSIS: __get_PRIMASK, __disable_irq, __set_PRIMASK

#ifndef NMEA_TIME_IS_NEXT_PULSE
#define NMEA_TIME_IS_NEXT_PULSE 0
#endif

static uint32_t nominal_cps;
static volatile uint64_t pps_posix_us;        // Unix time of the last accepted PPS
static volatile uint64_t pps_cycles;          // cycle count at the last accepted PPS
static volatile uint32_t pps_cycles_per_sec;  // measured CPU clock
static volatile uint64_t pending_posix_us;    // time from the newest RMC
static volatile uint8_t  gps_flags;           // GPS_TIME_VALID, GPS_RATE_CAL
static volatile uint32_t pps_holdoff_count_ms = 1000000;
static volatile uint32_t pps_nmea_age_ms = 1000000;
static volatile uint32_t pps_count;
static volatile uint32_t pps_rejected;

void gps_time_init(uint32_t nominal_cycles_per_sec)
{
  nominal_cps = nominal_cycles_per_sec;
  pps_cycles_per_sec = nominal_cycles_per_sec;
}

// Converts cycles since the last PPS to Unix time using the measured CPU clock
// rate. Split into whole seconds and remainder so it cannot overflow however
// long PPS is lost.
uint64_t gps_time_to_us(uint64_t cycles)
{
  uint32_t primask = __get_PRIMASK();
  __disable_irq();
  uint64_t base_cycles = pps_cycles;
  uint64_t base_us     = pps_posix_us;
  uint32_t cps         = pps_cycles_per_sec;
  __set_PRIMASK(primask);

  uint64_t elapsed = cycles - base_cycles;
  uint64_t secs    = elapsed / cps;
  uint64_t rem     = elapsed % cps;
  return base_us + secs * 1000000ULL + rem * 1000000ULL / cps;
}

uint8_t gps_time_status(void)
{
  uint8_t status = gps_flags;
  if ((status & GPS_TIME_VALID) && pps_holdoff_count_ms < 1100)
    status |= GPS_PPS_OK;
  if (pps_nmea_age_ms < 1000)
    status |= GPS_NMEA_OK;
  return status;
}

void gps_time_set_nmea(uint64_t posix_us)
{
  // PPS preempts the caller, so update both together or it could see a fresh
  // age with a stale or half-written 64-bit time.
  uint32_t primask = __get_PRIMASK();
  __disable_irq();
  pending_posix_us = posix_us;
  pps_nmea_age_ms  = 0;
  __set_PRIMASK(primask);
}

void gps_time_tick_1ms(void)
{
  pps_holdoff_count_ms++;
  pps_nmea_age_ms++;
}

void gps_time_on_pps(uint64_t cycles)
{
  // Accept a pulse only if there is a recent RMC and the previous pulse was
  // more than 900 ms ago (rejects glitches and double edges).
  if (pps_holdoff_count_ms <= 900 || pps_nmea_age_ms >= 999) {
    pps_rejected++;
    return;
  }

  // Round down: at NMEA rates above 1 Hz an RMC can be stamped .2/.4/...
  uint64_t rmc_sec = pending_posix_us / 1000000ULL;
#if NMEA_TIME_IS_NEXT_PULSE
  uint64_t posix_us = rmc_sec * 1000000ULL;
#else
  uint64_t posix_us = (rmc_sec + 1ULL) * 1000000ULL;
#endif

  // Measure the CPU clock against PPS when the previous pulse was exactly 1 s
  // earlier. Reject anything over 200 ppm off nominal (missed or false pulse)
  // and smooth by 1/8 to average out PPS jitter.
  if (posix_us - pps_posix_us == 1000000ULL) {
    uint64_t measured = cycles - pps_cycles;
    uint32_t limit    = nominal_cps / 5000U;
    if (measured > nominal_cps - limit && measured < nominal_cps + limit) {
      int32_t error = (int32_t)((uint32_t)measured - pps_cycles_per_sec);
      pps_cycles_per_sec += error / 8;
      gps_flags |= GPS_RATE_CAL;
    }
  }
  pps_cycles   = cycles;
  pps_posix_us = posix_us;
  pps_holdoff_count_ms = 0;
  gps_flags |= GPS_TIME_VALID;
  pps_count++;
}

uint32_t gps_time_pps_count(void) { return pps_count; }
uint32_t gps_time_pps_rejected(void) { return pps_rejected; }
uint32_t gps_time_cycles_per_sec(void) { return pps_cycles_per_sec; }
