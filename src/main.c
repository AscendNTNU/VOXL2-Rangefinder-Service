/*******************************************************************************
 * Copyright 2023 ModalAI Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * 4. The Software is used solely in conjunction with devices provided by
 *    ModalAI Inc.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR busINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/


#include <stdio.h>
#include <stdlib.h> // for exit()
#include <signal.h>
#include <unistd.h>
#include <getopt.h>
#include <stdint.h>
#include <string.h>

#include <voxl_io/i2c.h>
#include <modal_start_stop.h>
#include <modal_pipe_server.h>
#include <voxl_rangefinder_interface.h>

#include "common.h"
#include "config_file.h"
#include "vl53l1x.h"


#define PROCESS_NAME	"voxl-rangefinder-server"
#define PIPE_CH		0


static int en_debug = 0;
static int en_timing = 0;
static int en_config_mode = 0;
static int config_arrangement = 0;




// printed if some invalid argument was given
static void print_usage(void)
{
	printf("\n\
voxl-rangefinder-server usually runs as a systemd background service. However, for debug\n\
purposes it can be started from the command line manually with any of the following\n\
debug options.\n\
\n\
The --config argument is used to reset the config file back to a default sensor\n\
arrangement and should only be used by the voxl-configure-rangefinders script.\n\
\n\
-c, --config {config #}     set config file to default configuration\n\
-d, --debug                 print debug info\n\
-h, --help                  print this help message\n\
-t, --timing                print timing info\n\
\n");
	return;
}


static int __parse_opts(int argc, char* argv[])
{
	static struct option long_options[] =
	{
		{"config",				required_argument,	0,	'c'},
		{"debug",				no_argument,		0,	'd'},
		{"help",				no_argument,		0,	'h'},
		{"timing",				no_argument,		0,	't'},
		{0, 0, 0, 0}
	};

	while(1){
		int option_index = 0;
		int c = getopt_long(argc, argv, "c:dht", long_options, &option_index);

		if(c == -1) break; // Detect the end of the options.

		switch(c){
		case 0:
			// for long args without short equivalent that just set a flag
			// nothing left to do so just break.
			if (long_options[option_index].flag != 0) break;
			break;

		case 'c':
			en_config_mode = 1;
			config_arrangement = atoi(optarg);
			break;

		case 'd':
			en_debug = 1;
			vl53l1x_set_en_debug(1);
			break;

		case 'h':
			print_usage();
			return -1;

		case 't':
			en_timing = 1;
			break;

		default:
			print_usage();
			return -1;
		}
	}

	return 0;
}


static int64_t _apps_time_monotonic_ns(void)
{
	struct timespec ts;
	if(clock_gettime(CLOCK_MONOTONIC, &ts)){
		fprintf(stderr,"ERROR calling clock_gettime\n");
		return -1;
	}
	return (int64_t)ts.tv_sec*1000000000 + (int64_t)ts.tv_nsec;
}



static int _set_multiplexer(int mux_ch, uint8_t addr)
{
	// // first set slave address to the multiplexer
	// const int i2c_bit_rate_hz	= 400000;
	// const int i2c_timeout_us	= 1000;
	//

	if(en_debug) printf("setting mux to %d\n", mux_ch);

	if(voxl_i2c_set_device_address(bus, mux_address)){
		fprintf(stderr, "failed to set i2c slave config on bus %d, address %d\n",
				bus, mux_address);
		return -1;
	}

	// set bitmask for mux channels to open
	uint8_t bitmask;
	if(mux_ch>7){	// all
		bitmask = 0xFF;
	}
	else if(mux_ch<0){	// none
		bitmask = 0;
	}
	else{	// just one
		bitmask = 1 << mux_ch;
	}

	if(voxl_i2c_send_byte(bus, bitmask)){
		fprintf(stderr, "failed to write to i2c multiplexer\n");
		return -1;
	}

	// then put address back to the rangefinder
	if(voxl_i2c_set_device_address(bus, addr)){
		fprintf(stderr, "failed to set i2c slave config on bus %d, address %d\n",
				bus, addr);
		return -1;
	}
	return 0;

}



static void _quit(int ret)
{
	printf("closing i2c bus\n");
	if(voxl_i2c_close(bus)){
		fprintf(stderr, "failed to close bus\n");
	}
	printf("closing pipes\n");
	pipe_server_close_all();
	remove_pid_file(PROCESS_NAME);
	printf("exiting\n");
	exit(ret);
	return;
}


int main(int argc, char* argv[])
{
	int i;

	// check for options
	if(__parse_opts(argc, argv)) return -1;

	// write out a new config file and quit if requested
	if(en_config_mode){
		return write_new_config_file_with_defaults(config_arrangement);
	}

	// read in config file
	if(read_config_file()) return -1;
	print_config();


	// make sure another instance isn't running
	// if return value is -3 then a background process is running with
	// higher privaledges and we couldn't kill it, in which case we should
	// not continue or there may be hardware conflicts. If it returned -4
	// then there was an invalid argument that needs to be fixed.
	if(kill_existing_process(PROCESS_NAME, 2.0)<-2) return -1;

	// start signal handler so we can exit cleanly
	if(enable_signal_handler()==-1){
		fprintf(stderr,"ERROR: failed to start signal handler\n");
		return -1;
	}

	printf("initializing i2c bus %d\n", bus);
	// don't worry, we will be changing this address later
	if(voxl_i2c_init(bus, VL53L1X_TOF_DEFAULT_ADDR)){
		fprintf(stderr, "failed to init bus\n");
		return -1;
	}

	// let sensors wake up, todo check if this is needed
	usleep(10000);
	make_pid_file(PROCESS_NAME);

	// init all the sensors
	for(i=0;i<n_enabled_sensors;i++){

		// set up non-multiplexed sensor
		if(!enabled_sensors[i].is_on_mux){
			printf("initializing non-multiplexed tof sensor id %d\n", enabled_sensors[i].sensor_id);
			if(n_mux_sensors>0 && _set_multiplexer(MUX_NONE, VL53L1X_TOF_DEFAULT_ADDR)){
				fprintf(stderr, "failed to set slave\n");
				_quit(-1);
			}
		}
		else{
			printf("initializing multiplexed tof sensor id %d at mux port %d\n", enabled_sensors[i].sensor_id, enabled_sensors[i].i2c_mux_port);

			if(_set_multiplexer(enabled_sensors[i].i2c_mux_port, VL53L1X_TOF_DEFAULT_ADDR)){
				fprintf(stderr, "failed to set slave\n");
				_quit(-1);
			}
		}

		if(vl53l1x_init(i)) _quit(-1);
	}

	if(en_debug) printf("finished initializing %d vl53l1x sensors\n", n_enabled_sensors);


	// create the pipe
	pipe_info_t info = { \
		.name        = RANGEFINDER_PIPE_NAME,\
		.location    = RANGEFINDER_PIPE_LOCATION ,\
		.type        = "rangefinder_data_t",\
		.server_name = PROCESS_NAME,\
		.size_bytes  = RANGEFINDER_RECOMMENDED_PIPE_SIZE};

	if(pipe_server_create(PIPE_CH, info, 0)) _quit(-1);

	// pre-fill an array of data structs to send out the pipe
	rangefinder_data_t data[MAX_SENSORS];
	for(i=0; i<n_enabled_sensors;i++){
		data[i].magic_number			= RANGEFINDER_MAGIC_NUMBER;
		data[i].timestamp_ns			= 0;
		data[i].sample_id				= 0;
		data[i].sensor_id				= enabled_sensors[i].sensor_id;
		data[i].distance_m				= 0.0f;
		data[i].uncertainty_m			= enabled_sensors[i].uncertainty_m;
		data[i].fov_deg					= enabled_sensors[i].fov_deg;
		data[i].location_wrt_body[0]	= enabled_sensors[i].location_wrt_body[0];
		data[i].location_wrt_body[1]	= enabled_sensors[i].location_wrt_body[1];
		data[i].location_wrt_body[2]	= enabled_sensors[i].location_wrt_body[2];
		data[i].direction_wrt_body[0]	= enabled_sensors[i].direction_wrt_body[0];
		data[i].direction_wrt_body[1]	= enabled_sensors[i].direction_wrt_body[1];
		data[i].direction_wrt_body[2]	= enabled_sensors[i].direction_wrt_body[2];
		data[i].range_max_m				= enabled_sensors[i].range_max_m;
		data[i].type					= enabled_sensors[i].type;
		data[i].reserved				= 0;
	}



	// keep track of timing
	int64_t last_time_ns = _apps_time_monotonic_ns();

	// small array to keep the distances in
	int dist_mm[MAX_SENSORS];

	// keep sampling until signal handler tells us to stop
	main_running = 1;
	while(main_running){

		// nothing to do if there are no clients and not in debug mode
		if(pipe_server_get_num_clients(PIPE_CH)==0 && !en_debug){
			usleep(500000);
			continue;
		}

		// start the standalone sensor ranging if it exists
		// TODO this should be the secodnary address later
		if(has_nonmux_sensor){
			if(n_mux_sensors>0){
				_set_multiplexer(MUX_NONE, VL53L1X_TOF_DEFAULT_ADDR);
			}
			if(vl53l1x_start_ranging()){
				fprintf(stderr, "failed to start ranging\n");
				main_running = 0;
				break;
			}
		}
		// start all the multiplexed sensors reading at the same time
		if(n_mux_sensors>0){
			_set_multiplexer(MUX_ALL, VL53L1X_TOF_DEFAULT_ADDR);
			if(vl53l1x_start_ranging()){
				fprintf(stderr, "failed to start ranging\n");
				main_running = 0;
				break;
			}
		}

		// sleep a bit while they range, this should be less than the actual
		// ranging time so we can poll them at the end of the ranging process
		usleep(TimingBudgetInMs*1000);
		// now start reading the data back in
		memset(dist_mm, 0, sizeof(dist_mm));
		for(i=0;i<n_enabled_sensors;i++){

			// switch i2c bus and multiplexer over to either a multiplexed or non-multiplexed sensor
			if(enabled_sensors[i].is_on_mux){
				_set_multiplexer(	enabled_sensors[i].i2c_mux_port,\
							enabled_sensors[i].i2c_mux_address);
			}
			else if(n_mux_sensors>0) {
				_set_multiplexer(MUX_NONE, VL53L1X_TOF_DEFAULT_ADDR);
			}

			// wait for it to be done ranging
			if(vl53l1x_wait_for_data()){
				main_running = 0;
				break;
			}

			// read in the data and stop it ranging
			vl53l1x_get_distance_mm(&dist_mm[i]);
			vl53l1x_clear_interrupt();
			vl53l1x_stop_ranging();
		}


		// grab time at the end of ranging
		// assume timestamp of data was from halfway through the reading process
		// TODO validate this experimentally
		int64_t time_ns = _apps_time_monotonic_ns();
		int64_t timestamp_ns = time_ns - ((time_ns-last_time_ns)/2);

		// keep track of number of samples read
		static int sample_id = 0;
		sample_id ++;

		// populate data for pipe and send it out all at once
		for(i=0; i<n_enabled_sensors;i++){
			data[i].timestamp_ns			= timestamp_ns;
			data[i].sample_id				= sample_id;
			data[i].distance_m				= dist_mm[i]/1000.0;;
		}
		pipe_server_write(PIPE_CH, data, sizeof(rangefinder_data_t)*n_enabled_sensors);

		// print distances in debug mode
		if(en_timing){
			double dt_ms = (time_ns-last_time_ns)/1000000.0;
			printf("dt = %6.1fms ", dt_ms);
			printf("\n");
		}

		last_time_ns = time_ns;
	}

	// close and cleanup
	printf("exiting cleanly\n");
	_quit(0);
	return 0;
}
