#pragma once

#include <cstdint>
#include <string>

using namespace std;

class serial_port
{
public:
	serial_port() : m_handle(nullptr)
	{
	}

	~serial_port()
	{
		close();
	}

	serial_port(const serial_port&) = delete;
	serial_port& operator=(const serial_port&) = delete;

	bool open(const string& port_name, uint32_t baud_rate);
	bool is_open() const;
	void close();

	bool read(char* buffer, uint32_t buffer_size, uint32_t& bytes_read);

private:
	bool configure_port(uint32_t baud_rate);
	bool configure_timeouts();

	void* m_handle;
};
