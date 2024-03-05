#pragma once

#include <string>

struct gnss_data {
    std::string date{};
    std::string utc_time{};
    std::string latitude{};
    std::string longitude{};
    std::string altitude{};
    std::string speed{};
    std::string course{};
    bool data_valid{false};
};
