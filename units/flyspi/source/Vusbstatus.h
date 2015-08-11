#include "vmsp6.h"
#include "vmodule.h"

//usb status class is simple and just reads/writes one word
class Vusbstatus:public Vmodule<sp6adr,sp6data> {
	public:
	//status reg bit definitions mask
	enum status_reg_m {c1_selfrefresh_enter=1,
						c3_selfrefresh_enter=2,
						c1_selfrefresh_mode=4,
						c3_selfrefresh_mode=8,
						calib_done=0x10,
						mem_error=0x20,
						usb_time_out=0x40,
						loopback=0x80
					};
	Vusbstatus(Vmodule<sp6adr,sp6data> *parent):Vmodule<sp6adr,sp6data>(parent){};
	//HAL functions
	uint getStatus();

	void setStatus(uint s);

	/// Return clock in MHz
	double getUsbClockFrequency();
};
