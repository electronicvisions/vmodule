/*
       * libusb test for bulk transfer
*/
      
#include <gtest/gtest.h>
#include <stdio.h>
#include <sys/types.h>

#include <libusb.h>
#include "usbcom.h"

TEST(USBTestList, OpenFlyspiBoard)
      {

				usbcomm myusb(0);
			
				if(myusb.open_first(0x04b4,0x1003)){
					printf("open failed\n");ASSERT_EQ(0, myusb.status); return;;
				}else printf("opened.");

			}
			
