#include "nmea_framer.h"

void nmea_framer::append(const char* data, size_t size)
{
	m_pending_data.append(data, size);
}

bool nmea_framer::pop_line(string& line)
{
	const size_t pos = m_pending_data.find('\n');

	if (string::npos == pos)
	{
		return false;
	}

	line = m_pending_data.substr(0, pos);

	if (false == line.empty() && '\r' == line.back())
	{
		line.pop_back();
	}

	m_pending_data.erase(0, pos + 1);

	return true;
}
