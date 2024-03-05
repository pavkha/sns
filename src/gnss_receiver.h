#pragma once

#include "application.h"
#include "gnss_data.h"
#include "observer.h"
#include "serial_port_linux_api.h"

#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

namespace periphery {

class gnss_receiver : public patterns::subject, public app::task {
  public:
    gnss_receiver(const std::string& port, int baud, const std::string& params = "8n1");
    gnss_receiver(const gnss_receiver&) = delete;
    gnss_receiver& operator=(const gnss_receiver&) = delete;
    gnss_receiver(gnss_receiver&&) = delete;
    gnss_receiver& operator=(gnss_receiver&&) = delete;
    ~gnss_receiver();

    bool init() override;
    void run() override;
    void deinit() override;

    void reset();

    std::string date() const;
    std::string utc_time() const;
    std::string latitude() const;
    std::string longitude() const;
    std::string altitude() const;
    std::string speed() const;
    std::string course() const;

    std::string nmea_msg_type() const;
    std::string raw_nmea_msg() const;

  private:
    std::thread serial_port_thread_{};
    std::promise<void> serial_port_thread_terminator_{};

    std::string serial_port_name_{};
    int serial_port_baud_{};
    std::string serial_port_params_{};
    hardware::serial_port_linux_api serial_port_{};

    std::queue<std::string> nmea_msg_queue_{};
    std::mutex nmea_msg_queue_mutex_{};
    std::condition_variable nmea_msg_queue_cv_{};

    static const int rx_array_size_{4096};
    char rx_array_[rx_array_size_];

    gnss_data gps_{};
    std::string current_nmea_msg_type_{};
    std::string raw_nmea_msg_{};
};

}    // namespace periphery
