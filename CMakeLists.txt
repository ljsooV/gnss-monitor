cmake_minimum_required(VERSION 3.24)

project(gnss-monitor LANGUAGES CXX)

set(DEV_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/dev)

add_library(nmea STATIC
    ${DEV_ROOT}/nmea/nmea.cpp
    ${DEV_ROOT}/nmea/nmea_framer.cpp
)

target_include_directories(
    nmea
    PUBLIC
    ${DEV_ROOT}/nmea/include
)

target_compile_features(nmea PUBLIC cxx_std_20)

add_executable(gnss-monitor
    ${DEV_ROOT}/main.cpp
    ${DEV_ROOT}/data_source/file_data_source.cpp
    ${DEV_ROOT}/data_source/serial_data_source.cpp
    ${DEV_ROOT}/nmea/nmea_processor.cpp
    ${DEV_ROOT}/output/csv_writer.cpp
    ${DEV_ROOT}/serial/serial_port.cpp
)

target_include_directories(
    gnss-monitor
    PRIVATE
    ${DEV_ROOT}/data_source/include
    ${DEV_ROOT}/nmea/include
    ${DEV_ROOT}/output/include
    ${DEV_ROOT}/serial/include
)

target_link_libraries(gnss-monitor PRIVATE nmea)

target_compile_definitions(
    gnss-monitor
    PRIVATE
    GNSS_MONITOR_PROJECT_ROOT="${CMAKE_CURRENT_SOURCE_DIR}"
)

enable_testing()

add_executable(gnss-tests
    ${DEV_ROOT}/test_main.cpp
)

target_link_libraries(gnss-tests PRIVATE nmea)

set(GNSS_TEST_CASES
    coordinate_north
    coordinate_west
    checksum_valid
    checksum_mismatch
    checksum_malformed
    gga_valid
    gga_no_fix
    gga_missing_fields
    gga_invalid_direction
    gga_invalid_number
    rmc_valid
    rmc_no_fix
    framer_complete_line
    framer_lf_line
    framer_split_line
    framer_multiple_lines
    framer_complete_and_partial
)

foreach(test_case IN LISTS GNSS_TEST_CASES)
    add_test(NAME ${test_case} COMMAND gnss-tests ${test_case})
endforeach()

foreach(target nmea gnss-monitor gnss-tests)
    if(MSVC)
        target_compile_options(
            ${target}
            PRIVATE
            /W4
            /permissive-
            /source-charset:utf-8
            /execution-charset:.949
        )
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endforeach()
