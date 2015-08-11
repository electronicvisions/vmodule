#include <libusb.h>
#include "logger.h"

using namespace std;

int main()
{
	int status = 0;
	Logger& log = Logger::instance("main", Logger::DEBUG2);
	int vendor=0x04b4;
	int product=0x1003;
	libusb_context *context; //allow two instances of usbcomm in one process
	libusb_device_handle *mydev ;
	// discover devices
	libusb_device **list;
	libusb_device *found = NULL;

	status = libusb_init(&context);
	mydev = NULL;
	libusb_set_debug(context, 3);

	ssize_t cnt = libusb_get_device_list(context, &list);
	ssize_t i = 0;

	log(Logger::INFO) << "Found " << cnt << " devices";
	if (cnt < 0)
		return cnt;

	struct libusb_device_descriptor desc;
	for (i = 0; i < cnt; i++) {
		libusb_device *device = list[i];
		int r = libusb_get_device_descriptor(device, &desc);
		if (r < 0) {
			log(Logger::ERROR) << "failed to get device descriptor";
			return r;
		}
		//compare device to target
		log(Logger::INFO) << "Found device " << hex << desc.idVendor << ", " << desc.idProduct;
		if (desc.idVendor==vendor && desc.idProduct==product) {
			found = device;
			log(Logger::INFO) << " Trying to open device";
			status = libusb_open(found, &mydev);
			if(status!=0)
				throw std::runtime_error("Error opening device");

			status=libusb_claim_interface(mydev,0);
			if(status==LIBUSB_ERROR_BUSY)
			{
				libusb_close(mydev);
				mydev=NULL;
				continue; //try next device
			}

			unsigned char	text[100];
			// get address, vendor and product id
			log(Logger::DEBUG1) << " bus " << libusb_get_bus_number(found) << ", device " << hex << static_cast<unsigned int>(libusb_get_device_address(found)) << " vendor " << desc.idVendor << " product " << desc.idProduct << " version " << desc.bcdDevice;

			// get manufacturer name
			libusb_get_string_descriptor_ascii(mydev, desc.iManufacturer, text, sizeof(text) );
			log(Logger::DEBUG1) << " " << text;

			// get product name
			libusb_get_string_descriptor_ascii(mydev, desc.iProduct, text, sizeof(text) );
			log(Logger::DEBUG1) << " " << text;

			//get serial number
			char const hex_chars[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
			const size_t serial_size = 16;
			unsigned char serial[serial_size];
			status = libusb_get_string_descriptor_ascii(mydev, desc.iSerialNumber, serial, sizeof(serial) );
			log(Logger::DEBUG1) << " Serial No. " << desc.iSerialNumber << endl;
			log(Logger::DEBUG1) << " Serial No. " << serial << " status " << status << "\n";
			for (size_t ii = 0; ii < serial_size; ++ii)
			{
				log << hex_chars[( serial[ii] & 0x0F )]
				    << hex_chars[( serial[ii] & 0xF0 ) >> 4]
					<< ( ( (ii+1)%16 == 0 ) ? "\n" : " ");
			}
			log(Logger::DEBUG1) << Logger::flush;

			break;
		}
	}

	libusb_free_device_list(list, 1);
	if(found==0)status = -1;

	if(mydev) libusb_close(mydev);
	libusb_exit(context);

	return status;
}
