#pragma once

#include "application.h"
#include "console.h"
#include "gnss_receiver.h"
#include "gnss_view.h"
#include "observer.h"

#include <functional>
#include <queue>

class controller : public patterns::observer, public app::task {
  public:
    controller(console_input* subj,
               periphery::gnss_receiver* rec,
               gnss_console_view* view,
               std::function<void(void)> help,
               std::function<void(void)> quit);
    controller(const controller&) = delete;
    controller& operator=(const controller&) = delete;
    controller(controller&&) = delete;
    controller& operator=(controller&&) = delete;
    ~controller();

    bool init() override;
    void run() override;
    void deinit() override;

    void update(patterns::subject*) override;

  private:
    console_input* subj_{nullptr};
    periphery::gnss_receiver* receiver_{nullptr};
    gnss_console_view* view_{nullptr};

    std::function<void(void)> help_{};
    std::function<void(void)> quit_{};

    std::queue<std::string> input_cmd_queue_{};
};
