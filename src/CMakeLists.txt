add_executable(gnss_tracker)

target_sources(gnss_tracker PRIVATE
    main.cpp
    observer.cpp
    gnss_receiver.cpp
    gnss_view.cpp
    console.cpp
    controller.cpp)

target_link_libraries(gnss_tracker hw::serial_port)