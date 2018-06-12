#include <cstdio>
#include "../fec.h"
//#include "lib.h"
//#include "wifibroadcast.h"
//#include "radiotap.h"
#include "udp_client.h"
#include <time.h>
#include <sys/resource.h>
#include "rx_udp_util.h"

using namespace std;

#define MAX_PACKET_LENGTH 4192
#define MAX_USER_PACKET_LENGTH 1450
#define MAX_DATA_OR_FEC_PACKETS_PER_BLOCK 32
#define MAX_ADDRESS_LENGTH 500

#define DEBUG 0
#define debug_print(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)



int flagHelp = 0;
int param_port = 0;
int param_data_packets_per_block = 8;
int param_fec_packets_per_block = 4;
int param_block_buffers = 1;
int param_packet_length = MAX_USER_PACKET_LENGTH;
int param_packet_type = 1;

wifibroadcast_rx_status_t *rx_status = NULL;
int max_block_num = -1;
UdpSession session;
char remote_address[MAX_ADDRESS_LENGTH];
int remote_port;
int receive_port;

void
usage(void)
{
	printf(
	    "(c)2015 befinitiv. Dirty mods by Rodizio. Based on packetspammer by Andy Green. Licensed under GPL2\n"
	    "\n"
	    "Usage: rx [options] <interfaces>\n\nOptions\n"
	    "-p <port>   Port number 0-255 (default 0)\n"
	    "-b <count>  Number of data packets in a block (default 8). Needs to match with tx.\n"
	    "-r <count>  Number of FEC packets per block (default 4). Needs to match with tx.\n"
	    "-f <bytes>  Bytes per packet (default %d. max %d). This is also the FEC block size. Needs to match with tx\n"
	    "-d <blocks> Number of transmissions blocks that are buffered (default 1). This is needed in case of diversity if one\n"
	    "            adapter delivers data faster than the other. Note that this increases latency.\n"
	    "-t <type>   Packet type to receive. 0 = DATA short, 1 = DATA standard, 2 = CTS, 3 = RTS, 4 = ACK, 5 = Beacon\n"
	    "\n"
	    "Example:\n"
	    "  rx -b 8 -r 4 -f 1024 -t 1 wlan0 | cat /dev/null  (receive standard DATA frames on wlan0 and send payload to /dev/null)\n"
	    "\n", MAX_USER_PACKET_LENGTH, MAX_USER_PACKET_LENGTH);
	exit(1);
}

typedef struct {
	pcap_t *ppcap;
	int selectable_fd;
	int n80211HeaderLength;
} monitor_interface_t;

typedef struct {
	int block_num;
	packet_buffer_t *packet_buffer_list;
} block_buffer_t;




void block_buffer_list_reset(block_buffer_t *block_buffer_list, size_t block_buffer_list_len, int block_buffer_len) {
    int i;
    block_buffer_t *rb = block_buffer_list;

    for(i=0; i<block_buffer_list_len; ++i) {
        rb->block_num = -1;

        int j;
        packet_buffer_t *p = rb->packet_buffer_list;
        for(j=0; j<param_data_packets_per_block+param_fec_packets_per_block; ++j) {
            p->valid = 0;
            p->crc_correct = 0;
            p->len = 0;
            p++;
        }

        rb++;
    }
}

