#include "file_data_source.h"

bool file_data_source::open(const string& file_path)
{
	m_file.close();
	m_file.clear();
	m_file.open(file_path, ios::binary);

	return m_file.is_open();
}

bool file_data_source::is_open() const
{
	return m_file.is_open();
}

bool file_data_source::read(
	char* buffer,
	uint32_t buffer_size,
	uint32_t& bytes_read)
{
	bytes_read = 0;

	if (false == is_open() || nullptr == buffer || 0 == buffer_size)
	{
		return false;
	}

	m_file.read(buffer, static_cast<streamsize>(buffer_size));
	bytes_read = static_cast<uint32_t>(m_file.gcount());

	if (m_file.bad() || (m_file.fail() && false == m_file.eof()))
	{
		return false;
	}

	return true;
}

bool file_data_source::is_finished() const
{
	return m_file.eof();
}
