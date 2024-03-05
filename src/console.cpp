#include "console.h"

console_input::console_input(bool echo, bool input_view)
    : echo_{echo}, input_view_{input_view} {}

console_input::~console_input() {
    if (!echo_) {
        tty_.c_lflag |= ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &tty_);
    }

    if (console_thread_.joinable()) {
        console_thread_terminator_.set_value();
        console_thread_.join();
    }
}

bool console_input::init() {
    console_thread_ = std::thread([this] {
        if (tcgetattr(STDIN_FILENO, &tty_) < 0) {
            // error_exit();
        }

        if (!echo_) {
            tty_.c_lflag &= ~static_cast<tcflag_t>(ECHO);
        }

        if (tcsetattr(STDIN_FILENO, TCSANOW, &tty_) < 0) {
            // error_exit();
        }

        fd_set read_fds, work_read_fds;

        std::string input_str{};

        timeval timeout, work_timeout;
        const int timeout_s{0};
        const int timeout_us{100'000};

        timeout.tv_sec = timeout_s;
        timeout.tv_usec = timeout_us;

        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        auto terminate = console_thread_terminator_.get_future();
        std::chrono::microseconds wait_time{0};

        do {
            work_read_fds = read_fds;
            work_timeout = timeout;

            int ret = select(STDIN_FILENO + 1, &work_read_fds, NULL, NULL, &work_timeout);

            if (ret == -1 && errno != EINTR) {
                // error_exit();
            }

            if (ret > 0) {
                if (FD_ISSET(STDIN_FILENO, &work_read_fds)) {
                    std::getline(std::cin, input_str);

                    if (!input_str.empty()) {
                        std::lock_guard<std::mutex> lg{input_str_queue_mutex_};
                        input_str_queue_.push(std::move(input_str));

                        input_str_queue_cv_.notify_one();
                    }
                }
            }
        } while (terminate.wait_for(std::chrono::microseconds(wait_time)) ==
                 std::future_status::timeout);
    });

    return true;
}

void console_input::run() {
    std::unique_lock<std::mutex> ul{input_str_queue_mutex_};
    std::chrono::microseconds wait_time{100'000};

    if (input_str_queue_cv_.wait_for(
                ul, wait_time, [this] { return !input_str_queue_.empty(); })) {
        while (!input_str_queue_.empty()) {
            input_str_ = input_str_queue_.front();

            if (input_view_) {
                std::cout << "input: " << input_str_ << '\n';
            }

            notify();

            input_str_queue_.pop();
        }
    }
}

void console_input::deinit() {}

std::string console_input::input_str() const {
    return input_str_;
}
