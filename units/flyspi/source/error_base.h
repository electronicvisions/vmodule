#pragma once

#include <string>
#include <stdexcept>

namespace flyspi {

	class ErrorBase : public std::runtime_error {
		protected:
		std::string m_where;

		public:
		ErrorBase();
		~ErrorBase() throw();

		ErrorBase(std::string const& where,
				  std::string const& what);

		std::string where() const;
	};

	struct DeviceError : public ErrorBase {
		DeviceError(std::string const& where, std::string const& reason,
				    int lib_usb_error_code);
	};
}  /* namespace flyspi */

// vim: noexpandtab ts=4 sw=4 softtabstop=0 nosmarttab:
