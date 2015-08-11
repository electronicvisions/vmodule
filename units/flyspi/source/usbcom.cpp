/*
	usbcom class
	Johannes Schemmel 2011
*/

/* libusb test for bulk transfer

	log levels: 0 nothing
							1 error
							2 warning
							3 note
							4 data
*/
#include "usbcom.h"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <chrono>
extern "C" {
#include <sys/types.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#include <libusb.h>
#pragma GCC diagnostic pop
}
#include "error_base.h"
#include <boost/format.hpp>
#include "logger.h"
#include <string>
#include <time.h>

#include <log4cxx/logger.h>

static log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger("vmodule.usbcom");

const int CMD_SIZE = 12; //fpga command field size
using namespace std;

usbcomm::usbcomm(int loglevel) :
	timeout(10000)
{
	status = libusb_init(&context);
	mydev = NULL;
	buf = NULL;
	libusb_set_debug(context, loglevel);
}

std::string usbcomm::getSerial() {
	return std::string(serial);
}

unsigned int * usbcomm::getBuffer(size_t intsize) {
	size_t size=intsize*4;
	if(buf==NULL){
		if(size!=0){
			bufsize=size+CMD_SIZE;
			if(bufsize%512)bufsize=bufsize-bufsize%512+512; //round up to next 512 buffer
			LOG4CXX_DEBUG(logger, "Alloc new buf, size (bytes): " << bufsize);
			buf=new unsigned char[bufsize];
			datasize=size; //size of data field
			return (unsigned int*)	(buf+CMD_SIZE); //start of data area
		}else
			return NULL;
	}else {
		if(size==datasize)return (unsigned int*)(buf+CMD_SIZE);
		else{ //realloc buffer
			delete[] buf;
			if(size==0){
				buf=NULL;
				return NULL;
			}else {
				bufsize=size+CMD_SIZE;
				if(bufsize%512)bufsize=bufsize-bufsize%512+512; //round up to next 512 buffer
				buf=new unsigned char[bufsize];
				datasize=size; //size of data field
				return (unsigned int*)(buf+CMD_SIZE); //start of data area
			}
		}
	}
}

usbcomm::~usbcomm(void) {
	if(mydev) libusb_close(mydev);
	libusb_exit(context);
	if(buf) delete[] buf;
}

int usbcomm::selfrefresh(uint enablemask)
{
	uint data;
	data = enablemask &3;
	writestatus(data);
	readstatus(data);
	LOG4CXX_DEBUG(logger,"Statusreg:" << data);
	if((data & 0xc) != (enablemask<<2) )return -1; else return 0;
}

int usbcomm::open_first(int myvendor,int myproduct)
{
	vendor=myvendor;
	product=myproduct;
	// discover devices
	libusb_device **list;
	libusb_device *found = NULL;
	//ssize_t cnt = libusb_get_device_list(NULL, &list);
	ssize_t cnt = libusb_get_device_list(context, &list);
	ssize_t i = 0;

	status = 0;
	if (cnt < 0)
		return cnt;

	struct libusb_device_descriptor desc;
	for (i = 0; i < cnt; i++) {
		libusb_device *device = list[i];
		int r = libusb_get_device_descriptor(device, &desc);
		if (r < 0) {
			libusb_free_device_list(list, 1);
			throw flyspi::DeviceError(__PRETTY_FUNCTION__,
					"Getting USB device descriptor failed. "
					"Something must be really wrong.", r);
		}

		//compare device to target
		if (desc.idVendor==vendor && desc.idProduct==product) {
			found = device;
			status = libusb_open(found, &mydev);
			if(status!=0)
			{
				LOG4CXX_DEBUG(logger, "Found USB device matching vendor and "
					   "product, but it cannot be opened. LIBUSB_ERRO: " << status);
				continue;
			}

			status=libusb_claim_interface(mydev,0);
			if(status==LIBUSB_ERROR_BUSY)
			{
				libusb_close(mydev);mydev=NULL;
				continue; //try next device
			}

			{//debug messages
				//get serial number
				unsigned char buffer[BUFFERSIZE];
				size_t size = libusb_get_string_descriptor_ascii(mydev, desc.iSerialNumber, buffer, sizeof(buffer) );
				serial = std::string(reinterpret_cast<const char*>(buffer), size);
				LOG4CXX_DEBUG(logger, "Serial: " << serial);

				unsigned char	text[100];
				LOG4CXX_TRACE(logger, "bus " << libusb_get_bus_number(found)
						<< ", device " << libusb_get_device_address(found)
						<< " vendor " << desc.idVendor
						<< " product " << desc.idProduct
						<< " version " << desc.bcdDevice);
				libusb_get_string_descriptor_ascii(mydev,
						desc.iManufacturer, text, sizeof(text));
				LOG4CXX_TRACE(logger, "descriptor: " << text);
				libusb_get_string_descriptor_ascii(mydev,
						desc.iProduct, text, sizeof(text));
				LOG4CXX_TRACE(logger, "product: " << text);
			}

			break;
		}
	}

	libusb_free_device_list(list, 1);
	if(found==0)
		throw flyspi::DeviceError(__PRETTY_FUNCTION__,
			(boost::format("Opening USB device with vendor 0x%x and product 0x%x failed. No such device.")
			 % myvendor % myproduct).str(), LIBUSB_SUCCESS);
	return status;
}

int usbcomm::open(int myvendor,int myproduct,int myserial)
{
	std::stringstream tmp;
	tmp << myserial;
	return open(myvendor, myproduct, tmp.str());
}

int usbcomm::open(int myvendor,int myproduct,std::string myserial)
{
	vendor=myvendor;
	product=myproduct;
	// discover devices
	libusb_device **list;
	libusb_device *found = NULL;
	//ssize_t cnt = libusb_get_device_list(NULL, &list);
	ssize_t cnt = libusb_get_device_list(context, &list);
	ssize_t i = 0;

	status = 0;
	if (cnt < 0)
		return cnt;

	struct libusb_device_descriptor desc;
	for (i = 0; i < cnt; i++) {
		libusb_device *device = list[i];
		int r = libusb_get_device_descriptor(device, &desc);
		if (r < 0) {
			libusb_free_device_list(list, 1);
			throw flyspi::DeviceError(__PRETTY_FUNCTION__,
					"Getting USB device descriptor failed. "
					"Something must be really wrong.", r);
		}

		//compare device to target
		if (desc.idVendor==vendor && desc.idProduct==product ) {
			found = device;
			status = libusb_open(found, &mydev);
			if(status!=0)
			{
				found = NULL;
				LOG4CXX_DEBUG(logger, "Found USB device matching vendor and "
					   "product, but it cannot be opened. LIBUSB_ERRO: " << status);
				continue;
			}

			//get serial number
			unsigned char buffer[BUFFERSIZE];
			size_t size = libusb_get_string_descriptor_ascii(mydev, desc.iSerialNumber, buffer, sizeof(buffer) );
			serial = std::string(reinterpret_cast<const char*>(buffer), size);
			LOG4CXX_DEBUG(logger, "Found USB device matching vendor and "
					"product, serial number is " << serial);

			if (getSerial() != myserial)
			{
				LOG4CXX_DEBUG(logger, "Skipping this one.");
				libusb_close(mydev);
				mydev = NULL;
				found = NULL;
				continue; //try next device
			}

			status=libusb_claim_interface(mydev,0);
			if(status==LIBUSB_ERROR_BUSY)
			{
				libusb_close(mydev);
				found = NULL;
				mydev = NULL;
				libusb_free_device_list(list, 1);

				throw flyspi::DeviceError(__PRETTY_FUNCTION__,
					(boost::format("Claiming USB device with vendor 0x%x, product 0x%x and serial number %d failed. The interface seems to be in use.")
					 % myvendor % myproduct % myserial).str(),
					LIBUSB_ERROR_BUSY);
				//continue; //try next device
			}

			{//debug messages
				unsigned char	text[100];
				LOG4CXX_TRACE(logger, "bus " << libusb_get_bus_number(found)
						<< ", device " << libusb_get_device_address(found)
						<< " vendor " << desc.idVendor
						<< " product " << desc.idProduct
						<< " version " << desc.bcdDevice);
				libusb_get_string_descriptor_ascii(mydev,
						desc.iManufacturer, text, sizeof(text));
				LOG4CXX_TRACE(logger, "descriptor: " << text);
				libusb_get_string_descriptor_ascii(mydev,
						desc.iProduct, text, sizeof(text));
				LOG4CXX_TRACE(logger, "product: " << text);
			}

			break;
		}
	}

	libusb_free_device_list(list, 1);
	if(found==NULL) {
		throw flyspi::DeviceError(__PRETTY_FUNCTION__,
			(boost::format("Opening USB device with vendor 0x%x, product 0x%x and serial number %d failed. No such device or device has already been opened.")
			 % myvendor % myproduct % myserial).str(), LIBUSB_SUCCESS);
	}
	return status;
}

void usbcomm::pretty_print_buffer(std::string name, unsigned int size, unsigned char* buf)
{
	Logger& log = Logger::instance();
	unsigned int my_level = Logger::DEBUG1;
	if (log.getLevel() < my_level) return; // Fucking faster
	log(my_level) << name << " (" << size << " bytes):" << Logger::flush;
	log(my_level);
	for(unsigned int i=0;i<size;i++)
	{
		// cout stuff (ignore)
		if (i%16==0 && i>0)
		{
			log(my_level);
		}
		if (buf[i]<16) log << "0";
		log << hex << static_cast<unsigned int>(buf[i]);
		if ((i+1)%4==0 && (i+1)%16!=0) {
			log << " | ";
		}
		else {
			log << " ";
		}
	}
	log << Logger::flush;
	//for(unsigned int i=0;i<size;i++)
	//{
	//	if (i%16 == 0) cout << endl;
	//	if (buf[i] < 16) cout << 0;
	//	cout << hex << static_cast<unsigned int>(buf[i]);
	//}
}

static int timed_bulk_transfer(
		struct libusb_device_handle *dev_handle,
		unsigned char endpoint,
		unsigned char *data,
		int length,
		int *transferred,
		unsigned int timeout)
{
	using namespace std::chrono;
	typedef duration<double> ms;

	int status = 0;
	if (length > 512)
	{
		auto t0 = system_clock::now();
		status= libusb_bulk_transfer(dev_handle, endpoint, data, length, transferred, timeout);
		auto t1 = system_clock::now();
		LOG4CXX_DEBUG(logger, (endpoint == usbcomm::EP_OUT ? "Outgoing" : "Incoming")
				<< " bulk trans of "
				<< length << " bytes finished in "
				<< (duration_cast<ms>(t1-t0).count() / 1e3)
				<< "ms ("
				<< ((length/1048576.0) / (duration_cast<nanoseconds>(t1-t0).count() * 1e-9))
				<< "MB/s)");
	}
	else
	{
		status= libusb_bulk_transfer(dev_handle, endpoint, data, length, transferred, timeout);
		LOG4CXX_TRACE(logger, (endpoint == usbcomm::EP_OUT ? "Outgoing" : "Incoming")
				<< " bulk trans of "
				<< length << " bytes.")
	}
	return status;
}

int usbcomm::bulktrans(unsigned char * outbuf, unsigned char * inbuf,int out,int in)
{
	int actual_length;
	int statusIn = -999;
	int statusOut = -999;
	int actual_length_in = -1;
	int actual_length_out = -1;
	status= timed_bulk_transfer(mydev, EP_OUT, outbuf, out, &actual_length, timeout);
	statusOut = status;
	actual_length_out = actual_length;
	if (status== 0 && actual_length == out) {
		status= timed_bulk_transfer(mydev, EP_IN, inbuf, in, &actual_length, timeout);
		statusIn = status;
		actual_length_in = actual_length;
	}
	if (statusIn != 0 || statusOut != 0) {
		int charSize = 21;
		char timeChars[charSize];
		time_t t = time(0);
		strftime(timeChars, charSize, "%Y-%m-%d %H:%M:%S ", localtime(&t));

		ofstream fileDebug;
		fileDebug.open("vmod_debug.txt", std::ios_base::app);
		fileDebug << timeChars;
		fileDebug << "status_in " << statusIn;
		fileDebug << " status_out " << statusOut;
		fileDebug << " byte_in " << in;
		fileDebug << " byte_out " << out;
		fileDebug << " actual_length_in " << actual_length_in;
		fileDebug << " actual_length_out " << actual_length_out;
		fileDebug << " inbuf ";
		for(int i=0; i<32; i++){ //outbuf first 16 byte
			fileDebug << hex << static_cast<unsigned int>(inbuf[i]) << "|";
		}
		fileDebug << " outbuf ";
		for(int i=0; i<32; i++){ //outbuf first 16 byte
			fileDebug << hex << static_cast<unsigned int>(outbuf[i]) << "|";
		}
		fileDebug << std::endl;
		fileDebug.close();

		throw flyspi::DeviceError(__PRETTY_FUNCTION__,
				"submitting USB transfer failed", status);
	}

	//monitor data blocks
	pretty_print_buffer("Outbuf", out, outbuf);
	pretty_print_buffer("Inbuf", in, inbuf);

	return status;
}

int usbcomm::burstread(uint addr, size_t size){
	unsigned int *bp;
	size_t burstsize;

	bp=(unsigned int*)outbuf;
	bp[0]=__builtin_bswap32(READBURST);
	bp[1]=__builtin_bswap32(addr);
	bp[2]=__builtin_bswap32(size);

	burstsize=size*4+CMD_SIZE;
	if(burstsize%512)burstsize=burstsize-burstsize%512+512; //round up to next 512 buffer

	int actual_length;
	status= libusb_bulk_transfer( mydev, EP_OUT, outbuf, 512, &actual_length, timeout);
	if (status== 0 && actual_length == 512)
		status= libusb_bulk_transfer( mydev, EP_IN, buf, burstsize, &actual_length, timeout);
		if (status != 0) {
			throw flyspi::DeviceError(__PRETTY_FUNCTION__,
				"submitting USB transfer failed", status);
		}

	//monitor data blocks
	pretty_print_buffer("Outbuf", 16, outbuf);
	pretty_print_buffer("Inbuf", burstsize, buf);

	return status;
}
			
int usbcomm::burstwrite(uint addr, size_t size){
	unsigned int *bp;
	size_t burstsize;

	bp=(unsigned int*)buf;
	bp[0]=__builtin_bswap32(WRITEBURST);
	bp[1]=__builtin_bswap32(addr);
	bp[2]=__builtin_bswap32(size);

	burstsize=size*4+CMD_SIZE;
	if(burstsize%512)burstsize=burstsize-burstsize%512+512; //round up to next 512 buffer

	int actual_length;
	status= libusb_bulk_transfer( mydev, EP_OUT, buf, burstsize, &actual_length, timeout);
	if (status== 0 && actual_length == (int)burstsize)
		status= libusb_bulk_transfer( mydev, EP_IN, inbuf, 512, &actual_length, timeout);
		if (status != 0) {
			throw flyspi::DeviceError(__PRETTY_FUNCTION__,
				"submitting USB transfer failed", status);
		}

	//monitor data blocks
	pretty_print_buffer("Outbuf", burstsize, buf);
	pretty_print_buffer("Inbuf", 16, inbuf);

	return status;
}
			
