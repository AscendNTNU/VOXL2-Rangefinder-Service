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
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

#include <stdio.h>
#include <signal.h>
#include <getopt.h>
#include <unistd.h>	// for usleep()
#include <string.h>
#include <stdlib.h> // for atoi()
//#include <math.h>
#include <time.h>

#include <modal_pipe_client.h>
#include <modal_start_stop.h>
#include <voxl_rangefinder_interface.h>

#define CLIENT_NAME		"inspect-rangefinders"


static char pipe_path[MODAL_PIPE_MAX_PATH_LEN] = RANGEFINDER_PIPE_LOCATION;
static int en_newline = 0;
static bool test_mode = false;
static int test_passed = 0;


#define DISABLE_WRAP		"\033[?7l"	// disables line wrap, be sure to enable before exiting
#define ENABLE_WRAP			"\033[?7h"	// default terminal behavior
#define RESET_FONT			"\x1b[0m"	// undo any font/color settings
#define FONT_BOLD			"\033[1m"	// bold font
#define CLEAR_LINE			"\033[2K"	// erases line but leaves curser in place


static int64_t _apps_time_monotonic_ns(void)
{
	struct timespec ts;
	if(clock_gettime(CLOCK_MONOTONIC, &ts)){
		fprintf(stderr,"ERROR calling clock_gettime\n");
		return -1;
	}
	return (int64_t)ts.tv_sec*1000000000 + (int64_t)ts.tv_nsec;
}


static void _print_usage(void)
{
	printf("\n\
typical usage\n\
/# voxl-inspect-rangefinder\n\
\n\
This will print out rangefinder data from Modal Pipe Architecture.\n\
By default this opens the pipe \"rangefinders\"/\n\
but this can be changed with the --pipe option.\n\
\n\
Range in meters will always print. Additional options are:\n\
-h, --help                  print this help message\n\
-n, --newline               print each sample on a new line\n\
-p, --pipe {pipe_name}      optionally specify the pipe name\n\
-t, --test					test rangefinder feedback\n\
\n");
	return;
}



// called whenever we connect or reconnect to the server
static void _connect_cb(__attribute__((unused)) int ch, __attribute__((unused)) void* context)
{
	printf(FONT_BOLD);
	printf("   id  |");
	printf("latency(ms)|");
	printf("distances (m)");
	printf("\n");
	printf(RESET_FONT);
	return;
}


// called whenever we disconnect from the server
static void _disconnect_cb(__attribute__((unused)) int ch, __attribute__((unused)) void* context)
{
	fprintf(stderr, "\nserver disconnected\n");
	return;
}



static void _helper_cb( __attribute__((unused)) int ch, char* data, int bytes, __attribute__((unused)) void* context)
{
	// validate that the data makes sense
	int n_packets, i;
	rangefinder_data_t* d = voxl_rangefinder_validate_pipe_data(data, bytes, &n_packets);
	if(d == NULL) return;

	// keep track of current sample id, multiple rangefinder readings will have
	// the same sample_id if fired together
	static uint32_t current_sample = 0;

	for(i=0;i<n_packets;i++){

		// if a new sample_id is detected, start next line
		if(current_sample != d[i].sample_id){

			if(en_newline){
				printf("\n");
			}
			else{
				printf("\r" CLEAR_LINE);
			}

			// print sample and latency
			current_sample = d[i].sample_id;
			double latency_ns = _apps_time_monotonic_ns() - d[i].timestamp_ns;
			printf("%6d |", current_sample);
			printf("%8.1f   |", latency_ns/1000000.0);
		}

		// now just print each distance
		printf("%6.3f ", (double)d[i].distance_m);
	}

	fflush(stdout);
	return;
}


static int _parse_opts(int argc, char* argv[])
{
	static struct option long_options[] =
	{
		{"help",				no_argument,		0, 'h'},
		{"newline",				no_argument,		0, 'n'},
		{"pipe",				required_argument,	0, 'p'},
		{"test",				no_argument,		0, 't'},
		{0, 0, 0, 0}
	};

	while(1){
		int option_index = 0;
		int c = getopt_long(argc, argv, "hnp:", long_options, &option_index);

		if(c == -1) break; // Detect the end of the options.

		switch(c){
		case 0:
			// for long args without short equivalent that just set a flag
			// nothing left to do so just break.
			if (long_options[option_index].flag != 0) break;
			break;

		case 'h':
			_print_usage();
			return -1;

		case 'n':
			en_newline = 1;
			break;

		case 'p':
			if(pipe_expand_location_string(optarg, pipe_path)<0){
				fprintf(stderr, "Invalid pipe name: %s\n", optarg);
				return -1;
			}
			break;
		
		case 't':
			test_mode = true;
			break;

		default:
			_print_usage();
			return -1;
		}
	}

	return 0;
}


int main(int argc, char* argv[])
{
	// check for options
	if(_parse_opts(argc, argv)) return -1;

	// set some basic signal handling for safe shutdown.
	// quitting without cleanup up the pipe can result in the pipe staying
	// open and overflowing, so always cleanup properly!!!
	enable_signal_handler();
	main_running = 1;

	// prints can be quite long, disable terminal wrapping
	printf(DISABLE_WRAP);

	// set up all our MPA callbacks
	pipe_client_set_simple_helper_cb(0, _helper_cb, NULL);
	pipe_client_set_connect_cb(0, _connect_cb, NULL);
	pipe_client_set_disconnect_cb(0, _disconnect_cb, NULL);

	// request a new pipe from the server
	printf("waiting for server\n");
	int ret = pipe_client_open(0, pipe_path, CLIENT_NAME, \
				EN_PIPE_CLIENT_SIMPLE_HELPER, \
				RANGEFINDER_RECOMMENDED_READ_BUF_SIZE);

	// check for MPA errors
	if(ret<0){
		pipe_print_error(ret);
		printf(ENABLE_WRAP);
		return -1;
	}

	// keep going until signal handler sets the running flag to 0
	if (test_mode){
		printf(FONT_BOLD);
		printf("   id  |");
		printf("latency(ms)|");
		printf("distances (m)");
		printf("\n");
		printf(RESET_FONT);
	}

	while(main_running){
		if(test_mode){
			pipe_client_wait(0, PIPE_CLIENT_READ, 1000);
			test_passed = 1;
		}
		else{
			usleep(200000);
		}
	}

	// all done, signal pipe read threads to stop
	printf("\nclosing and exiting\n");
	pipe_client_close_all();

	if(test_mode){
		if(test_passed){
			printf("\n\nTEST PASSED\n");
			return 0;
		}
		else{
			printf("\n\nTEST FAILED\n");
			return -1;
		}
	}

	printf(ENABLE_WRAP);

	return 0;
}
