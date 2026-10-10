# imu-core: code shared by the SCH16T firmware projects.
#
# INTERFACE library: the sources compile as part of the consuming target, so
# they see its include paths. The project must provide:
#   config.h  SCH1 filter/sensitivity/decimation settings (+ NMEA_TIME_IS_NEXT_PULSE)
#   hw.h      SCH1 hardware layer (hw_SPI48_Send_Request, hw_EXTRESN_*, hw_delay)
#   main.h    CubeMX main header (CMSIS / HAL)
# and define timegm=mktime if its libc has no timegm (newlib).
#
#   add_subdirectory(common/imu-core)
#   target_link_libraries(${CMAKE_PROJECT_NAME} imu_core)

add_library(imu_core INTERFACE)
target_sources(imu_core INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/SCH1.c
    ${CMAKE_CURRENT_LIST_DIR}/minmea.c
    ${CMAKE_CURRENT_LIST_DIR}/gps_time.c
    ${CMAKE_CURRENT_LIST_DIR}/nmea_parser.c
    ${CMAKE_CURRENT_LIST_DIR}/imu_packet.c
)
target_include_directories(imu_core INTERFACE ${CMAKE_CURRENT_LIST_DIR})