int usbcomm::burstwriteocp(size_t size){
	unsigned int *bp;
	size_t burstsize;

	bp=(unsigned int*)buf;
	bp[0]=__builtin_bswap32(WRITEOCPBURST);
	bp[1]=__builtin_bswap32(0);
	bp[2]=__builtin_bswap32(size);

	burstsize=size*4+CMD_SIZE;
	if(burstsize%512)burstsize=burstsize-burstsize%512+512; //round up to next 512 buffer

	int actual_length;
	status= libusb_bulk_transfer( mydev, EP_OUT, buf, burstsize, &actual_length, timeout);
	if (status== 0 && actual_length == (int)burstsize)
		status= libusb_bulk_transfer( mydev, EP_IN, inbuf, 512, &actual_length, timeout);
		if (status != 0) {
			throw flyspi::DeviceError(__PRETTY_FUNCTION__,
				"submitting USB transfer failed", status);
		}

	//monitor data blocks
	pretty_print_buffer("Outbuf", burstsize, buf);
	pretty_print_buffer("Inbuf", 16, inbuf);

	return status;
}

			int usbcomm::write(uint addr, uint & data){
				unsigned int *bp;
				
				bp=(unsigned int*)outbuf;
				bp[0]=__builtin_bswap32(WRITE);
				bp[1]=__builtin_bswap32(addr);
				bp[2]=__builtin_bswap32(data);

				return bulktrans(outbuf,inbuf);
			}

			int usbcomm::read(uint addr, uint & data){
				unsigned int *bp;
				
				bp=(unsigned int*)outbuf;
				bp[0]=__builtin_bswap32(READBURST);
				bp[1]=__builtin_bswap32(addr);
				bp[2]=__builtin_bswap32(1);

				status= bulktrans(outbuf,inbuf);
				
				data=__builtin_bswap32( ((int*)inbuf)[3] );
	
				return status;
			}

			int usbcomm::writestatus(uint & data){
				unsigned int *bp;
				
				bp=(unsigned int*)outbuf;
				bp[0]=__builtin_bswap32(WRITESTATUS);
				bp[2]=__builtin_bswap32(data);

				return bulktrans(outbuf,inbuf);
			}

			int usbcomm::readstatus(uint & data){
				unsigned int *bp;
				
				bp=(unsigned int*)outbuf;
				bp[0]=__builtin_bswap32(READSTATUS);

				status= bulktrans(outbuf,inbuf);
				
				data=__builtin_bswap32( ((int*)inbuf)[2] );
	
				return status;
			}

			int usbcomm::raw(uint cmd, uint & addr, uint & data){
				unsigned int *bp;
				
				bp=(unsigned int*)outbuf;
				bp[0]=__builtin_bswap32(cmd);
				bp[1]=__builtin_bswap32(addr);
				bp[2]=__builtin_bswap32(data);

				status= bulktrans(outbuf,inbuf);
				addr=__builtin_bswap32( ((int*)inbuf)[1] );
				data=__builtin_bswap32( ((int*)inbuf)[2] );
				
				return status;
			}


int usbcomm::raw_out(uint cmd, uint & addr, uint & data)
{
	unsigned int *bp=(unsigned int*)outbuf;
	bp[0]=__builtin_bswap32(cmd);
	bp[1]=__builtin_bswap32(addr);
	bp[2]=__builtin_bswap32(data);
	int actual_length;

	status= libusb_bulk_transfer( mydev, EP_OUT, outbuf, 512, &actual_length, timeout);
	if(actual_length!=512)status|=0x1000;

	//monitor data blocks
	pretty_print_buffer("Outbuf", 16, outbuf);
	pretty_print_buffer("Inbuf", 512, inbuf);

	return status;
}


			int usbcomm::align(){//try to align ep_in fifo
				inbuf[0]=0xff; //clear any possible commands
				int actual_length;
				libusb_control_transfer( mydev, LIBUSB_REQUEST_TYPE_VENDOR, 0xa5, //reset fifos 
						0, 0, outbuf,0, 10);
				libusb_bulk_transfer( mydev, EP_IN, inbuf, 512, &actual_length, 10);
				libusb_bulk_transfer( mydev, EP_IN, inbuf, 512, &actual_length, 10);
				libusb_bulk_transfer( mydev, EP_IN, inbuf, 512, &actual_length, 10);
				libusb_bulk_transfer( mydev, EP_IN, inbuf, 512, &actual_length, 10);

				libusb_reset_device(mydev);
				return 0;
			}

void usbcomm::set_usb_transfer_timeout(size_t new_timeout)
{
	timeout = new_timeout;
}

// ****************** END USBCOMM *****************************
