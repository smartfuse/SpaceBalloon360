# wifibroadcast x UDP

This version of the wifibroadcast receiver adds the functionality to forward packet data via UDP to another receiver in
addition to printing the contents in `stdout`.

To do this, pass in the additional arguments `-s <UDP server> -n <UDP port>` when starting wifibroadcast.

To have the receiver read from UDP instead of a WiFi adapter, pass in `-u` when starting wifibroadcast.

**UDP Stack**

The UDP code is stored in `/udp/`. `rx.c` references the methods in `udp_client.h` to send/receive data via UDP. There
are also tests for the UDP client located in `/udp/test`. The test receiver and sender can be compiled and run using
`scripts/run_udp_test_receiver.sh` and `scripts/run_udp_test_sender.sh`. Start the receiver first, and in a new shell
window, start the sender. The output on both should match.
