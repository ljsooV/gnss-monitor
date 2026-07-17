#include "nmea.h"
#include "nmea_framer.h"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

namespace
{
constexpr double tolerance = 0.000001;

bool nearly_equal(double lhs, double rhs)
{
	return abs(lhs - rhs) < tolerance;
}

bool test_coordinate_north()
{
	return nearly_equal(convert_nmea_to_decimal_degrees(4807.038, 'N'), 48.1173);
}

bool test_coordinate_west()
{
	return nearly_equal(convert_nmea_to_decimal_degrees(1131.000, 'W'), -11.5166666667);
}

bool test_checksum_valid()
{
	return is_checksum_valid("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47");
}

bool test_checksum_mismatch()
{
	return false == is_checksum_valid("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*48");
}

bool test_checksum_malformed()
{
	return false == is_checksum_valid("$GPGGA,123519*ZZ") &&
		false == is_checksum_valid("$GPGGA,123519*1");
}

bool test_gga_valid()
{
	gga_data data{};
	const bool parsed = parse_gga(data, "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47");

	return parsed &&
		data.has_fix &&
		data.utc_time == "123519" &&
		nearly_equal(data.latitude, 48.1173) &&
		nearly_equal(data.longitude, 11.5166666667) &&
		data.fix_quality == 1 &&
		data.satellite_count == 8 &&
		nearly_equal(data.hdop, 0.9) &&
		nearly_equal(data.altitude, 545.4);
}

bool test_gga_no_fix()
{
	gga_data data{};
	const bool parsed = parse_gga(data, "$GPGGA,,,,,,0,00,99.99,,,,,,*48");

	return parsed &&
		false == data.has_fix &&
		data.fix_quality == 0 &&
		data.satellite_count == 0 &&
		nearly_equal(data.hdop, 99.99);
}

bool test_gga_missing_fields()
{
	gga_data data{};
	return false == parse_gga(data, "$GPGGA,123519,4807.038,N");
}

bool test_gga_invalid_direction()
{
	gga_data data{};
	return false == parse_gga(data, "$GPGGA,123519,4807.038,X,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47");
}

bool test_gga_invalid_number()
{
	gga_data data{};
	return false == parse_gga(data, "$GPGGA,123519,not-a-number,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47");
}

bool test_rmc_valid()
{
	rmc_data data{};
	const bool parsed = parse_rmc(data, "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A");

	return parsed &&
		data.has_fix &&
		data.utc_time == "123519" &&
		data.status == 'A' &&
		nearly_equal(data.latitude, 48.1173) &&
		nearly_equal(data.longitude, 11.5166666667) &&
		nearly_equal(data.speed_knots, 22.4) &&
		nearly_equal(data.course_degrees, 84.4) &&
		data.date == "230394";
}

bool test_rmc_no_fix()
{
	rmc_data data{};
	const bool parsed = parse_rmc(data, "$GPRMC,,V,,,,,,,,,,N*53");

	return parsed && false == data.has_fix && data.status == 'V';
}

bool test_framer_complete_line()
{
	nmea_framer framer;
	const string input = "$GPGGA,123*00\r\n";
	string line;

	framer.append(input.data(), input.size());

	return framer.pop_line(line) &&
		line == "$GPGGA,123*00" &&
		false == framer.pop_line(line);
}

bool test_framer_lf_line()
{
	nmea_framer framer;
	const string input = "$GPGGA,123*00\n";
	string line;

	framer.append(input.data(), input.size());

	return framer.pop_line(line) && line == "$GPGGA,123*00";
}

bool test_framer_split_line()
{
	nmea_framer framer;
	const string first = "$GPGGA,123";
	const string second = "*00\r\n";
	string line;

	framer.append(first.data(), first.size());

	if (framer.pop_line(line))
	{
		return false;
	}

	framer.append(second.data(), second.size());

	return framer.pop_line(line) &&
		line == "$GPGGA,123*00" &&
		false == framer.pop_line(line);
}

bool test_framer_multiple_lines()
{
	nmea_framer framer;
	const string input = "$GPGGA,123*00\r\n$GPRMC,456*00\r\n";
	string first_line;
	string second_line;
	string remaining_line;

	framer.append(input.data(), input.size());

	return framer.pop_line(first_line) &&
		first_line == "$GPGGA,123*00" &&
		framer.pop_line(second_line) &&
		second_line == "$GPRMC,456*00" &&
		false == framer.pop_line(remaining_line);
}

bool test_framer_complete_and_partial()
{
	nmea_framer framer;
	const string first = "$GPGGA,123*00\r\n$GPRMC,456";
	const string second = "*00\r\n";
	string line;

	framer.append(first.data(), first.size());

	if (false == framer.pop_line(line) || line != "$GPGGA,123*00")
	{
		return false;
	}

	if (framer.pop_line(line))
	{
		return false;
	}

	framer.append(second.data(), second.size());

	return framer.pop_line(line) &&
		line == "$GPRMC,456*00" &&
		false == framer.pop_line(line);
}

struct test_case
{
	string name;
	bool (*run)();
};
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		cerr << "테스트 이름이 필요합니다.\n";
		return 1;
	}

	const vector<test_case> tests = {
		{"coordinate_north", test_coordinate_north},
		{"coordinate_west", test_coordinate_west},
		{"checksum_valid", test_checksum_valid},
		{"checksum_mismatch", test_checksum_mismatch},
		{"checksum_malformed", test_checksum_malformed},
		{"gga_valid", test_gga_valid},
		{"gga_no_fix", test_gga_no_fix},
		{"gga_missing_fields", test_gga_missing_fields},
		{"gga_invalid_direction", test_gga_invalid_direction},
		{"gga_invalid_number", test_gga_invalid_number},
		{"rmc_valid", test_rmc_valid},
		{"rmc_no_fix", test_rmc_no_fix},
		{"framer_complete_line", test_framer_complete_line},
		{"framer_lf_line", test_framer_lf_line},
		{"framer_split_line", test_framer_split_line},
		{"framer_multiple_lines", test_framer_multiple_lines},
		{"framer_complete_and_partial", test_framer_complete_and_partial},
	};

	const string requested_test = argv[1];

	for (const test_case& test : tests)
	{
		if (test.name == requested_test)
		{
			if (test.run())
			{
				cout << "[PASS] " << test.name << '\n';
				return 0;
			}

			cerr << "[FAIL] " << test.name << '\n';
			return 1;
		}
	}

	cerr << "알 수 없는 테스트: " << requested_test << '\n';
	return 1;
}
