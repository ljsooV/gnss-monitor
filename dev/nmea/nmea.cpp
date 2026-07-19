#include "nmea.h"

#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace
{
vector<string> split_fields(const string& line)
{
	vector<string> fields;
	size_t begin = 0;

	while (true)
	{
		const size_t end = line.find(',', begin);

		if (string::npos == end)
		{
			fields.push_back(line.substr(begin));

			break;
		}

		fields.push_back(line.substr(begin, end - begin));
		begin = end + 1;
	}

	return fields;
}
}

double convert_nmea_to_decimal_degrees(double coordinate, char direction)
{
	int degrees = static_cast<int>(coordinate / 100);
	double minutes = coordinate - (degrees * 100);

	if (direction == 'S' || direction == 'W')
	{
		return -(degrees + (minutes / 60));
	}

	return degrees + (minutes / 60);
}

bool parse_gga(gga_data& data, const string& line)
{
	data = {};
	const vector<string> fields = split_fields(line);

	if (10 > fields.size() ||
		fields[6].empty() ||
		fields[7].empty() ||
		fields[8].empty())
	{
		return false;
	}

	try
	{
		data.utc_time = fields[1];
		data.fix_quality = stoi(fields[6]);
		data.satellite_count = stoi(fields[7]);
		data.hdop = stod(fields[8]);

		if (0 == data.fix_quality)
		{
			data.has_fix = false;

			return true;
		}

		if (fields[1].empty() ||
			fields[2].empty() ||
			(fields[3] != "N" && fields[3] != "S") ||
			fields[4].empty() ||
			(fields[5] != "E" && fields[5] != "W") ||
			fields[9].empty())
		{
			return false;
		}

		data.has_fix = true;
		data.latitude = convert_nmea_to_decimal_degrees(stod(fields[2]), fields[3][0]);
		data.longitude = convert_nmea_to_decimal_degrees(stod(fields[4]), fields[5][0]);
		data.altitude = stod(fields[9]);
	}
	catch (const invalid_argument&)
	{
		return false;
	}
	catch (const out_of_range&)
	{
		return false;
	}

	return true;
}

bool parse_rmc(rmc_data& data, const string& line)
{
	data = {};
	const vector<string> fields = split_fields(line);

	if (10 > fields.size() ||
		(fields[2] != "A" && fields[2] != "V"))
	{
		return false;
	}

	data.utc_time = fields[1];
	data.status = fields[2][0];
	data.date = fields[9];

	if ('V' == data.status)
	{
		data.has_fix = false;

		return true;
	}

	if (fields[1].empty() ||
		fields[3].empty() ||
		(fields[4] != "N" && fields[4] != "S") ||
		fields[5].empty() ||
		(fields[6] != "E" && fields[6] != "W") ||
		fields[9].empty())
	{
		return false;
	}

	try
	{
		data.has_fix = true;
		data.latitude = convert_nmea_to_decimal_degrees(stod(fields[3]), fields[4][0]);
		data.longitude = convert_nmea_to_decimal_degrees(stod(fields[5]), fields[6][0]);

		if (false == fields[7].empty())
		{
			data.speed_knots = stod(fields[7]);
		}

		if (false == fields[8].empty())
		{
			data.course_degrees = stod(fields[8]);
		}
	}
	catch (const invalid_argument&)
	{
		return false;
	}
	catch (const out_of_range&)
	{
		return false;
	}

	return true;
}

bool is_checksum_valid(const string& line)
{
	unsigned char checksum = 0;

	if (string::npos == line.find('$') ||
		string::npos == line.find('*'))
	{
		return false;
	}

	for (size_t i = 0; i < line.size(); i++)
	{
		if (line[i] == '$')
		{
			continue;
		}

		if (line[i] == '*')
		{
			string check_str = line.substr(i + 1, 2);

			if (check_str.size() != 2)
			{
				return false;
			}

			try
			{
				size_t converted_count = 0;
				int check_val = stoi(check_str, &converted_count, 16);

				if (converted_count != check_str.size())
				{
					return false;
				}

				return static_cast<int>(checksum) == check_val;
			}
			catch (const invalid_argument&)
			{
				return false;
			}
			catch (const out_of_range&)
			{
				return false;
			}
		}

		checksum ^= line[i];
	}

	return false;
}
