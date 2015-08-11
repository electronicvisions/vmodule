/*
	usbcom header
	Johannes Schemmel 2011
*/
#ifndef USBCOMH
#define USBCOMH

#include <cstring> // size_t
#include <string>

// fwd decls (libusb.h isn't needed here)
struct libusb_context;
struct libusb_device_handle;

extern const int CMD_SIZE; //fpga command field size
/*
       * libusb test for bulk transfer

	log levels: 0 nothing
							1 error
							2 warning
							3 note
							4 data
*/


	class usbcomm
	{
		public:
			typedef unsigned int uint;

			enum loglevel {nothing=0,error,warning,note,data};
			void pretty_print_buffer(std::string name, unsigned int size, unsigned char* buf);
			libusb_context *context; //allow two instances of usbcomm in one process
			std::string serial;
			unsigned char *buf; //burst buffer pointer
			size_t bufsize; //burst buffer size
			size_t datasize; //data field in burst buffer
			unsigned char inbuf[512],outbuf[512];
			libusb_device_handle *mydev ;
			int vendor, product;
			enum endpoint {EP_OUT=2,EP_IN=0x86};
			enum command {NOP,READ,WRITE,READBURST,WRITEBURST,READSTATUS,WRITESTATUS,READOCP,WRITEOCP,READOCPFIFO,WRITEOCPBURST};
			struct libusb_device_handle *desc;
			int status;
			int align(void);
			int read(uint addr, uint & data);
			int write(uint addr, uint & data);
			int burstread(uint addr,  size_t size);
			int burstwrite(uint addr, size_t size);
			int burstwriteocp(size_t size);
			int selfrefresh(uint enablemask);
			int readstatus(uint & data);
			int writestatus(uint & data);
			int raw(uint cmd, uint & addr, uint & data); 
			int raw_out(uint cmd, uint & addr, uint & data); 
			std::string getSerial(void);
			//transfer buffer forth and back to sp6 board
			int bulktrans(unsigned char *out,unsigned char * in,int numout=512,int numin=512);
			unsigned int * getBuffer(size_t intsize=0);

			usbcomm(int loglevel);
			~usbcomm(void);
			int open_first(int vendor,int product);
			int open(int vendor,int product,int myserial);
			int open(int vendor, int product, std::string myserial);

			void set_usb_transfer_timeout(size_t timeout);

		private:
			static const size_t BUFFERSIZE = 128;
			size_t timeout; // timeout for usb transfers in ms [default 10000]
      };

#endif
