#include <vector>
#include <stdexcept>

extern "C" {
#include <arpa/inet.h>
}

#include <gtest/gtest.h>

#include "vmodule.h"
#include "Vmemory.h"
#include "Vusbmaster.h"
#include "Vocpfifo.h"
#include "Vmux_board.h"
#include "Vmodule_adc.h"
#include "Vusbstatus.h"
#include "Vmoduleusb.h"


struct ADCHWTest : public ::testing::Test {
protected:
	ADCHWTest() :
		log(Logger::instance())
	{
		Logger::instance("vmodule.test.ADC", Logger::INFO, "");
	}

	void TearDown() {
		delete adc;
		adc = nullptr;
		delete mux_board;
		mux_board = nullptr;
		delete ocp;
		ocp = nullptr;
		delete mem;
		mem = nullptr;
		delete status;
		status = nullptr;
		delete usb;
		usb = nullptr;
		delete io;
		io = nullptr;
	}

	void create_vmodule_stuff() {
		// get some adc via usb vendor and product id
		io = new Vmoduleusb(usbcomm::note, Vflyspi_adc::idVendor, Vflyspi_adc::idProduct);
		usb = new Vusbmaster(io);
		status = new Vusbstatus(usb);
		mem = new Vmemory(usb);
		ocp = new Vocpfifo(usb);
		mux_board = new Vmux_board(ocp);
		adc = new Vflyspi_adc(ocp);
	}


	void SetUp() { ASSERT_NO_THROW(create_vmodule_stuff()); }


protected:
	Vflyspi_adc* adc = nullptr;

	// config adc and fpga and acquire data from fpga memory
	std::vector<Vflyspi_adc::adc_sample_t> config_and_sample(size_t const cnt_adc_samples,
	                                                         Vflyspi_adc::modes const mode) {
		uint32_t const startaddr = 0;
		uint32_t const endaddr = startaddr + cnt_adc_samples;

		log(Logger::DEBUG0) << "Num samples: " << cnt_adc_samples;

		adc->configure(mode);

		// manually trigger and sample TEST_SIZE samples from the adcs...
		adc->setup_controller(startaddr,
		                      endaddr,
		                      false /* single mode */,
		                      false /* trigger enable */,
		                      false /* trigger channel */);
		adc->manual_trigger();

		size_t const cnt_words_in_memory = (cnt_adc_samples + Vflyspi_adc::samples_per_word / 2)
			/ Vflyspi_adc::samples_per_word;

		// ECM: ugly type pointing to uint32_t... Unsoft terrory (i.e.
		// insta-eye-cancer), i.e. round(half of the number of adc samples to read)
		Vbufuint_p raw_data =
		    mem->readBlock(startaddr + Vflyspi_adc::mem_read_offset, cnt_words_in_memory);

		std::vector<Vflyspi_adc::adc_sample_t> samples;
		samples.reserve(cnt_adc_samples);

		// we have to iterate using [] (crude container implementation; we cannot
		// even check the size of it? HARRRD WTF)
		for (size_t i = 0; i < cnt_words_in_memory; i++) {
			// don't use abstruse big/little endian conversion of vmodule... do
			// it explicitly!
			Vflyspi_adc::sp6_word_t const tmp = ntohl(raw_data[i].get_raw());
			samples.push_back(tmp >> 16); // FIXME... not size-independent
			if (samples.size() >= cnt_adc_samples)
				break;
			samples.push_back(tmp);
		}

		return samples;
	}

private:
	Logger& log;
	Vmoduleusb* io = nullptr;
	Vusbmaster* usb = nullptr;
	Vusbstatus* status = nullptr;
	Vmemory* mem = nullptr;
	Vocpfifo* ocp = nullptr;
	Vmux_board* mux_board = nullptr;
};


/* Verifies the sampled data using the synthetic "test modes" of the ADC chip:
 *   * ZEROS
 *   * ONES
 *   * TOGGELING (0x555 and 0xAAA)
 *   * RAMP
 */
TEST_F(ADCHWTest, SampleSyntheticData) {
	size_t const TEST_SIZE = 1000 * 1000 + 1;

	{
		auto samples = config_and_sample(TEST_SIZE, Vflyspi_adc::ADC_ZEROS);
		std::vector<Vflyspi_adc::adc_sample_t> generated(TEST_SIZE, 0);
		ASSERT_EQ(generated, samples);
	}

	{
		auto samples = config_and_sample(TEST_SIZE, Vflyspi_adc::ADC_ONES);
		std::vector<Vflyspi_adc::adc_sample_t> generated(TEST_SIZE, Vflyspi_adc::adc_sample_mask);
		ASSERT_EQ(generated, samples);
	}

	{
		auto samples = config_and_sample(TEST_SIZE, Vflyspi_adc::ADC_TOGGLING);
		auto toggle_values = std::make_tuple(0xAAA, 0x555);
		ASSERT_TRUE(samples.size() > 1);
		ASSERT_NE(samples.at(0), samples.at(1));
		ASSERT_TRUE((samples[0] == std::get<0>(toggle_values)) ||
		            (samples[0] == std::get<1>(toggle_values)));
		ASSERT_TRUE((samples[0] != samples[1]) && ((samples[1] == std::get<0>(toggle_values)) ||
		                                           (samples[1] == std::get<1>(toggle_values))));
		std::vector<Vflyspi_adc::adc_sample_t> generated(TEST_SIZE, samples[0]);
		for (size_t i = 1; i < generated.size(); i += 2) {
			generated[i] = samples[1];
		}
		ASSERT_EQ(generated, samples);
	}

	{
		auto samples = config_and_sample(TEST_SIZE, Vflyspi_adc::ADC_RAMP);
		ASSERT_EQ(samples.size(), TEST_SIZE);
		Vflyspi_adc::adc_sample_t last_value = samples.at(0);

		for (size_t no_same = 0, i = 1; i < samples.size(); i++) {
			Vflyspi_adc::adc_sample_t value = samples.at(i);
			Vflyspi_adc::adc_sample_t next_valid_value = (last_value + 1) & Vflyspi_adc::adc_sample_mask;
			if (value == last_value) {
				no_same++;
				EXPECT_LE(no_same, 3) << " at position " << i << std::endl;
			} else {
				last_value = value;
				no_same = 0;
				EXPECT_EQ(next_valid_value, value) << " at position " << i << std::endl;
			}
		}
	}
}
