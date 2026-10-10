# imu-core

Code shared by Murata SCH16T firmware projects, e.g.
[AwesomeMurataIMU](https://github.com/michalpelka/AwesomeMurataIMU) (STM32F103 Blue Pill).

| File | What |
|---|---|
| `SCH1.c/h` | Murata SCH1 driver (BSD licence, Murata Electronics Oy), unchanged |
| `minmea.c/h` | NMEA parser by Kosma Moczek (WTFPL), unchanged |
| `imu_packet.c/h` | The 39-byte sample packet and its CRC-8 |
| `gps_time.c/h` | PPS + NMEA disciplined timestamps: drift correction, holdover, `gps_status` flags |
| `nmea_parser.c/h` | Bytes → NMEA sentences → RMC time → `gps_time` |

## Using it

Add as a submodule and link the INTERFACE library:

```sh
git submodule add https://github.com/michalpelka/imu-core.git common/imu-core
```

```cmake
add_subdirectory(common/imu-core)
target_link_libraries(${CMAKE_PROJECT_NAME} imu_core)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE timegm=mktime)  # newlib
```

The project provides `config.h` (SCH1 settings), `hw.h` + its implementation
(SPI and reset pin for SCH1) and the CubeMX `main.h`.

## Packet format

39 bytes, packed little-endian (`<BQ3i3iiBB` in Python):

| Field        | Type       | Notes                                                       |
|--------------|------------|-------------------------------------------------------------|
| sync         | `uint8`    | `0xAA`                                                      |
| timestamp_us | `uint64`   | Unix time in µs once GPS-locked, otherwise µs since boot    |
| rate[3]      | `int32` ×3 | X, Y, Z; 1600 LSB per °/s                                   |
| acc[3]       | `int32` ×3 | X, Y, Z; 3200 LSB per m/s²                                  |
| temp         | `int32`    | Raw sensor value                                            |
| gps_status   | `uint8`    | GPS sync flags, see below                                   |
| crc          | `uint8`    | CRC-8 (polynomial 0x07, init 0) over the first 38 bytes     |

`0xAA` can also appear inside a packet, so a reader should check the CRC and,
if it fails, resync from the next `0xAA`.

| Bit  | Name           | Meaning                                                          |
|------|----------------|------------------------------------------------------------------|
| 0x01 | GPS_TIME_VALID | Locked to PPS at least once: timestamps are Unix time            |
| 0x02 | GPS_PPS_OK     | Last PPS accepted less than 1.1 s ago; otherwise holding over    |
| 0x04 | GPS_NMEA_OK    | A valid RMC time arrived less than 1.1 s ago                     |
| 0x08 | GPS_RATE_CAL   | The CPU clock rate has been measured against PPS                 |

## Which second does an RMC name?

u-blox receivers send the RMC after the PPS edge it describes (default,
`NMEA_TIME_IS_NEXT_PULSE 0`). mandeye's `fake_pps` sends it ~110 ms after a
pulse but stamped with the *next* second (`NMEA_TIME_IS_NEXT_PULSE 1` in
`config.h`).
