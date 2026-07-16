#pragma once

#include "nmea.h"

#include <fstream>
#include <string>

using namespace std;

class csv_writer
{
public:
	bool open(const string& output_directory);
	bool is_open() const;

	bool write(const gga_data& data);
	bool write(const rmc_data& data);
	void flush();

private:
	ofstream m_gga_file;
	ofstream m_rmc_file;
};
