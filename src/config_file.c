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
#include <unistd.h>		// for access()
#include <modal_json.h>

#include <voxl_rangefinder_interface.h>
#include "common.h"
#include "config_file.h"

#define CONFIG_FILE_PATH	"/etc/modalai/voxl-rangefinder-server.conf"
#define SIN45 0.707106781186547524400844362105



// all sensors, including disabled ones
// everything else is just concerned with the enabled_sensors array
int n_total_sensors;
rangefinder_config_t r[MAX_SENSORS];


// all enabled sensors and some easy-access data about them
int n_enabled_sensors;
rangefinder_config_t enabled_sensors[MAX_SENSORS];

int has_nonmux_sensor = 0;
int n_mux_sensors = 0;
int mux_address = 0;
int bus;



#define CONFIG_FILE_HEADER "\
/**\n\
 * Rangefinder Configuration File\n\
 * This file is used by voxl-rangefinder-server\n\
 * please use voxl-rangefinder-server --config {arrangement}\n\
 * to set up this file.\n\
 *\n\
 * FOV for VL53l1X TOF rangefinder is a diagonal FOV in degrees and\n\
 * can be set between 15 and 27 degrees.\n\
 */\n"


// return a default config struct. used in other functions to save space and remain
// consistent as to what the defaults are.
static rangefinder_config_t _get_default_config(void)
{
	rangefinder_config_t r;

	r.enabled = 1;
	r.sensor_id = 0;
	r.type = RANGEFINDER_TYPE_TOF_VL53L1X;
	r.i2c_address = VL53L1X_TOF_DEFAULT_ADDR;
	r.uncertainty_m = 0.02;
	r.fov_deg = 27;
	r.range_max_m = 3.0;
	r.location_wrt_body[0] = 0.0;
	r.location_wrt_body[1] = 0.0;
	r.location_wrt_body[2] = 0.0;
	r.direction_wrt_body[0] = 0.0;
	r.direction_wrt_body[1] = 0.0;
	r.direction_wrt_body[2] = 0.0;
	r.is_on_mux = 1;
	r.i2c_mux_address = TCA9548A_MUX_DEFAULT_ADDR;
	r.i2c_mux_port = 0;

	return r;
}


void print_config(void)
{
	int i,j;
	const char* type_strings[] = RANGEFINDER_TYPE_STRINGS;

	printf("i2c_bus: %d\n", bus);

	for(i=0; i<n_total_sensors; i++){
		printf("#%d:\n",i);
		printf("    enabled:               %d\n", r[i].enabled);
		printf("    sensor_id:             %d\n", r[i].sensor_id);
		printf("    type:                  %s\n", type_strings[r[i].type]);
		printf("    i2c_address:           0x%X\n", r[i].i2c_address);

		printf("    uncertainty_m:         %0.3f\n", (double)r[i].uncertainty_m);
		printf("    fov_deg:               %0.3f\n", (double)r[i].fov_deg);
		printf("    range_max_m:           %0.3f\n", (double)r[i].range_max_m);

		printf("    location_wrt_body:     ");
		for(j=0;j<3;j++) printf("%0.1f ", (double)r[i].location_wrt_body[j]);
		printf("\n");
		printf("    direction_wrt_body:    ");
		for(j=0;j<3;j++) printf("%0.1f ", (double)r[i].direction_wrt_body[j]);
		printf("\n");

		printf("    is_on_mux:             %d\n", r[i].is_on_mux);
		printf("    i2c_mux_address:       0x%X\n", r[i].i2c_mux_address);
		printf("    i2c_mux_port:          %d\n", r[i].i2c_mux_port);

		printf("\n");
	}
	return;
}



