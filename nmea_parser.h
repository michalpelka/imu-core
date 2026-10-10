/*
 * Assembles NMEA sentences from bytes and passes each valid RMC time to
 * gps_time_set_nmea(). Feed it from a UART RX interrupt or a software UART.
 */
#ifndef NMEA_PARSER_H
#define NMEA_PARSER_H

#include <stdint.h>

void        nmea_parser_feed(char c);
uint32_t    nmea_parser_rmc_count(void);    // valid RMC sentences
const char *nmea_parser_last_sentence(void);

#endif /* NMEA_PARSER_H */
