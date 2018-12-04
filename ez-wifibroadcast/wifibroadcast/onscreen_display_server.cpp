#include <iostream>
#include <pcap.h>
#include <string>
#include <sstream>
#include <sys/mman.h>
#include <unistd.h>
#include <utime.h>

#include "lib.h"

using namespace std;

const string kOutputFileName = "/tmp/mpvsocket";
const string kSharedMemoryFileName = "wifibroadcast_rx_status_0";
const long kLoopIntervalMicroSec = 1e5L;

typedef wifibroadcast_rx_status_t_rc* rx_status_ref;

/**
 * Gets the file descriptor pointing to the shared memory containing the
 * receiver status structure.
 */
int get_shared_memory_file() {
	int file_descriptor;
	while (true) {
		    	char buf[128];

    	sprintf(buf, "/wifibroadcast_rx_status_%d", 0);

		file_descriptor =
				shm_open(buf, O_RDWR, S_IRUSR | S_IWUSR);
		if (file_descriptor > 0) {
			break;
		}
		usleep(kLoopIntervalMicroSec);
	}

	return file_descriptor;
}

/**
 * Opens the shared memory containing the receiver status structure that is
 * constantly updated by the receiver.
 */
rx_status_ref open_status_memory() {
	int file_descriptor = get_shared_memory_file();
	void *memory_reference =
			mmap(
					NULL,
					sizeof(wifibroadcast_rx_status_t_rc),
					PROT_READ,
					MAP_SHARED,
					file_descriptor,
					0);
	if (memory_reference == MAP_FAILED) {
		perror("mmap failure.");
		exit(1);
	}

	return (wifibroadcast_rx_status_t_rc *) memory_reference;
}

/**
 * Writes the current status to console.
 * @param status_ref the reference to the current status
 */
void output_status(int &best_signal, rx_status_ref status_ref) {
			// Blocks
		unsigned long received_blocks = status_ref->received_block_cnt;
		unsigned long damaged_blocks = status_ref->damaged_block_cnt;

		// Adapter
		unsigned long received_packets = status_ref->adapter[0].received_packet_cnt;
		long signal_strength = status_ref->adapter[0].current_signal_dbm;

		// Packets
		unsigned long lost_packets = status_ref->lost_packet_cnt;
		unsigned long lost_packets_per_block = status_ref->lost_per_block_cnt;

		// Transmission
		unsigned long transmission_restarts = status_ref->tx_restart_cnt;
		unsigned long bitrate = status_ref->kbitrate;

		if (best_signal < signal_strength) {
			best_signal = signal_strength;
		}

		stringstream output;

		output << "Periscope Space Console" << "\\n";
		output << "Blocks received |\t" << received_blocks << "\\n";
		output << "Blocks damaged |\t" << damaged_blocks << "\\n";
		output << "Packets received |\t" << received_packets << "\\n";
		output << "Signal strength |\t" << signal_strength << " dbm\\n";
		output << "Packets lost |\t" << lost_packets << "\\n";
		output << "Packets lost per block |\t" << lost_packets_per_block << "\\n";
		output << "Transmission restarts |\t" << transmission_restarts << "\\n";
		output << "Bitrate |\t" << bitrate << " kbps\\n";
		output << "Best signal |\t" << best_signal << " dbm";

		cout << "show-text \"" << output.str() << "\"" << endl;
}

int main(int argc, char *argv[]) {
	rx_status_ref status_ref = open_status_memory();
	int best_signal = -1000;

	while (true) {
		output_status(best_signal, status_ref);
		sleep(1);
	}

	return 0;
}
