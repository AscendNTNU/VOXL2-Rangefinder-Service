
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <voxl_io/i2c.h>
#include "sf20c.h"
#include "config_file.h" // for bus global variable
#include "string.h"
#include <stdint.h>


static int en_debug = 0;

#define I2C_BUS 1 // /dev/i2c-{I2C_BUS}
#define SF20_START_BYTE 0xAA
#define SF20_MAX_PAYLOAD 1023  /* protocol limit in the manual */
#define SF20_PACKET_MIN_BYTES 6 // Packet size with no payload

typedef struct __attribute__((packed)){
	uint8_t 	start;
	uint16_t 	flags;
	uint8_t 	id;
	uint8_t 	size;
	uint16_t 	crc;
	uint8_t 	payload[];
} sf20_packet_t;

static inline uint16_t sf20_make_flags(uint16_t payload_len, uint8_t write)
{
    return (uint16_t)(((payload_len & SF20_MAX_PAYLOAD) << 6) | (write ? 1u : 0u));
}

static inline uint16_t sf20_crc(sf20_packet_t* packet)
{
	uint8_t data[1 + 2 + 1 + packet->size];
	data[0] = packet->start;
	data[1] = (packet->flags >> 8) & 0xff;
	data[2] = packet->flags & 0xff;
	data[3] = packet->id;
	memcpy(data + 4, packet->payload, packet->size);

	uint16_t crc = 0;
	for (uint32_t i = 0; i < 4 + packet->size; ++i)
	{
		uint16_t code = crc >> 8;
		code ^= data[i];
		code ^= code >> 4;
		crc = crc << 8;
		crc ^= code;
		code = code << 5;
		crc ^= code;
		code = code << 7;
		crc ^= code;
	}
	return crc;
}

static inline size_t sf20_make_packet(uint8_t id, uint8_t* payload, uint16_t size, uint8_t* out_buf, uint8_t w)
{	
	
	sf20_packet_t* pkt = (sf20_packet_t*)out_buf;

	pkt->start = SF20_START_BYTE;
	pkt->flags = sf20_make_flags(1 + size, 1); // id + payload_len is 1 + size
	pkt->id = id;
	pkt->size = size;

	memcpy(pkt->payload, payload, size);

	uint16_t crc = sf20_crc(pkt);
	memcpy(pkt->payload + size, &crc, 2);

	return 1 + 2 + 1 + size + 2; // Total number of bytes
}

static inline size_t sf20_make_write_packet(uint8_t id, uint8_t* payload, uint16_t size, uint8_t* out_buf) {
	return sf20_make_packet(id, payload, size, out_buf, 1);
}

static inline size_t sf20_make_read_packet(uint8_t id, uint8_t* payload, uint16_t size, uint8_t* out_buf) {
	return sf20_make_packet(id, payload, size, out_buf, 0);
}

void sf20c_set_en_debug(int en){
	en_debug = en;
	return;
}

// reverse lsb and msb bytes of a 16-bit register for DSPAL
static uint32_t _reverse_lsb_msb_16(uint16_t reg)
{
	uint32_t out = reg >> 8;
	out |= (reg & 0xff) << 8;
	return out;
}

static void _print_status(uint8_t status)
{
	printf("status: %d  ", status);
	switch(status){
		case 0:
			printf("Valid Range\n");
			break;
		case 1:
			printf("Sigma Fail\n");
			break;
		case 2:
			printf("Low Signal\n");
			break;
		case 3:
			printf("Min Range\n");
			break;
		case 4:
			printf("Phase OOB\n");
			break;
		case 5:
			printf("Hardware Failure\n");
			break;
		case 7:
			printf("Wrapped Target\n");
			break;
		case 8:
			printf("Processing Failure\n");
			break;
		case 14:
			printf("Range Invalid\n");
			break;
		default:
			printf("Other Error\n");
	}
	return;
}

int sf20c_get_distance_mm(int* dist_mm, int* sd)
{
	// set outputs to -1 so we can quit right away on error
	*dist_mm = -1000;
	*sd = -1;

	uint8_t data[sizeof(int32_t)*4];
	voxl_i2c_read_bytes(I2C_BUS, 45, sizeof(int32_t)*4, data);

	int32_t dist_mm_raw = ((uint32_t)data[0] << 0) | ((uint32_t)data[1] << 8) | ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24);
	int32_t signal_strength = ((uint32_t)data[4] << 0) | ((uint32_t)data[5] << 8) | ((uint32_t)data[6] << 16) | ((uint32_t)data[7] << 24);

	if(en_debug){
		printf("mm:%5d ", dist_mm_raw);
		printf("signal:%6d \r\n", signal_strength);
	}

	*dist_mm = dist_mm_raw;
	*sd = signal_strength;

	return 0;
}



// argument is the index of this sensor in the enabled_sensors array
int sf20c_init(void)
{

	if(en_debug){
		printf("initializing an sf20c\r\n");
	}

	usleep(10000);

	// Wake up by reading product name
	uint8_t prod_name[16];
	voxl_i2c_read_bytes(I2C_BUS, 0, 16, prod_name);
	voxl_i2c_read_bytes(I2C_BUS, 0, 16, prod_name);
	voxl_i2c_read_bytes(I2C_BUS, 0, 16, prod_name);
	
	if (en_debug) { // Print product name
		printf("Read product name: '%s'\n", (char*)prod_name);
	}

	// Ensure register based protocol.
	uint8_t comm_bytes[2] = {0xAA, 0xAA};
	voxl_i2c_write_bytes(I2C_BUS, 120, 2, comm_bytes); // Enable register based protocol
	voxl_i2c_read_bytes(I2C_BUS, 120, 2, comm_bytes); // Read to ensure
	
	if (comm_bytes[0] != 0xCC && comm_bytes[1] != 0) {
		fprintf(stderr, "Failed to activate register mode\r\n");
		return -1;
	}

	if(sf20c_set_distance_output(561)) { // first return strength and distance + last return strength + distance
		fprintf(stderr, "Failed to set output distance mode for sensor.\r\n");
		return -1;
	}

	if(en_debug){
		printf("done initializing a sensor\n");
	}

	return 0;
}


// this assumes mux is off and we can only see one sensor
int sf20c_set_bus_to_default_slave_address(void)
{
	if(voxl_i2c_set_device_address(bus, SF20C_TOF_DEFAULT_ADDR)){
		fprintf(stderr, "failed to set i2c slave config on bus %d, address %d\n",
											bus, SF20C_TOF_DEFAULT_ADDR);
		return -1;
	}
	return 0;
}


int sf20c_set_distance_output(uint32_t bits) {
	uint8_t payload[4] = {
		bits >> 0 & 0xff,
		bits >> 8 & 0xff,
		bits >> 16 & 0xff,
		bits >> 24 & 0xff
	};

	printf("%d %d %d %d", payload[0], payload[1], payload[2], payload[3]);

	voxl_i2c_write_bytes(I2C_BUS, 27, 4, payload);

	uint8_t recv[4];
	voxl_i2c_read_bytes(I2C_BUS, 27, 4, recv);

	for (uint8_t i = 0; i < 4; i++) {
		if(recv[i] != payload[i]) {
			return -1;
		}
	}

	return 0;
}