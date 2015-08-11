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

#include "logging_ctrl.h"

int main (int argc, char *argv[]) {
	//configure logger
	Logger::instance("main", Logger::INFO, "");
	logger_default_config(log4cxx::Level::getInfo());
	static log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger("vmodule.readout_adc_spikey");

	std::string serial = "";
	uint boardVersion = 2;
	uint muxboardMode = 3;
    int mode = 0;
    int muxSelectInt = 8;
    Vmux_board::mux_input muxSelect = Vmux_board::MUX_GND;

    //evaluate command line arguments
    if(argc < 3){
		throw std::runtime_error("provide [board version] and [serial number] as first two arguments");
    } else {
		boardVersion = atoi(argv[1]);
		serial = std::string(argv[2]);
		switch(boardVersion){
			case 1: muxboardMode = 1; break;
			case 2: muxboardMode = 3; break;
			default:
				string msg = "Invalid spikey board version!";
				throw std::runtime_error(msg);
		}
		LOG4CXX_INFO(logger, "board version: " << boardVersion << "; serial: " << serial << "; mode: " << mode << "; signal: " << muxSelectInt);
	}
    if(argc == 4){
        mode = atoi(argv[3]);
        assert(mode >= 0 && mode <= 7); //allowed input modes
    }
    if(argc == 5){
        //mode is 0
        muxSelectInt = atoi(argv[4]);
        assert(muxSelectInt >= 0 && muxSelectInt <= 8); //allowed mux configs
        switch(muxSelectInt){
            case 0: muxSelect = Vmux_board::OUTAMP_0; break;
            case 1: muxSelect = Vmux_board::OUTAMP_1; break;
            case 2: muxSelect = Vmux_board::OUTAMP_2; break;
            case 3: muxSelect = Vmux_board::OUTAMP_3; break;
            case 4: muxSelect = Vmux_board::OUTAMP_4; break;
            case 5: muxSelect = Vmux_board::OUTAMP_5; break;
            case 6: muxSelect = Vmux_board::OUTAMP_6; break;
            case 7: muxSelect = Vmux_board::OUTAMP_7; break;
            case 8: muxSelect = Vmux_board::MUX_GND; break;
            default: assert(false);
        }
    }
    if(argc > 5){
		throw std::runtime_error("too many arguments");
    }

    unsigned int adc_start_adr = 0x0;
    unsigned int sample_time_us = 1.0 * 1000.0 * 1000.0 / 10000.0; //1s at 10^4 acceleration

    //Logger& log = Logger::instance(5);

    Vmoduleusb io(usbcomm::note, 0x04b4, 0x1003, serial);
    //create sp6 class tree
    Vusbmaster usb(&io);
    //usbmaster knows three clients, must be in this order!!!
    Vusbstatus status(&usb);
    Vmemory mem(&usb);
    Vocpfifo ocp(&usb);
    //ocpfifo clients
    // spikey stuff
    Vmux_board mux_board(&ocp, muxboardMode);
    Vspikeyfastadc fadc(&ocp);

    mux_board.enable_power(); //turn power for Spikey board on
    usleep(100000.0); //wait until power on
    //std::cout << std::bitset<32>(muxSelect) << std::endl;
    mux_board.set_Mux(muxSelect);

    //adc_num_samples = 4 * 1024 * 1024 - 1; //MAX!!!
    //TP: TODO: investigate, should be 32Msamples
    size_t adc_num_samples = (size_t) (floor(float(sample_time_us) * 96.0 / 2.0 + 0.5) + 1); //ADC samples with 96MHz

    unsigned int endaddr = adc_start_adr + adc_num_samples;
    LOG4CXX_INFO(logger, "sampling duration: " << sample_time_us << " micro seconds");
    fadc.configure(mode); //configures ADC itself
    fadc.setup_controller(adc_start_adr, endaddr); // configures controller in FPGA

    //trigger sampling
    sp6data *buf = ocp.writeBlock(0,4);
    buf[0]=0x80003009;buf[1]=0x1;//start
    buf[2]=0x80003009;buf[3]=0x0;//start off
    ocp.doWB();

    //wait for sampling
    usleep(sample_time_us * 2.0);

    Vbufuint_p data = mem.readBlock(adc_start_adr + 0x08000000, adc_num_samples);

    //each 32-bit double word represents 2 sampling points
    std::vector<uint16_t> raw_data;
    for(size_t i = 0; i<adc_num_samples; i++){
        raw_data.push_back((data[i]>>16)&0xfff);
        raw_data.push_back(data[i]&0xfff);
    }

    ofstream file;
    file.open("delme.txt");

    //for(size_t i = raw_data.size(); i>0; i--){
    for(size_t i = 0; i<raw_data.size(); i++){
        file << raw_data[i] << std::endl;
    }

    file.close();

    size_t len = raw_data.size();
    LOG4CXX_INFO(logger, len << " samples acquired");
    assert(len == 2 * adc_num_samples);

    //check whether mean near ground
    if(mode == 0){
        // http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
        double sum = 0;
        double sum_sqr = 0;
        for(size_t i = 0; i<raw_data.size(); i++){
            sum += raw_data[i];
            sum_sqr += raw_data[i] * raw_data[i];
        }
        double mean = sum / raw_data.size();
        double var = (sum_sqr - (sum * sum / raw_data.size())) / (raw_data.size() - 1);
		LOG4CXX_INFO(logger, "mean / std of digital ADC value (approx. in V): " << mean << " / " << sqrt(var)
			<< " (" << -(mean - 3235) / 2.83 << "mV)");
        if(muxSelectInt == 8){
            assert(mean > 2500);
            assert(mean < 3500);
            LOG4CXX_INFO(logger, "ground successfully measured (mean in digital value: " << mean << ")!");
        }

    //check whether ramp measured (monotonically increasing ADC values)
    } else if(mode == 4){
        int count = 0;
        for(size_t i = 1; i<raw_data.size(); i++){
            if(raw_data[i] != 0){
                count += 1;
                assert(raw_data[i] >= raw_data[i - 1]);
            }
        }
        assert(count > 0);
        LOG4CXX_INFO(logger, "ramp successfully measured!");
    } else {
		throw std::runtime_error("invalid mode!");
    }
}
