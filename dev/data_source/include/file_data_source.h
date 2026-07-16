#pragma once

#include "data_source.h"

#include <fstream>
#include <string>

using namespace std;

class file_data_source : public data_source
{
public:
	bool open(const string& file_path);
	bool is_open() const;

	bool read(
		char* buffer,
		uint32_t buffer_size,
		uint32_t& bytes_read) override;

	bool is_finished() const override;

private:
	ifstream m_file;
};
