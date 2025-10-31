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

#include <c_library_v2/common/mavlink.h>
#include <modal_pipe_client.h>
#include <voxl_rangefinder_interface.h>

#include "mavlink.h"
#include "common.h"
#include "config_file.h"

// keep track of the autopilot sysid
static uint8_t current_sysid = 0;



// called whenever we disconnect from the server
static void _disconnect_cb(__attribute__((unused)) int ch, __attribute__((unused)) void* context)
{
	printf("mavlink pipe disconnected\n");
	return;
}

// called whenever we disconnect from the server
static void _connect_cb(__attribute__((unused)) int ch, __attribute__((unused)) void* context)
{
	printf("mavlink pipe connected\n");
	return;
}

static void _data_from_autopilot_helper_cb(__attribute__((unused))int ch, char* data, \
							int bytes, __attribute__((unused)) void* context)
{
	// validate that the data makes sense
	int n_packets;
	mavlink_message_t* msg_array = pipe_validate_mavlink_message_t(data, bytes, &n_packets);
	if(msg_array == NULL){
		return;
	}

	// scrape every message for our own use, then decide if we should forward it
	for(int i=0; i<n_packets; i++){

		mavlink_message_t* msg = &msg_array[i];

		// always monitor the autopilot sysid in case it changes which may happen
		// during setup and config
		if(msg->compid == MAV_COMP_ID_AUTOPILOT1 && msg->sysid != current_sysid){
			current_sysid = msg->sysid;
			printf("Detected Autopilot Mavlink SYSID %d\n", current_sysid);
		}
	}

	return;
}


int mavlink_start(void)
{
	pipe_client_set_connect_cb(MAV_PIPE_CH, _connect_cb, NULL);
	pipe_client_set_disconnect_cb(MAV_PIPE_CH, _disconnect_cb, NULL);
	pipe_client_set_simple_helper_cb(MAV_PIPE_CH, _data_from_autopilot_helper_cb, NULL);
	pipe_client_open(MAV_PIPE_CH, MAVLINK_PIPE_NAME, PROCESS_NAME, \
					EN_PIPE_CLIENT_SIMPLE_HELPER | EN_PIPE_CLIENT_AUTO_RECONNECT, \
									MAVLINK_MESSAGE_T_RECOMMENDED_READ_BUF_SIZE);

	return 0;
}

int mavlink_stop(void)
{
	pipe_client_close(MAV_PIPE_CH);
	return 0;
}

// publish a single reading as a DOWNWARD distance sensor
int mavlink_publish(rangefinder_data_t d) // This looks good -Peter L
{
	if(!pipe_client_is_connected(MAV_PIPE_CH)){
		return 0;
	}

	uint32_t time_boot_ms = 0; // PX4 ignores this right now
	uint16_t min_distance = 0;
	uint16_t max_distance = d.range_max_m*100;
	uint16_t current_distance = d.distance_m*100;
	uint8_t type = MAV_DISTANCE_SENSOR_INFRARED;
	uint8_t id = 0;
	uint8_t orientation = MAV_SENSOR_ROTATION_PITCH_270;
	uint8_t covariance = UINT8_MAX;
	float horizontal_fov = d.fov_deg * 3.14159f / 180.0f; // to radians
	float vertical_fov = horizontal_fov;
	float quaternion[4] = {NAN, NAN, NAN, NAN};
	uint8_t signal_quality = 100;

	if(d.distance_m<0){
		signal_quality = -1; // indicate invalid signal
	}

	mavlink_message_t msg;
	mavlink_msg_distance_sensor_pack(current_sysid, \
									MAV_COMP_ID_VISUAL_INERTIAL_ODOMETRY, \
									&msg, \
									time_boot_ms, \
									min_distance, \
									max_distance, \
									current_distance, \
									type, \
									id, \
									orientation, \
									covariance, \
									horizontal_fov, \
									vertical_fov, \
									quaternion, \
									signal_quality);

	pipe_client_send_control_cmd_bytes(MAV_PIPE_CH, &msg, sizeof(mavlink_message_t));
	return 0;
}


