#pragma once

#include "application.h"
#include "observer.h"

#include <termios.h>
#include <unistd.h>

#include <condition_variable>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

class console_input : public patterns::subject, public app::task {
  public:
    console_input(bool echo, bool input_view);
    console_input(const console_input&) = delete;
    console_input& operator=(const console_input&) = delete;
    console_input(console_input&&) = delete;
    console_input& operator=(console_input&&) = delete;
    ~console_input();

    bool init() override;
    void run() override;
    void deinit() override;

    std::string input_str() const;

    enum {
        echo_on = 1,
        echo_off = 0,

        input_view_on = 1,
        input_view_off = 0
    };

  private:
    std::thread console_thread_{};
    std::promise<void> console_thread_terminator_{};

    termios tty_{};
    bool echo_{false};

    bool input_view_{false};

    std::queue<std::string> input_str_queue_{};
    std::mutex input_str_queue_mutex_{};
    std::condition_variable input_str_queue_cv_{};
    std::string input_str_{};
};
