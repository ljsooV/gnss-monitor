#include "csv_writer.h"

#include <filesystem>
#include <iomanip>
#include <system_error>

namespace
{
	bool should_write_header(const filesystem::path& path, error_code& error)
	{
		error.clear();

		if (false == filesystem::exists(path, error))
		{
			return false == static_cast<bool>(error);
		}

		return 0 == filesystem::file_size(path, error) &&
			false == static_cast<bool>(error);
	}
}

bool csv_writer::open(const string& output_directory)
{
	m_gga_file.close();
	m_rmc_file.close();
	m_gga_file.clear();
	m_rmc_file.clear();

	const filesystem::path directory(output_directory);
	error_code error;
	filesystem::create_directories(directory, error);

	if (error)
	{
		return false;
	}

	const filesystem::path gga_path = directory / "gga.csv";
	const filesystem::path rmc_path = directory / "rmc.csv";
	const bool write_gga_header = should_write_header(gga_path, error);

	if (error)
	{
		return false;
	}

	const bool write_rmc_header = should_write_header(rmc_path, error);

	if (error)
	{
		return false;
	}

	m_gga_file.open(gga_path, ios::out | ios::app);
	m_rmc_file.open(rmc_path, ios::out | ios::app);

	if (false == is_open())
	{
		m_gga_file.close();
		m_rmc_file.close();
		return false;
	}

	if (write_gga_header)
	{
		m_gga_file << "utc_time,has_fix,latitude,longitude,fix_quality,satellite_count,hdop,altitude\n";
	}

	if (write_rmc_header)
	{
		m_rmc_file << "utc_time,status,has_fix,latitude,longitude,speed_knots,course_degrees,date\n";
	}

	flush();

	return m_gga_file.good() && m_rmc_file.good();
}

bool csv_writer::is_open() const
{
	return m_gga_file.is_open() && m_rmc_file.is_open();
}

bool csv_writer::write(const gga_data& data)
{
	if (false == is_open())
	{
		return false;
	}

	m_gga_file << data.utc_time << ',' << (data.has_fix ? 1 : 0) << ',';

	if (data.has_fix)
	{
		m_gga_file
			<< fixed << setprecision(8) << data.latitude << ','
			<< fixed << setprecision(8) << data.longitude << ',';
	}
	else
	{
		m_gga_file << ",,";
	}

	m_gga_file
		<< data.fix_quality << ','
		<< data.satellite_count << ','
		<< fixed << setprecision(2) << data.hdop << ',';

	if (data.has_fix)
	{
		m_gga_file << fixed << setprecision(3) << data.altitude;
	}

	m_gga_file << '\n';
	m_gga_file.flush();

	return m_gga_file.good();
}

bool csv_writer::write(const rmc_data& data)
{
	if (false == is_open())
	{
		return false;
	}

	m_rmc_file
		<< data.utc_time << ','
		<< data.status << ','
		<< (data.has_fix ? 1 : 0) << ',';

	if (data.has_fix)
	{
		m_rmc_file
			<< fixed << setprecision(8) << data.latitude << ','
			<< fixed << setprecision(8) << data.longitude << ','
			<< fixed << setprecision(3) << data.speed_knots << ','
			<< fixed << setprecision(3) << data.course_degrees << ',';
	}
	else
	{
		m_rmc_file << ",,,,";
	}

	m_rmc_file << data.date << '\n';
	m_rmc_file.flush();

	return m_rmc_file.good();
}

void csv_writer::flush()
{
	if (m_gga_file.is_open())
	{
		m_gga_file.flush();
	}

	if (m_rmc_file.is_open())
	{
		m_rmc_file.flush();
	}
}
