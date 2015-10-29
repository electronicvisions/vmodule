#include <chrono>
#include <mutex>
#include "logger.h"      // global logging class

class VmodTrafficLogger {

	std::mutex mtx;
	std::chrono::microseconds last_update;
	size_t effective_data_sum;
	size_t data_sum;
	size_t traffic_res = 10e3; //in ns
	log4cxx::LoggerPtr perflogger = log4cxx::Logger::getLogger("vmodule.usbcomPerf");

public:

	VmodTrafficLogger() :
		last_update(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch())),
		effective_data_sum(0),
		data_sum(0)
	{}

	~VmodTrafficLogger() {
		update(0, 0, true);
	}

	void update(size_t effective_data_size, size_t data_size, bool force_update = false) {
		std::chrono::microseconds now = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch());
		std::lock_guard<std::mutex> lock(mtx);
		if (now.count() - last_update.count() > traffic_res || force_update) {
			LOG4CXX_TRACE(perflogger, "\t" << last_update.count() << "\t" << effective_data_sum << "\t" << data_sum);
			last_update = now;
			data_sum = data_size;
			effective_data_sum = effective_data_size;
		} else {
			effective_data_sum += effective_data_size;
			data_sum += data_size;
		}
	}
};
