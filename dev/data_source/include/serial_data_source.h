#pragma once

#include "data_source.h"
#include "serial_port.h"

#include <string>

using namespace std;

class serial_data_source : public data_source
{
public:
	bool open(const string& port_name, uint32_t baud_rate);
	bool is_open() const;
	void close();

	bool read(
		char* buffer,
		uint32_t buffer_size,
		uint32_t& bytes_read) override;

	bool is_finished() const override;

private:
	serial_port m_port;
};
