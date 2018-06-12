LDFLAGS=-lrt -lpcap -lwiringPi
CPPFLAGS=-Wall -D _GNU_SOURCE

all: rx rcrx rssirx rssitx tx rx_status tracker channelscan check_alive rssi_forward rssi_forward_stdout rssi_write_shmem

%.o: %.c
	gcc -c -o $@ $< $(CPPFLAGS)

udp_test_sender: udp/test/udp_test_sender.o udp/test/udp_test_util.o udp/udp_client.o
	gcc -o $@ $^

udp_test_receiver: udp/test/udp_test_receiver.o udp/test/udp_test_util.o udp/udp_client.o
	gcc -o $@ $^

rx: rx.o lib.o radiotap.o fec.o
	gcc -o $@ $^ -lrt -lpcap

rcrx: rcrx.o lib.o radiotap.o
	gcc -o $@ $^ $(LDFLAGS)

rssirx: rssirx.o lib.o radiotap.o
	gcc -o $@ $^ $(LDFLAGS)

rssitx: rssitx.o lib.o radiotap.o
	gcc -o $@ $^ $(LDFLAGS)

tx: tx.o lib.o fec.o
	gcc -o $@ $^ $(LDFLAGS)

rx_status: rx_status.o
	gcc -o $@ $^ $(LDFLAGS)

tracker: tracker.o
	gcc -o $@ $^ $(LDFLAGS)

channelscan: channelscan.o
	gcc -o $@ $^ $(LDFLAGS)

check_alive: check_alive.o
	gcc -o $@ $^ $(LDFLAGS)

rssi_forward: rssi_forward.o
	gcc -o $@ $^ $(LDFLAGS)

rssi_forward_stdout: rssi_forward_stdout.o
	gcc -o $@ $^ $(LDFLAGS)

rssi_write_shmem: rssi_write_shmem.o
	gcc -o $@ $^ $(LDFLAGS)



clean:
	rm -f rx rcrx rssirx rssitx tx rx_status tracker channelscan check_alive rssi_forward rssi_forward_stdout rssi_write_shmem udp_test_sender udp_test_receiver *~ *.o udp/*.o udp/test/*.o
