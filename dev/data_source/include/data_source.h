#pragma once

#include <cstdint>

class data_source
{
public:
	virtual ~data_source() = default;

	virtual bool read(
		char* buffer,
		uint32_t buffer_size,
		uint32_t& bytes_read) = 0;

	virtual bool is_finished() const = 0;
};
