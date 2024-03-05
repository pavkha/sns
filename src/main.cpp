#include "application.h"
#include "console.h"
#include "controller.h"
#include "gnss_receiver.h"
#include "gnss_view.h"
#include "serial_port_linux_api.h"

#include <signal.h>

#include <exception>
#include <iostream>
#include <queue>
#include <sstream>

volatile bool main_thread_terminate{false};

void signal_handler(int sig) {
    if (sig == SIGINT) {
        main_thread_terminate = true;
    }
}

void usage(const char* prog) {
    std::cout << "\nusage: " << prog << " -d /dev/ttyS0 -b 9600" << std::endl;
}

int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            usage(argv[0]);
            return 0;
        }

        std::string port_name{};
        int baudrate{};
        int opt{};

        while ((opt = getopt(argc, argv, "d:b:h")) != -1) {
            switch (opt) {
                case 'd':
                    port_name = optarg;
                    break;
                case 'b':
                    baudrate = atoi(optarg);
                    break;
                case 'h':
                default:
                    usage(argv[0]);
                    return 0;
            }
        }

        if (port_name.empty() || baudrate == 0) {
            usage(argv[0]);
            return 0;
        }

        signal(SIGINT, signal_handler);

        periphery::gnss_receiver gnss{port_name, baudrate};
        gnss_console_view gnss_view{&gnss};
        console_input input_cmd{console_input::echo_off, console_input::input_view_on};
        controller controll{&input_cmd,
                            &gnss,
                            &gnss_view,
                            [] {
                                std::cout << "\navailable commands: \n"
                                             "\thelp\t - this help\n"
                                             "\treset\t - reset tty\n"
                                             "\tclear\t - clear console\n"
                                             "\traw on\t - on raw nmea message view\n"
                                             "\traw off\t - off raw nmea message view\n"
                                             "\n"
                                             "\tquit\t - quit\n"
                                             "\tctrl+c\t - quit\n"
                                          << std::endl;
                            },
                            [] { main_thread_terminate = true; }};

        if (!input_cmd.init()) {
            throw std::runtime_error{"Console input init Error"};
        }

        if (!gnss.init()) {
            throw std::runtime_error{"GNSS init Error"};
        }

        if (!controll.init()) {
            throw std::runtime_error{"Controll init Error"};
        }

        std::vector<app::task*> tasks{&input_cmd, &controll, &gnss, &gnss_view};

        while (!main_thread_terminate) {
            for (auto task : tasks) {
                task->run();
            }
        }

        std::cout << "quit" << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}
