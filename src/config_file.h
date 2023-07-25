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

#ifndef CONFIG_FILE_H
#define CONFIG_FILE_H


#include "common.h"


// struct to contain all data from each single tag entry in config file
typedef struct rangefinder_config_t{

	int enabled;					///< a sensor is allowed to be listed and configured but disabled
	int sensor_id;					///< ID of the rangefinder, must be unique
	int type;						///< see voxl_rangefinder_interface.h

	float fov_deg;					///< field of view of the sensor in degrees
	float range_max_m;				///< Maximum range of the sensor in meters

	float location_wrt_body[3];		///< location of the rangefinder with respect to body frame.
	float direction_wrt_body[3];	///< direction vector of the rangefinder with respect to body frame

	int is_on_mux;					// set non-zero to indicate this is connected through an i2c multiplexer
	int i2c_mux_address;			// multiplexer address
	int i2c_mux_port;				// 1-8

} rangefinder_config_t;



// all sensors, including disabled ones
// everything else is just concerned with the enabled_sensors array
extern int n_total_sensors;
extern rangefinder_config_t r[MAX_SENSORS];
extern int vl53l1x_timing_budget_ms;


// all enabled sensors and some easy-access data about them
extern int n_enabled_sensors;
extern rangefinder_config_t enabled_sensors[MAX_SENSORS];

extern int has_nonmux_sensor; // should ideally be the first one!
extern int n_mux_sensors;
extern int mux_address;
extern int bus;


void print_config(void);
int read_config_file(void);
int write_new_config_file_with_defaults(int arrangement);


#endif // end #define CONFIG_FILE_H
