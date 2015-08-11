#include "vmodule.h"
#include "Vmemory.h"
#include "Vusbmaster.h"
#include "Vocpfifo.h"
#include "Vmux_board.h"
#include "Vmodule_adc.h"
#include "Vmoduleusb.h"
#include "Vusbstatus.h"
#include "Vspikey.h"
#include <iostream>
#include <fstream>
#include <math.h>

int main (int argc, char *argv[]) {
	Logger::instance("main",Logger::DEBUG0,"");

    int serial_1 = -1;
    int serial_2 = -1;

    //evaluate command line arguments
    if(argc < 2){
        throw std::runtime_error("provide serial number as first argument");
    } else serial_1 = atoi(argv[1]);
    if(argc > 2){
        serial_2 = atoi(argv[2]);
    }

    if(serial_2 != -1) {
        Vmoduleusb io_1(usbcomm::note, 0x04b4, 0x1003, serial_1);
        Vmoduleusb io_2(usbcomm::note, 0x04b4, 0x1003, serial_2);

        std::cout << "opened boards with serial " << serial_1 << " and " << serial_2 << std::endl;
    }

    uint i = 0;
    while(true) {
        i++;
        std::cout << "opening board with serial " << serial_1 << " - " << i << std::endl;

        Vmoduleusb io(usbcomm::note, 0x04b4, 0x1003, serial_1);
        std::cout << "succeeded opening board with serial " << io.getSerial() << std::endl;

        usleep(100000.0);
    }
}
