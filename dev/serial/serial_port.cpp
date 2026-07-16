#include <Windows.h>

#include "serial_port.h"

bool serial_port::open(const string& port_name, uint32_t baud_rate)
{
	close();

	const string device_path = "\\\\.\\" + port_name;

	HANDLE handle = CreateFileA(
		device_path.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		0,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	);

	if (handle == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	m_handle = handle;

	if (false == configure_port(baud_rate) ||
		false == configure_timeouts())
	{
		close();

		return false;
	}

	return true;
}

void serial_port::close()
{
	if (nullptr == m_handle)
	{
		return;
	}

	CloseHandle(static_cast<HANDLE>(m_handle));
	m_handle = nullptr;
}

bool serial_port::is_open() const
{
	return nullptr != m_handle;
}

bool serial_port::configure_port(uint32_t baud_rate)
{
	DCB dcb{};
	dcb.DCBlength = sizeof(DCB);

	if (FALSE == GetCommState(static_cast<HANDLE>(m_handle), &dcb))
	{
		return false;
	}

	dcb.BaudRate = static_cast<DWORD>(baud_rate);
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;

	return FALSE != SetCommState(static_cast<HANDLE>(m_handle), &dcb);
}

bool serial_port::configure_timeouts()
{
	COMMTIMEOUTS timeouts{};

	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant = 1000;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 1000;

	return FALSE != SetCommTimeouts(
		static_cast<HANDLE>(m_handle),
		&timeouts
	);
}

bool serial_port::read(char* buffer, uint32_t buffer_size, uint32_t& bytes_read)
{
	bytes_read = 0;

	if (false == is_open() || nullptr == buffer || 0 == buffer_size)
	{
		return false;
	}

	DWORD received_size = 0;

	if (FALSE == ReadFile(
		static_cast<HANDLE>(m_handle),
		buffer,
		static_cast<DWORD>(buffer_size),
		&received_size,
		nullptr))
	{
		return false;
	}

	bytes_read = static_cast<uint32_t>(received_size);

	return true;
}
