#include "gnss_receiver.h"

#include <chrono>
#include <iostream>
#include <sstream>

namespace periphery {

gnss_receiver::gnss_receiver(const std::string& port, int baud, const std::string& params)
    : serial_port_name_{port}, serial_port_baud_{baud}, serial_port_params_{params} {}

gnss_receiver::~gnss_receiver() {
    if (serial_port_thread_.joinable()) {
        serial_port_thread_terminator_.set_value();
        serial_port_thread_.join();
    }
}

bool gnss_receiver::init() {
    serial_port_.set_port_name(serial_port_name_.c_str());
    if (serial_port_.open(serial_port_baud_, serial_port_params_.c_str()) == -1) {
        return false;
    }

    serial_port_thread_ = std::thread([this] {
        auto terminate = serial_port_thread_terminator_.get_future();
        std::chrono::microseconds wait_time{100'000};

        std::string nmea_msg{};

        do {
            if (int n{}; (n = serial_port_.available()) > 0) {
                serial_port_.read(rx_array_,
                                  static_cast<std::size_t>(
                                          (n <= rx_array_size_) ? n : rx_array_size_));

                for (int i = 0; i < n; ++i) {
                    if (rx_array_[i] == '$') {
                        nmea_msg.clear();
                    }

                    nmea_msg.push_back(rx_array_[i]);

                    if (rx_array_[i] == '\n') {
                        if (nmea_msg[0] == '$') {
                            {
                                std::lock_guard<std::mutex> lg{nmea_msg_queue_mutex_};
                                nmea_msg_queue_.push(std::move(nmea_msg));
                            }

                            nmea_msg_queue_cv_.notify_one();
                        }
                    }
                }
            }
        } while (terminate.wait_for(std::chrono::microseconds(wait_time)) ==
                 std::future_status::timeout);
    });

    return true;
}

void gnss_receiver::run() {
    std::unique_lock<std::mutex> ul{nmea_msg_queue_mutex_};
    std::chrono::microseconds wait_time{0};

    if (nmea_msg_queue_cv_.wait_for(
                ul, wait_time, [this] { return !nmea_msg_queue_.empty(); })) {
        while (!nmea_msg_queue_.empty()) {
            std::string nmea_msg{nmea_msg_queue_.front()};
            raw_nmea_msg_ = nmea_msg;
            current_nmea_msg_type_ = "";

            const auto star_pos = nmea_msg.rfind("*");

            if (star_pos != std::string::npos) {
                unsigned char calc_checksum{0};

                std::for_each(nmea_msg.begin() + 1,
                              nmea_msg.begin() + static_cast<int>(star_pos),
                              [&calc_checksum](char c) {
                                  calc_checksum ^= static_cast<unsigned char>(c);
                              });

                const char exp_checksum_str[]{
                        nmea_msg[star_pos + 1], nmea_msg[star_pos + 2], 0};
                unsigned char exp_checksum = static_cast<unsigned char>(
                        std::stoul(exp_checksum_str, nullptr, 16));

                if (calc_checksum == exp_checksum) {
                    std::vector<std::string> nmea_msg_tokens{};
                    std::string nmea_msg_token{};

                    nmea_msg = std::string{nmea_msg.substr(1, star_pos - 1)};
                    std::istringstream nmea_msg_stream{nmea_msg};

                    while (std::getline(nmea_msg_stream, nmea_msg_token, ',')) {
                        nmea_msg_tokens.push_back(nmea_msg_token);
                    }

                    current_nmea_msg_type_ = nmea_msg_tokens[0];

                    if (current_nmea_msg_type_ == "GPRMC") {
                        gps_.date = nmea_msg_tokens[9];
                        gps_.utc_time = nmea_msg_tokens[1];
                        gps_.latitude = nmea_msg_tokens[4] + nmea_msg_tokens[3];
                        gps_.longitude = nmea_msg_tokens[6] + nmea_msg_tokens[5];
                        gps_.speed = nmea_msg_tokens[7];
                        gps_.course = nmea_msg_tokens[8];
                        gps_.data_valid = nmea_msg_tokens[2] == "A" ? true : false;
                    } else if (current_nmea_msg_type_ == "GPGGA") {
                        gps_.altitude = nmea_msg_tokens[9];
                    }
                }
            }

            nmea_msg_queue_.pop();

            notify();
        }
    }
}

void gnss_receiver::deinit() {}

void gnss_receiver::reset() {
    if (serial_port_thread_.joinable()) {
        serial_port_thread_terminator_.set_value();
        serial_port_thread_.join();
    }

    serial_port_.close();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    serial_port_thread_terminator_ = std::promise<void>{};

    if (!init()) {
        throw std::runtime_error{"Reset tty error"};
    } else {
        std::cout << "Reset tty ok\n";
    }
}

std::string gnss_receiver::date() const {
    if (gps_.data_valid) {
        return gps_.date;
    }

    return std::string{};
}

std::string gnss_receiver::utc_time() const {
    if (gps_.data_valid) {
        return gps_.utc_time;
    }

    return std::string{};
}

std::string gnss_receiver::latitude() const {
    if (gps_.data_valid) {
        return gps_.latitude;
    }

    return std::string{};
}

std::string gnss_receiver::longitude() const {
    if (gps_.data_valid) {
        return gps_.longitude;
    }

    return std::string{};
}

std::string gnss_receiver::altitude() const {
    if (gps_.data_valid) {
        return gps_.altitude;
    }

    return std::string{};
}

std::string gnss_receiver::speed() const {
    if (gps_.data_valid) {
        return gps_.speed;
    }

    return std::string{};
}

std::string gnss_receiver::course() const {
    if (gps_.data_valid) {
        return gps_.course;
    }

    return std::string{};
}

std::string gnss_receiver::nmea_msg_type() const {
    return current_nmea_msg_type_;
}

std::string gnss_receiver::raw_nmea_msg() const {
    return raw_nmea_msg_;
}

}    // namespace periphery
