#pragma once

#include "csv_writer.h"
#include "data_source.h"
#include "nmea_framer.h"

#include <csignal>
#include <cstddef>
#include <string>

using namespace std;

class nmea_processor
{
public:
	explicit nmea_processor(csv_writer& writer);

	bool run(
		data_source& source,
		const volatile sig_atomic_t& stop_requested);

	size_t gga_count() const;
	size_t rmc_count() const;
	size_t no_fix_count() const;
	size_t checksum_error_count() const;
	size_t parse_error_count() const;

private:
	bool process_line(const string& line);

	csv_writer& m_writer;
	nmea_framer m_framer;
	size_t m_gga_count;
	size_t m_rmc_count;
	size_t m_no_fix_count;
	size_t m_checksum_error_count;
	size_t m_parse_error_count;
};
