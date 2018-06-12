cmake_minimum_required(VERSION 3.0)
project(wifibroadcast C)

set (CMAKE_C_FLAGS "-std=c11 -D_GNU_SOURCE ${CMAKE_C_FLAGS}")

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
        rssi_forward.c
        rssilogger.c
        rssirx.c
        rssitx.c
        rx.c
        rx_rc_telemetry.c
        rx_rc_telemetry_buf.c
        rx_status.c
        sharedmem_init_rx.c
        sharedmem_init_tx.c
        syslogger.c
        tracker.c
        tx_measure.c
        tx_rawsock.c
        tx_telemetry.c
        wifibackgroundscan.c
        wifibroadcast.h
        wifiscan.c
        udp/rx_udp_util.h)

add_executable(udp_test_receiver
        udp/test/udp_test_receiver.c
        udp/test/udp_test_util.c
        udp/udp_client.c
        #udp/rx_udp_util.c
        )

add_executable(udp_test_sender
        udp/test/udp_test_sender.c
        udp/test/udp_test_util.c
        udp/udp_client.c)