void process_payload(uint8_t *data, size_t data_len, int crc_correct, block_buffer_t *block_buffer_list, int adapter_no)
{
	if(!crc_correct) {
		rx_status->adapter[adapter_no].wrong_crc_cnt++;
	}

	rx_status->adapter[adapter_no].received_packet_cnt++;
	rx_status->last_update = time(NULL);

    wifi_packet_header_t *wph;
    int block_num;
    int packet_num;
    int i;

    wph = (wifi_packet_header_t*)data;
    data += sizeof(wifi_packet_header_t);
    data_len -= sizeof(wifi_packet_header_t);

    block_num = wph->sequence_number / (param_data_packets_per_block+param_fec_packets_per_block);//if aram_data_packets_per_block+param_fec_packets_per_block would be limited to powers of two, this could be replaced by a logical AND operation

    //debug_print("adap %d rec %x blk %x crc %d len %d\n", adapter_no, wph->sequence_number, block_num, crc_correct, data_len);

    //we have received a block number that exceeds the currently seen ones -> we need to make room for this new block
    //or we have received a block_num that is several times smaller than the current window of buffers -> this indicated that either the window is too small or that the transmitter has been restarted
    int tx_restart = (block_num + 128*param_block_buffers < max_block_num);
    if((block_num > max_block_num || tx_restart) && crc_correct) {
        if(tx_restart) {
	    rx_status->tx_restart_cnt++;
//            fprintf(stderr, "TX re-start detected\n");
            block_buffer_list_reset(block_buffer_list, param_block_buffers, param_data_packets_per_block + param_fec_packets_per_block);
        }

        //first, find the minimum block num in the buffers list. this will be the block that we replace
        int min_block_num = INT_MAX;
        int min_block_num_idx;
        for(i=0; i<param_block_buffers; ++i) {
            if(block_buffer_list[i].block_num < min_block_num) {
                min_block_num = block_buffer_list[i].block_num;
                min_block_num_idx = i;
            }
        }

        //debug_print("removing block %x at index %i for block %x\n", min_block_num, min_block_num_idx, block_num);

        packet_buffer_t *packet_buffer_list = block_buffer_list[min_block_num_idx].packet_buffer_list;
        int last_block_num = block_buffer_list[min_block_num_idx].block_num;

        if(last_block_num != -1) {
            rx_status->received_block_cnt++;

            //we have both pointers to the packet buffers (to get information about crc and vadility) and raw data pointers for fec_decode
            packet_buffer_t *data_pkgs[MAX_DATA_OR_FEC_PACKETS_PER_BLOCK];
            packet_buffer_t *fec_pkgs[MAX_DATA_OR_FEC_PACKETS_PER_BLOCK];
            uint8_t *data_blocks[MAX_DATA_OR_FEC_PACKETS_PER_BLOCK];
            uint8_t *fec_blocks[MAX_DATA_OR_FEC_PACKETS_PER_BLOCK];
            int datas_missing = 0, datas_corrupt = 0, fecs_missing = 0,fecs_corrupt = 0;
            int di = 0, fi = 0;

            //first, split the received packets into DATA a FEC packets and count the damaged packets
            i = 0;
            while(di < param_data_packets_per_block || fi < param_fec_packets_per_block) {
                if(di < param_data_packets_per_block) {
                    data_pkgs[di] = packet_buffer_list + i++;
                    data_blocks[di] = data_pkgs[di]->data;
                    if(!data_pkgs[di]->valid)
                        datas_missing++;

                    if(data_pkgs[di]->valid && !data_pkgs[di]->crc_correct)
                        datas_corrupt++;
                    di++;
                }

                if(fi < param_fec_packets_per_block) {
                    fec_pkgs[fi] = packet_buffer_list + i++;
                    if(!fec_pkgs[fi]->valid)
                        fecs_missing++;
                    if(fec_pkgs[fi]->valid && !fec_pkgs[fi]->crc_correct)
                        fecs_corrupt++;

                    fi++;
                }
            }

            const int good_fecs_c = param_fec_packets_per_block - fecs_missing - fecs_corrupt;
            const int datas_missing_c = datas_missing;
            const int datas_corrupt_c = datas_corrupt;
            const int fecs_missing_c = fecs_missing;
//            const int fecs_corrupt_c = fecs_corrupt;

	    int packets_lost_in_block = 0;
            int good_fecs = good_fecs_c;
            //the following three fields are infos for fec_decode
            unsigned int fec_block_nos[MAX_DATA_OR_FEC_PACKETS_PER_BLOCK];
            unsigned int erased_blocks[MAX_DATA_OR_FEC_PACKETS_PER_BLOCK];
            unsigned int nr_fec_blocks = 0;

            if(datas_missing_c + fecs_missing_c > 0) {
		packets_lost_in_block = (datas_missing_c + fecs_missing_c);
                rx_status->lost_packet_cnt = rx_status->lost_packet_cnt + packets_lost_in_block;
	    }

	    rx_status->received_packet_cnt = rx_status->received_packet_cnt + param_data_packets_per_block + param_fec_packets_per_block - packets_lost_in_block;


#if DEBUG
            if(datas_missing_c + datas_corrupt_c > good_fecs_c) {
                int x;

                for(x=0;x<param_data_packets_per_block; ++x) {
                    if(data_pkgs[x]->valid) {
                        if(data_pkgs[x]->crc_correct)
                            fprintf(stderr, "v");
                        else
                            fprintf(stderr, "c");
                    }
                    else
                        fprintf(stderr, "m");
                }

                fprintf(stderr, " ");

                for(x=0;x<param_fec_packets_per_block; ++x) {
                    if(fec_pkgs[x]->valid) {
                        if(fec_pkgs[x]->crc_correct)
                            fprintf(stderr, "v");
                        else
                            fprintf(stderr, "c");
                    }
                    else
                        fprintf(stderr, "m");
                }

                fprintf(stderr, "\n");
            }
#endif

            fi = 0;
            di = 0;

            //look for missing DATA and replace them with good FECs
            while(di < param_data_packets_per_block && fi < param_fec_packets_per_block) {
                //if this data is fine we go to the next
                if(data_pkgs[di]->valid && data_pkgs[di]->crc_correct) {
                    di++;
                    continue;
                }

                //if this DATA is corrupt and there are less good fecs than missing datas we cannot do anything for this data
                if(data_pkgs[di]->valid && !data_pkgs[di]->crc_correct && good_fecs <= datas_missing) {
                    di++;
                    continue;
                }

                //if this FEC is not received we go on to the next
                if(!fec_pkgs[fi]->valid) {
                    fi++;
                    continue;
                }

                //if this FEC is corrupted and there are more lost packages than good fecs we should replace this DATA even with this corrupted FEC
                if(!fec_pkgs[fi]->crc_correct && datas_missing > good_fecs) {
                    fi++;
                    continue;
                }


                if(!data_pkgs[di]->valid)
                    datas_missing--;
                else if(!data_pkgs[di]->crc_correct)
                    datas_corrupt--;

                if(fec_pkgs[fi]->crc_correct)
                    good_fecs--;

                //at this point, data is invalid and fec is good -> replace data with fec
                erased_blocks[nr_fec_blocks] = di;
                fec_block_nos[nr_fec_blocks] = fi;
                fec_blocks[nr_fec_blocks] = fec_pkgs[fi]->data;
                di++;
                fi++;
                nr_fec_blocks++;
            }


            int reconstruction_failed = datas_missing_c + datas_corrupt_c > good_fecs_c;

            if(reconstruction_failed) {
                //we did not have enough FEC packets to repair this block
                rx_status->damaged_block_cnt++;
                //fprintf(stderr, "Could not fully reconstruct block %x! Damage rate: %f (%d / %d blocks)\n", last_block_num, 1.0 * rx_status->damaged_block_cnt / rx_status->received_block_cnt, rx_status->damaged_block_cnt, rx_status->received_block_cnt);
                //debug_print("Data mis: %d\tData corr: %d\tFEC mis: %d\tFEC corr: %d\n", datas_missing_c, datas_corrupt_c, fecs_missing_c, fecs_corrupt_c);
            }

            //decode data and write it to STDOUT
            fec_decode((unsigned int) param_packet_length, data_blocks, param_data_packets_per_block, fec_blocks, fec_block_nos, erased_blocks, nr_fec_blocks);
            for(i=0; i<param_data_packets_per_block; ++i) {
                payload_header_t *ph = (payload_header_t*)data_blocks[i];

                if(!reconstruction_failed || data_pkgs[i]->valid) {
                    //if reconstruction did fail, the data_length value is undefined. better limit it to some sensible value
                    if(ph->data_length > param_packet_length)
                        ph->data_length = param_packet_length;

                    write(STDOUT_FILENO, data_blocks[i] + sizeof(payload_header_t), ph->data_length);
		    fflush(stdout);
                }
            }

            //reset buffers
            for(i=0; i<param_data_packets_per_block + param_fec_packets_per_block; ++i) {
                packet_buffer_t *p = packet_buffer_list + i;
                p->valid = 0;
                p->crc_correct = 0;
                p->len = 0;
            }
        }

    block_buffer_list[min_block_num_idx].block_num = block_num;
    max_block_num = block_num;
    }

    //find the buffer into which we have to write this packet
    block_buffer_t *rbb = block_buffer_list;
    for(i=0; i<param_block_buffers; ++i) {
        if(rbb->block_num == block_num) {
            break;
        }
        rbb++;
    }



    //check if we have actually found the corresponding block. this could not be the case due to a corrupt packet
    if(i != param_block_buffers) {
        packet_buffer_t *packet_buffer_list = rbb->packet_buffer_list;
        packet_num = wph->sequence_number % (param_data_packets_per_block+param_fec_packets_per_block); //if retr_block_size would be limited to powers of two, this could be replace by a locical and operation

        //only overwrite packets where the checksum is not yet correct. otherwise the packets are already received correctly
        if(packet_buffer_list[packet_num].crc_correct == 0) {
	    memcpy(packet_buffer_list[packet_num].data, data, data_len);
            packet_buffer_list[packet_num].len = data_len;
            packet_buffer_list[packet_num].valid = 1;
            packet_buffer_list[packet_num].crc_correct = crc_correct;
        }
    }

}