int read_config_file()
{
	// vars and defaults
	int i;
	const char* type_strings[] = RANGEFINDER_TYPE_STRINGS;
	rangefinder_config_t default_r = _get_default_config();

	// set number of sensors to 0 at first in case there is an error
	n_total_sensors = 0;
	n_enabled_sensors = 0;

	// check file exists
	if(access(CONFIG_FILE_PATH, F_OK) == -1){
		printf("no config file found, please run voxl-configure-rangefinders\n");
		return -1;
	}

	// read the data in
	cJSON* parent = json_read_file(CONFIG_FILE_PATH);
	if(parent==NULL){
		printf("error reading config file, please run voxl-configure-rangefinders\n");
		return -1;
	}

	// file structure is just one big array of rangefinder_config_t structures
	cJSON* json_array = json_fetch_array_and_add_if_missing(parent, "sensors", &n_total_sensors);
	if(n_total_sensors > MAX_SENSORS){
		fprintf(stderr, "ERROR found %d sensors in file but maximum number is %d\n", n_total_sensors, MAX_SENSORS);
		return -1;
	}

	// no sensors found
	if(n_total_sensors == 0){
		printf("no sensors found in config file, please run voxl-configure-rangefinders\n");
		return -1;
	}

	// for now, the i2c bus is the only thing not in the array
	json_fetch_int_with_default(parent, "i2c_bus", &bus, 1);

	// copy out each item in the array
	for(i=0; i<n_total_sensors; i++){
		cJSON* json_item = cJSON_GetArrayItem(json_array, i);

		json_fetch_bool_with_default(json_item, "enabled", &r[i].enabled, default_r.enabled);
		json_fetch_int_with_default(json_item, "sensor_id", &r[i].sensor_id, i);
		json_fetch_enum_with_default(json_item, "type",	&r[i].type, type_strings, N_RANGEFINDER_TYPES, default_r.type);
		json_fetch_int_with_default(json_item, "i2c_address", &r[i].i2c_address, default_r.i2c_address);

		json_fetch_float_with_default(json_item, "uncertainty_m", &r[i].uncertainty_m, default_r.uncertainty_m);
		json_fetch_float_with_default(json_item, "fov_deg", &r[i].fov_deg, default_r.fov_deg);
		json_fetch_float_with_default(json_item, "range_max_m", &r[i].range_max_m, default_r.range_max_m);

		json_fetch_fixed_vector_float_with_default(json_item, "location_wrt_body",	r[i].location_wrt_body, 3,	default_r.location_wrt_body);
		json_fetch_fixed_vector_float_with_default(json_item, "direction_wrt_body",	r[i].direction_wrt_body, 3,	default_r.direction_wrt_body);

		json_fetch_bool_with_default(json_item, "is_on_mux", &r[i].is_on_mux, default_r.is_on_mux);
		json_fetch_int_with_default(json_item, "i2c_mux_address", &r[i].i2c_mux_address, default_r.i2c_mux_address);
		json_fetch_int_with_default(json_item, "i2c_mux_port", &r[i].i2c_mux_port, default_r.i2c_mux_port);
	}

	// check if we got any errors in that process
	if(json_get_parse_error_flag()){
		fprintf(stderr, "failed to parse data in %s\n", CONFIG_FILE_PATH);
		cJSON_Delete(parent);
		return -1;
	}

	// write modified data to disk if neccessary
	if(json_get_modified_flag()){
		printf("The JSON data was modified during parsing, saving the changes to disk\n");
		json_write_to_file_with_header(CONFIG_FILE_PATH, parent, CONFIG_FILE_HEADER);
	}
	cJSON_Delete(parent);


	// now go through the sensors to figure out the higher level information
	for(i=0; i<n_total_sensors; i++){

		// keep an array of just the enabled sensors to read from later
		if(r[i].enabled){
			n_enabled_sensors++;
			enabled_sensors[n_enabled_sensors-1] = r[i];
		}

		// flag if we have an oddball sensor not on multiplexer
		if(!r[i].is_on_mux && r[i].enabled){
			has_nonmux_sensor = 1;
		}

		// make sure mux port is in 0-7
		if(r[i].is_on_mux && r[i].enabled){
			if(r[i].i2c_mux_port<0 || r[i].i2c_mux_port>7){
				fprintf(stderr, "ERROR reading config file, i2c_mux_port must be in 0-7\n");
				return -1;
			}
			n_mux_sensors++;
			mux_address = r[i].i2c_mux_address;
		}
	}

	return 0;
}










