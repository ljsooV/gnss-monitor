#pragma once

#include <optional>
#include <string>

using namespace std;

struct gga_data
{
	string utc_time;
	bool has_fix;
	double latitude;
	double longitude;
	int fix_quality;
	int satellite_count;
	double hdop;
	double altitude;
};

struct rmc_data
{
	string utc_time;
	char status;
	bool has_fix;
	double latitude;
	double longitude;
	optional<double> speed_knots;
	optional<double> course_degrees;
	string date;
};

double convert_nmea_to_decimal_degrees(double coordinate, char direction);
bool parse_gga(gga_data& data, const string& line);
bool parse_rmc(rmc_data& data, const string& line);
bool is_checksum_valid(const string& line);
