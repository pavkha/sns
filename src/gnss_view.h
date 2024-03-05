#pragma once

#include "application.h"
#include "gnss_data.h"
#include "gnss_receiver.h"
#include "observer.h"

class gnss_console_view : public patterns::observer, public app::task {
  public:
    explicit gnss_console_view(periphery::gnss_receiver*);
    gnss_console_view(const gnss_console_view&) = delete;
    gnss_console_view& operator=(const gnss_console_view&) = delete;
    gnss_console_view(gnss_console_view&&) = delete;
    gnss_console_view& operator=(gnss_console_view&&) = delete;
    ~gnss_console_view();

    bool init() override;
    void run() override;
    void deinit() override;

    void update(patterns::subject*) override;

    void raw_view(bool action);

  private:
    std::string convert_date(const std::string& date) const;
    std::string convert_time(const std::string& time) const;
    std::string convert_latitude(const std::string& lat) const;
    std::string convert_longitude(const std::string& lon) const;

  private:
    periphery::gnss_receiver* subj_{nullptr};

    gnss_data data_{};
    bool data_changed_{false};
    bool raw_view_on_{false};
};
