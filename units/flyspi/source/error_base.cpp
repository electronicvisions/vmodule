#include "error_base.h"

#include <sstream>
#include <errno.h>

extern "C" {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#include <libusb.h>
#pragma GCC diagnostic pop
}

namespace flyspi {

	ErrorBase::ErrorBase() :
		std::runtime_error(""),
		m_where("")
	{}

	ErrorBase::~ErrorBase() throw()
	{}

	ErrorBase::ErrorBase(std::string const& where, std::string const& what) :
		runtime_error(what),
		m_where(where)
	{
	}

	std::string ErrorBase::where() const {
		return m_where;
	}

	static std::string format_device_error(std::string const& what, int error)
	{
		std::stringstream err;
		if (error != LIBUSB_SUCCESS)
		{
			err << libusb_error_name(error)
				<< " (" << error << ", errno=" << errno << "):\n    ";
		}
		err << what;
		if (errno == ENOMEM)
		{
			err << "\nHint: Check maximum DMA dingdsda stuff";
		}

		return err.str();
	}

	DeviceError::DeviceError(
			std::string const& where,
			std::string const& reason,
			int lib_usb_error_code) :
		ErrorBase(where, format_device_error(reason, lib_usb_error_code))
	{}
} // end namespace flyspi
