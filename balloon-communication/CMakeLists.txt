cmake_minimum_required(VERSION 3.10)
project(wifibroadcast C)

set(CMAKE_C_STANDARD 11)

include_directories(.)
include_directories(udp)

add_executable(wifibroadcast
        udp/udp_client.c
        udp/udp_client.h
        channelscan.c
        check_alive.c
        fec.c
        fec.h
        ieee80211_radiotap.h
        lib.c
        lib.h
        radiotap.c
        radiotap.h
        rcrx.c
        rssi_forward.c
        rssi_forward_stdout.c
        rssi_write_shmem.c
        rssirx.c
        rssitx.c
        rx.c
        rx_status.c
        tracker.c
        tx.c
        wifibroadcast.h udp/rx_udp_util.h)

add_executable(udp_test_receiver
        udp/test/udp_test_receiver.c
        udp/test/udp_test_util.c
        udp/udp_client.c udp/rx_udp_util.c)

add_executable(udp_test_sender
        udp/test/udp_test_sender.c
        udp/test/udp_test_util.c
        udp/udp_client.c)
