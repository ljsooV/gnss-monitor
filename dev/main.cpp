#include "csv_writer.h"
#include "data_source.h"
#include "file_data_source.h"
#include "nmea_processor.h"
#include "serial_data_source.h"

#include <csignal>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace std;

namespace
{
	volatile sig_atomic_t stop_requested = 0;

	void handle_stop_signal(int)
	{
		stop_requested = 1;
	}

	filesystem::path make_log_directory(const string& source_type)
	{
		const auto now = chrono::system_clock::now();
		const time_t current_time = chrono::system_clock::to_time_t(now);
		tm local_time{};

		if (0 != localtime_s(&local_time, &current_time))
		{
			throw runtime_error("failed to create log timestamp");
		}

		ostringstream timestamp;
		timestamp << put_time(&local_time, "%Y%m%d");

		filesystem::path directory = filesystem::path(GNSS_MONITOR_PROJECT_ROOT)
			/ "logs"
			/ timestamp.str()
			/ source_type;

		directory.make_preferred();

		return directory;
	}

	void print_usage()
	{
		cerr << "사용법:\n";
		cerr << "  gnss-monitor.exe serial <COM 포트> <baud rate>\n";
		cerr << "  gnss-monitor.exe file <NMEA 파일 경로>\n";
	}
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		print_usage();

		return 1;
	}

	unique_ptr<data_source> source;
	const string source_type = argv[1];

	if ("file" == source_type)
	{
		if (3 != argc)
		{
			print_usage();

			return 1;
		}

		auto file_source = make_unique<file_data_source>();

		if (false == file_source->open(argv[2]))
		{
			cerr << "파일을 열 수 없습니다: " << argv[2] << '\n';

			return 1;
		}

		source = move(file_source);
	}
	else if ("serial" == source_type)
	{
		if (4 != argc)
		{
			print_usage();

			return 1;
		}

		uint32_t baud_rate = 0;

		try
		{
			size_t parsed_size = 0;
			const unsigned long value = stoul(argv[3], &parsed_size);

			if ('\0' != argv[3][parsed_size] ||
				0 == value ||
				value > numeric_limits<uint32_t>::max())
			{
				throw invalid_argument("invalid baud rate");
			}

			baud_rate = static_cast<uint32_t>(value);
		}
		catch (const exception&)
		{
			cerr << "잘못된 baud rate입니다: " << argv[3] << '\n';

			return 1;
		}

		auto serial_source = make_unique<serial_data_source>();

		if (false == serial_source->open(argv[2], baud_rate))
		{
			cerr << argv[2] << " 열기 실패\n";

			return 1;
		}

		cout << argv[2] << " 열기 성공\n";

		source = move(serial_source);
	}
	else
	{
		print_usage();

		return 1;
	}

	csv_writer writer;
	const string log_directory = make_log_directory(source_type).string();

	if (false == writer.open(log_directory))
	{
		cerr << "CSV 로그 폴더를 열 수 없습니다: " << log_directory << '\n';

		return 1;
	}

	cout << "CSV 저장 경로: " << log_directory << '\n';

	signal(SIGINT, handle_stop_signal);

	nmea_processor processor(writer);
	const bool succeeded = processor.run(*source, stop_requested);

	writer.flush();
	source.reset();

	if (false == succeeded)
	{
		cerr << "입력 처리 또는 CSV 저장 실패\n";

		return 1;
	}

	if (0 != stop_requested)
	{
		cout << "종료 신호를 받아 정상 종료합니다.\n";
	}

	cout << "처리 통계: GGA=" << processor.gga_count()
		<< ", RMC=" << processor.rmc_count()
		<< ", No Fix=" << processor.no_fix_count()
		<< ", Checksum Error=" << processor.checksum_error_count()
		<< ", Parse Error=" << processor.parse_error_count() << '\n';

	return 0;
}
