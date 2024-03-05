#include "controller.h"

#include <string>

controller::controller(console_input* subj,
                       periphery::gnss_receiver* rec,
                       gnss_console_view* view,
                       std::function<void(void)> help,
                       std::function<void(void)> quit)
    : subj_{subj}, receiver_{rec}, view_{view}, help_{help}, quit_{quit} {
    subj_->attach(this);
}

controller::~controller() {
    subj_->dettach(this);
}

bool controller::init() {
    if (receiver_ == nullptr || subj_ == nullptr || view_ == nullptr ||
        help_ == nullptr || quit_ == nullptr) {
        return false;
    }

    return true;
}

void controller::run() {
    if (!input_cmd_queue_.empty()) {
        std::string cmd{input_cmd_queue_.front()};

        if (cmd == "quit") {
            quit_();
        } else if (cmd == "help") {
            help_();
        } else if (cmd == "clear") {
            auto ret = system("clear");
            (void)ret;
        } else if (cmd == "raw on") {
            view_->raw_view(true);
        } else if (cmd == "raw off") {
            view_->raw_view(false);
        } else if (cmd == "reset") {
            receiver_->reset();
        }

        input_cmd_queue_.pop();
    }
}

void controller::deinit() {}

void controller::update(patterns::subject* s) {
    if (s == subj_) {
        input_cmd_queue_.push(subj_->input_str());
    }
}