// used when constructing default sensor configurations
static int _add_rangefinder_config_to_json(rangefinder_config_t* r, int n, int bus, cJSON* parent)
{
	int i,m;
	const char* type_strings[] = RANGEFINDER_TYPE_STRINGS;

	cJSON_AddNumberToObject(parent, "i2c_bus", bus);
	cJSON* json_array = json_fetch_array_and_add_if_missing(parent, "sensors", &m);

	for(i=0;i<n;i++){
		cJSON* json_item = cJSON_CreateObject();
		cJSON_AddItemToArray(json_array, json_item);

		cJSON_AddBoolToObject(json_item, "enabled", r[i].enabled);
		cJSON_AddNumberToObject(json_item, "sensor_id", r[i].sensor_id);
		cJSON_AddStringToObject(json_item, "type", type_strings[r[i].type]);
		cJSON_AddNumberToObject(json_item, "i2c_address", r[i].i2c_address);

		cJSON_AddNumberToObject(json_item, "uncertainty_m", r[i].uncertainty_m);
		cJSON_AddNumberToObject(json_item, "fov_deg", r[i].fov_deg);
		cJSON_AddNumberToObject(json_item, "range_max_m", r[i].range_max_m);

		cJSON_AddItemToObject(json_item, "location_wrt_body", cJSON_CreateFloatArray(r[i].location_wrt_body, 3));
		cJSON_AddItemToObject(json_item, "direction_wrt_body", cJSON_CreateFloatArray(r[i].direction_wrt_body, 3));

		cJSON_AddBoolToObject(json_item, "is_on_mux", r[i].is_on_mux);
		cJSON_AddNumberToObject(json_item, "i2c_mux_address", r[i].i2c_mux_address);
		cJSON_AddNumberToObject(json_item, "i2c_mux_port", r[i].i2c_mux_port);
	}

	return 0;
}

#define RANGEFINDER_ARRANGEMENT_1_TOF_ON_M0141	1 // for testing without multiplexer
#define RANGEFINDER_ARRANGEMENT_4_TOF_ON_M0141	2 // for Nokia Starling

int write_new_config_file_with_defaults(int arrangement)
{
	int bus,i;
	int n_sensors;
	rangefinder_config_t r[MAX_SENSORS];

	switch(arrangement){

		case RANGEFINDER_ARRANGEMENT_1_TOF_ON_M0141:

			printf("creating new config file for 1 TOF without multiplexer\n");
			r[0] = _get_default_config();
			r[0].is_on_mux = 0;
			bus = 1;
			n_sensors = 1;
			break;

		case RANGEFINDER_ARRANGEMENT_4_TOF_ON_M0141:

			printf("creating new config file for 4 TOF sensors on Starling with M0141\n");

			bus = 1;
			n_sensors = 4;

			// configure common values
			for(i=0;i<n_sensors;i++){
				r[i] = _get_default_config();
				r[i].sensor_id = i;
				r[i].i2c_mux_address = TCA9548A_MUX_DEFAULT_ADDR;
				r[i].i2c_mux_port = i;
			}

			// LEFT
			r[0].location_wrt_body[0]  =  0.0;
			r[0].location_wrt_body[1]  =  0.0;
			r[0].location_wrt_body[2]  =  0.0;
			r[0].direction_wrt_body[0] =  0.0;
			r[0].direction_wrt_body[1] = -1.0;
			r[0].direction_wrt_body[2] =  0.0;

			// RIGHT
			r[1].location_wrt_body[0]  =  0.0;
			r[1].location_wrt_body[1]  =  0.0;
			r[1].location_wrt_body[2]  =  0.0;
			r[1].direction_wrt_body[0] =  0.0;
			r[1].direction_wrt_body[1] =  1.0;
			r[1].direction_wrt_body[2] =  0.0;

			// BACK LEFT (rough guess, need to make accurate later)
			r[2].location_wrt_body[0]  =  0.0;
			r[2].location_wrt_body[1]  =  0.0;
			r[2].location_wrt_body[2]  =  0.0;
			r[2].direction_wrt_body[0] = -0.974;
			r[2].direction_wrt_body[1] = -0.225;
			r[2].direction_wrt_body[2] =  0.0;

			r[3].location_wrt_body[0]  =  0.0;
			r[3].location_wrt_body[1]  =  0.0;
			r[3].location_wrt_body[2]  =  0.0;
			r[3].direction_wrt_body[0] = -0.974;
			r[3].direction_wrt_body[1] =  0.225;
			r[3].direction_wrt_body[2] =  0.0;

			break;

		default:
			fprintf(stderr, "ERROR in %s, invalid arrangement\n", __FUNCTION__);
			return -1;
	}

	cJSON* parent = cJSON_CreateObject();
	_add_rangefinder_config_to_json(r,n_sensors,bus, parent);
	json_write_to_file_with_header(CONFIG_FILE_PATH, parent, CONFIG_FILE_HEADER);
	cJSON_Delete(parent);

	printf("DONE\n");

	return 0;
}

