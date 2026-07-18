#include "nmea_processor.h"

#include "nmea.h"

#include <cstdint>
#include <iomanip>
#include <iostream>

nmea_processor::nmea_processor(csv_writer& writer)
	: m_writer(writer),
	m_gga_count(0),
	m_rmc_count(0),
	m_no_fix_count(0),
	m_checksum_error_count(0),
	m_parse_error_count(0)
{
}

bool nmea_processor::run(
	data_source& source,
	const volatile sig_atomic_t& stop_requested)
{
	char buffer[256]{};

	while (0 == stop_requested)
	{
		uint32_t bytes_read = 0;

		if (false == source.read(buffer, sizeof(buffer), bytes_read))
		{
			return false;
		}

		if (0 < bytes_read)
		{
			m_framer.append(buffer, bytes_read);

			string line;

			while (m_framer.pop_line(line))
			{
				cout << "[RAW] " << line << '\n';

				if (false == process_line(line))
				{
					return false;
				}
			}
		}

		if (source.is_finished())
		{
			break;
		}
	}

	return true;
}

size_t nmea_processor::gga_count() const
{
	return m_gga_count;
}

size_t nmea_processor::rmc_count() const
{
	return m_rmc_count;
}

size_t nmea_processor::no_fix_count() const
{
	return m_no_fix_count;
}

size_t nmea_processor::checksum_error_count() const
{
	return m_checksum_error_count;
}

size_t nmea_processor::parse_error_count() const
{
	return m_parse_error_count;
}

bool nmea_processor::process_line(const string& line)
{
	if (line.empty())
	{
		return true;
	}

	if (false == is_checksum_valid(line))
	{
		++m_checksum_error_count;
		cerr << "[CHECKSUM ERROR] " << line << '\n';
		return true;
	}

	if (string::npos != line.find("GGA"))
	{
		gga_data data{};

		if (false == parse_gga(data, line))
		{
			++m_parse_error_count;
			cerr << "[GGA PARSE ERROR] " << line << '\n';
			return true;
		}

		++m_gga_count;

		if (false == data.has_fix)
		{
			++m_no_fix_count;
			cout << "[GGA] No Fix, satellites=" << data.satellite_count
				<< ", hdop=" << data.hdop << '\n';
		}
		else
		{
			cout << fixed << setprecision(8)
				<< "[GGA] utc=" << data.utc_time
				<< ", lat=" << data.latitude
				<< ", lon=" << data.longitude
				<< ", satellites=" << data.satellite_count
				<< ", hdop=" << setprecision(2) << data.hdop
				<< ", altitude=" << setprecision(3) << data.altitude << '\n';
		}

		return m_writer.write(data);
	}

	if (string::npos != line.find("RMC"))
	{
		rmc_data data{};

		if (false == parse_rmc(data, line))
		{
			++m_parse_error_count;
			cerr << "[RMC PARSE ERROR] " << line << '\n';
			return true;
		}

		++m_rmc_count;

		if (false == data.has_fix)
		{
			++m_no_fix_count;
			cout << "[RMC] No Fix, status=" << data.status << '\n';
		}
		else
		{
			cout << fixed << setprecision(8)
				<< "[RMC] utc=" << data.utc_time
				<< ", lat=" << data.latitude
				<< ", lon=" << data.longitude
				<< ", speed_knots=";

			if (data.speed_knots.has_value())
			{
				cout << setprecision(3) << *data.speed_knots;
			}
			else
			{
				cout << "N/A";
			}

			cout << ", course=";

			if (data.course_degrees.has_value())
			{
				cout << setprecision(3) << *data.course_degrees;
			}
			else
			{
				cout << "N/A";
			}

			cout << '\n';
		}

		return m_writer.write(data);
	}

	return true;
}