void send_udp(uint8_t *data, size_t data_len, int crc_correct) {
    char *buffer = create_buffer();
    struct RxStruct rxStruct;
    memset(rxStruct.data, data, MAX_PACKET_LEN);
    memcpy(rxStruct.data, data, data_len);

    rxStruct.data_len = data_len;
    rxStruct.crc_correct = crc_correct;

    size_t dataSize = write_buffer(buffer, rxStruct);
    send_data(session, buffer, dataSize);

    free_buffer(&buffer);
}

int main(int argc, char *argv[])
{

	setpriority(PRIO_PROCESS, 0, -10);

	monitor_interface_t interfaces[MAX_PENUMBRA_INTERFACES];
	int num_interfaces = 0;
	int i;

	block_buffer_t *block_buffer_list;

	int c;
	
	

	if(param_packet_length > MAX_USER_PACKET_LENGTH) {
		printf("Packet length is limited to %d bytes (you requested %d bytes)\n", MAX_USER_PACKET_LENGTH, param_packet_length);
		return (1);
	}


	fec_init();


	rx_status = status_memory_open();

	int j = 0;
	int x = optind;

	char path[45], line[100];


    //block buffers contain both the block_num as well as packet buffers for a block.
    block_buffer_list = malloc(sizeof(block_buffer_t) * param_block_buffers);
    for(i=0; i<param_block_buffers; ++i)
    {
        block_buffer_list[i].block_num = -1;
        block_buffer_list[i].packet_buffer_list = lib_alloc_packet_buffer_list(param_data_packets_per_block+param_fec_packets_per_block, MAX_PACKET_LENGTH);
    }


    // FIXME: hardcode... bind to all of them?
    strncpy(remote_address, "192.168.1.249", MAX_ADDRESS_LENGTH);
    remote_port = 6000;

    fprintf(stderr, "my own udp server: %s:%u\n", &remote_address[0], remote_port);
    if (strlen(remote_address) > 0) {
        session = start_session(remote_address, remote_port, 1);
        if (!session) {
            fprintf(stderr, "whoops no udp bits\n");
            return 0;
        }
    }


	for(;;)
	{
        char    buffer[42*1024];    // just pick a number

        ssize_t r = receive_data(session, &buffer[0], sizeof(buffer));
        if (r > 0) {
            process_payload(pu8Payload, bytes, checksum_correct, block_buffer_list, adapter_no);
        }
    }

	return (0);
}
