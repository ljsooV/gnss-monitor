#pragma once

#include <cstddef>
#include <string>

using namespace std;

class nmea_framer
{
public:
	void append(const char* data, size_t size);
	bool pop_line(string& line);

private:
	string m_pending_data;
};