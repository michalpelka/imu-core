#include "nmea_parser.h"

#include <string.h>
#include <sys/time.h>

#include "gps_time.h"
#include "minmea.h"

static char     line[100];
static uint8_t  line_len;
static char     last_sentence[100];
static uint32_t n_rmc;

void nmea_parser_feed(char c)
{
  if (c == '$')
    line_len = 0;
  if (line_len < sizeof line - 1)
    line[line_len++] = c;
  if (c != '\n' || line_len <= 6)
    return;
  line[line_len] = '\0';
  memcpy(last_sentence, line, line_len + 1u);
  if (minmea_sentence_id(line) != MINMEA_SENTENCE_RMC)
    return;
  struct minmea_sentence_rmc rmc;
  if (!minmea_parse_rmc(&rmc, line) || !rmc.valid)
    return;
  struct timeval tv;
  if (minmea_gettimeofday(&tv, &rmc.date, &rmc.time) != 0)
    return;
  gps_time_set_nmea((uint64_t)tv.tv_sec * 1000000ULL + (uint64_t)tv.tv_usec);
  n_rmc++;
}

uint32_t nmea_parser_rmc_count(void) { return n_rmc; }
const char *nmea_parser_last_sentence(void) { return last_sentence; }
