#include "serial_data_source.h"

bool serial_data_source::open(const string& port_name, uint32_t baud_rate)
{
	return m_port.open(port_name, baud_rate);
}

bool serial_data_source::is_open() const
{
	return m_port.is_open();
}

void serial_data_source::close()
{
	m_port.close();
}

bool serial_data_source::read(
	char* buffer,
	uint32_t buffer_size,
	uint32_t& bytes_read)
{
	return m_port.read(buffer, buffer_size, bytes_read);
}

bool serial_data_source::is_finished() const
{
	return false;
}
