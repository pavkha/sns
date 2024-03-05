#include "gnss_view.h"

#include <iostream>

gnss_console_view::gnss_console_view(periphery::gnss_receiver* s) : subj_{s} {
    subj_->attach(this);
}

gnss_console_view::~gnss_console_view() {
    subj_->dettach(this);
}

bool gnss_console_view::init() {
    return true;
}

void gnss_console_view::run() {
    if (data_changed_) {
        std::cout << "************ GPS ************\n";
        std::cout << "date:      " << data_.date << '\n';
        std::cout << "utc time:  " << data_.utc_time << '\n';
        std::cout << "latitude:  " << data_.latitude << '\n';
        std::cout << "longitude: " << data_.longitude << '\n';
        std::cout << "altitude:  " << data_.altitude << '\n';
        std::cout << "speed:     " << data_.speed << '\n';
        std::cout << "course:    " << data_.course << '\n';
        std::cout << "*****************************\n";

        data_changed_ = false;
    }
}

void gnss_console_view::deinit() {}

std::string gnss_console_view::convert_date(const std::string& date) const {
    if (date.size() >= 6) {
        return date.substr(0, 2) + '.' + date.substr(2, 2) + '.' + date.substr(4, 2);
    } else {
        return std::string{};
    }
}

std::string gnss_console_view::convert_time(const std::string& time) const {
    if (time.size() >= 6) {
        return time.substr(0, 2) + ':' + time.substr(2, 2) + ':' + time.substr(4, 2);
    } else {
        return std::string{};
    }
}

std::string gnss_console_view::convert_latitude(const std::string& lat) const {
    if (lat.size() > 5) {
        float latf = std::stof(lat.substr(1, 2)) +
                     std::stof(lat.substr(3, lat.size() - 3)) / 60;
        return std::to_string(latf) + ' ' + lat[0];
    } else {
        return std::string{};
    }
}

std::string gnss_console_view::convert_longitude(const std::string& lon) const {
    if (lon.size() > 5) {
        float lonf = std::stof(lon.substr(1, 3)) +
                     std::stof(lon.substr(4, lon.size() - 4)) / 60;
        return std::to_string(lonf) + ' ' + lon[0];
    } else {
        return std::string{};
    }
}

void gnss_console_view::update(patterns::subject* s) {
    if (s == subj_) {
        if (raw_view_on_) {
            std::cout << subj_->raw_nmea_msg();
        }

        if (subj_->nmea_msg_type() == "GPRMC") {
            std::string rmc_data = subj_->date();
            data_.date = convert_date(rmc_data);

            rmc_data = subj_->utc_time();
            data_.utc_time = convert_time(rmc_data);

            rmc_data = subj_->latitude();
            data_.latitude = convert_latitude(rmc_data);

            rmc_data = subj_->longitude();
            data_.longitude = convert_longitude(rmc_data);

            data_.altitude = subj_->altitude();
            data_.speed = subj_->speed();
            data_.course = subj_->course();

            data_changed_ = true;
        }
    }
}

void gnss_console_view::raw_view(bool action) {
    raw_view_on_ = action;
}
