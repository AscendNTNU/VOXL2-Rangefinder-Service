/*******************************************************************************
 Copyright Ã‚Â© 2018, STMicroelectronics International N.V.
 All rights reserved.
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of STMicroelectronics nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND
 NON-INFRINGEMENT OF INTELLECTUAL PROPERTY RIGHTS ARE DISCLAIMED.
 IN NO EVENT SHALL STMICROELECTRONICS INTERNATIONAL N.V. BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#ifndef VL53L1X_REGISTERS_H
#define VL53L1X_REGISTERS_H



#define VL53L1X_IMPLEMENTATION_VER_MAJOR					1
#define VL53L1X_IMPLEMENTATION_VER_MINOR					0
#define VL53L1X_IMPLEMENTATION_VER_SUB						1
#define VL53L1X_IMPLEMENTATION_VER_REVISION					0000

#define SOFT_RESET											0x0000
#define VL53L1_I2C_SLAVE__DEVICE_ADDRESS					0x0001
#define VL53L1_VHV_CONFIG__TIMEOUT_MACROP_LOOP_BOUND		0x0008
#define ALGO__CROSSTALK_COMPENSATION_PLANE_OFFSET_KCPS		0x0016
#define ALGO__CROSSTALK_COMPENSATION_X_PLANE_GRADIENT_KCPS	0x0018
#define ALGO__CROSSTALK_COMPENSATION_Y_PLANE_GRADIENT_KCPS	0x001A
#define ALGO__PART_TO_PART_RANGE_OFFSET_MM					0x001E
#define MM_CONFIG__INNER_OFFSET_MM							0x0020
#define MM_CONFIG__OUTER_OFFSET_MM							0x0022
#define GPIO_HV_MUX__CTRL									0x0030
#define GPIO__TIO_HV_STATUS									0x0031
#define SYSTEM__INTERRUPT_CONFIG_GPIO						0x0046
#define PHASECAL_CONFIG__TIMEOUT_MACROP						0x004B
#define RANGE_CONFIG__TIMEOUT_MACROP_A_HI					0x005E
#define RANGE_CONFIG__VCSEL_PERIOD_A						0x0060
#define RANGE_CONFIG__VCSEL_PERIOD_B						0x0063
#define RANGE_CONFIG__TIMEOUT_MACROP_B_HI					0x0061
#define RANGE_CONFIG__TIMEOUT_MACROP_B_LO					0x0062
#define RANGE_CONFIG__SIGMA_THRESH							0x0064
#define RANGE_CONFIG__MIN_COUNT_RATE_RTN_LIMIT_MCPS			0x0066
#define RANGE_CONFIG__VALID_PHASE_HIGH						0x0069
#define VL53L1_SYSTEM__INTERMEASUREMENT_PERIOD				0x006C
#define SYSTEM__THRESH_HIGH									0x0072
#define SYSTEM__THRESH_LOW									0x0074
#define SD_CONFIG__WOI_SD0									0x0078
#define SD_CONFIG__INITIAL_PHASE_SD0						0x007A
#define ROI_CONFIG__USER_ROI_CENTRE_SPAD					0x007F
#define ROI_CONFIG__USER_ROI_REQUESTED_GLOBAL_XY_SIZE		0x0080
#define SYSTEM__SEQUENCE_CONFIG								0x0081
#define VL53L1_SYSTEM__GROUPED_PARAMETER_HOLD				0x0082
#define SYSTEM__INTERRUPT_CLEAR								0x0086
#define SYSTEM__MODE_START									0x0087
#define VL53L1_RESULT__INTERRUPT_STATUS						0x0088
#define VL53L1_RESULT__RANGE_STATUS							0x0089
#define VL53L1_RESULT__REPORT_STATUS						0x008A
#define VL53L1_RESULT__STREAM_COUNT							0x008B
#define VL53L1_RESULT__DSS_ACTUAL_EFFECTIVE_SPADS_SD0		0x008C
#define VL53L1_RESULT__PEAK_SIGNAL_COUNT_RATE_MCPS_SD0		0x008E
#define VL53L1_RESULT__AMBIENT_COUNT_RATE_MCPS_SD0			0x0090
#define VL53L1_RESULT__SIGMA_SD0							0x0092 //(fixed point 14.2)
#define VL53L1_RESULT__PHASE_SD0							0x0094 //(fixed point 5.11)
#define VL53L1_RESULT__FINAL_CROSSTALK_CORRECTED_RANGE_MM_SD0				0x0096
#define VL53L1_RESULT__PEAK_SIGNAL_COUNT_RATE_CROSSTALK_CORRECTED_MCPS_SD0	0x0098
#define VL53L1_RESULT__OSC_CALIBRATE_VAL					0x00DE
#define VL53L1_FIRMWARE__SYSTEM_STATUS						0x00E5
#define VL53L1_IDENTIFICATION__MODEL_ID						0x010F
#define VL53L1_ROI_CONFIG__MODE_ROI_CENTRE_SPAD				0x013E


#define VL53L1X_DEFAULT_DEVICE_ADDRESS						0x52

typedef int8_t VL53L1_Error;

#define VL53L1_ERROR_NONE                              ((VL53L1_Error)  0)
#define VL53L1_ERROR_CALIBRATION_WARNING               ((VL53L1_Error) - 1)
	/*!< Warning invalid calibration data may be in used
		\a  VL53L1_InitData()
		\a VL53L1_GetOffsetCalibrationData
		\a VL53L1_SetOffsetCalibrationData */
#define VL53L1_ERROR_MIN_CLIPPED                       ((VL53L1_Error) - 2)
	/*!< Warning parameter passed was clipped to min before to be applied */

#define VL53L1_ERROR_UNDEFINED                         ((VL53L1_Error) - 3)
	/*!< Unqualified error */
#define VL53L1_ERROR_INVALID_PARAMS                    ((VL53L1_Error) - 4)
	/*!< Parameter passed is invalid or out of range */
#define VL53L1_ERROR_NOT_SUPPORTED                     ((VL53L1_Error) - 5)
	/*!< Function is not supported in current mode or configuration */
#define VL53L1_ERROR_RANGE_ERROR                       ((VL53L1_Error) - 6)
	/*!< Device report a ranging error interrupt status */
#define VL53L1_ERROR_TIME_OUT                          ((VL53L1_Error) - 7)
	/*!< Aborted due to time out */
#define VL53L1_ERROR_MODE_NOT_SUPPORTED                ((VL53L1_Error) - 8)
	/*!< Asked mode is not supported by the device */
#define VL53L1_ERROR_BUFFER_TOO_SMALL                  ((VL53L1_Error) - 9)
	/*!< ... */
#define VL53L1_ERROR_COMMS_BUFFER_TOO_SMALL            ((VL53L1_Error) - 10)
	/*!< Supplied buffer is larger than I2C supports */
#define VL53L1_ERROR_GPIO_NOT_EXISTING                 ((VL53L1_Error) - 11)
	/*!< User tried to setup a non-existing GPIO pin */
#define VL53L1_ERROR_GPIO_FUNCTIONALITY_NOT_SUPPORTED  ((VL53L1_Error) - 12)
	/*!< unsupported GPIO functionality */
#define VL53L1_ERROR_CONTROL_INTERFACE                 ((VL53L1_Error) - 13)
	/*!< error reported from IO functions */
#define VL53L1_ERROR_INVALID_COMMAND                   ((VL53L1_Error) - 14)
	/*!< The command is not allowed in the current device state
	 *  (power down) */
#define VL53L1_ERROR_DIVISION_BY_ZERO                  ((VL53L1_Error) - 15)
	/*!< In the function a division by zero occurs */
#define VL53L1_ERROR_REF_SPAD_INIT                     ((VL53L1_Error) - 16)
	/*!< Error during reference SPAD initialization */
#define VL53L1_ERROR_GPH_SYNC_CHECK_FAIL               ((VL53L1_Error) - 17)
	/*!<  GPH sync interrupt check fail - API out of sync with device*/
#define VL53L1_ERROR_STREAM_COUNT_CHECK_FAIL           ((VL53L1_Error) - 18)
	/*!<  Stream count check fail - API out of sync with device */
#define VL53L1_ERROR_GPH_ID_CHECK_FAIL                 ((VL53L1_Error) - 19)
	/*!<  GPH ID check fail - API out of sync with device */
#define VL53L1_ERROR_ZONE_STREAM_COUNT_CHECK_FAIL      ((VL53L1_Error) - 20)
	/*!<  Zone dynamic config stream count check failed - API out of sync */
#define VL53L1_ERROR_ZONE_GPH_ID_CHECK_FAIL            ((VL53L1_Error) - 21)
	/*!<  Zone dynamic config GPH ID check failed - API out of sync */

#define VL53L1_ERROR_XTALK_EXTRACTION_NO_SAMPLE_FAIL   ((VL53L1_Error) - 22)
	/*!<  Thrown when run_xtalk_extraction fn has 0 succesful samples
	 * when using the full array to sample the xtalk. In this case there is
	 * not enough information to generate new Xtalk parm info. The function
	 * will exit and leave the current xtalk parameters unaltered */
#define VL53L1_ERROR_XTALK_EXTRACTION_SIGMA_LIMIT_FAIL ((VL53L1_Error) - 23)
	/*!<  Thrown when run_xtalk_extraction fn has found that the
	 * avg sigma estimate of the full array xtalk sample is > than the
	 * maximal limit allowed. In this case the xtalk sample is too noisy for
	 * measurement. The function will exit and leave the current xtalk parameters
	 * unaltered. */


#define VL53L1_ERROR_OFFSET_CAL_NO_SAMPLE_FAIL           ((VL53L1_Error) - 24)
	/*!<  Thrown if there one of stages has no valid offset calibration
	 *    samples. A fatal error calibration not valid */
#define VL53L1_ERROR_OFFSET_CAL_NO_SPADS_ENABLED_FAIL    ((VL53L1_Error) - 25)
	/*!<  Thrown if there one of stages has zero effective SPADS
	 *    Traps the case when MM1 SPADs is zero.
	 *    A fatal error calibration not valid */
#define VL53L1_ERROR_ZONE_CAL_NO_SAMPLE_FAIL             ((VL53L1_Error) - 26)
	/*!<  Thrown if then some of the zones have no valid samples
	 *    A fatal error calibration not valid */

#define VL53L1_ERROR_TUNING_PARM_KEY_MISMATCH             ((VL53L1_Error) - 27)
	/*!<  Thrown if the tuning file key table version does not match with
	 * expected value. The driver expects the key table version to match
	 * the compiled default version number in the define
	 * #VL53L1_TUNINGPARM_KEY_TABLE_VERSION_DEFAULT
	 * */

#define VL53L1_WARNING_REF_SPAD_CHAR_NOT_ENOUGH_SPADS   ((VL53L1_Error) - 28)
	/*!<  Thrown if there are less than 5 good SPADs are available. */
#define VL53L1_WARNING_REF_SPAD_CHAR_RATE_TOO_HIGH      ((VL53L1_Error) - 29)
	/*!<  Thrown if the final reference rate is greater than
	      the upper reference rate limit - default is 40 Mcps.
	      Implies a minimum Q3 (x10) SPAD (5) selected */
#define VL53L1_WARNING_REF_SPAD_CHAR_RATE_TOO_LOW       ((VL53L1_Error) - 30)
	/*!<  Thrown if the final reference rate is less than
	      the lower reference rate limit - default is 10 Mcps.
	      Implies maximum Q1 (x1) SPADs selected */


#define VL53L1_WARNING_OFFSET_CAL_MISSING_SAMPLES       ((VL53L1_Error) - 31)
	/*!<  Thrown if there is less than the requested number of
	 *    valid samples. */
#define VL53L1_WARNING_OFFSET_CAL_SIGMA_TOO_HIGH        ((VL53L1_Error) - 32)
	/*!<  Thrown if the offset calibration range sigma estimate is greater
	 *    than 8.0 mm. This is the recommended min value to yield a stable
	 *    offset measurement */
#define VL53L1_WARNING_OFFSET_CAL_RATE_TOO_HIGH         ((VL53L1_Error) - 33)
	/*!< Thrown when VL53L1_run_offset_calibration()  peak rate is greater
	     than that 50.0Mcps. This is the recommended  max rate to avoid
	     pile-up influencing the offset measurement */
#define VL53L1_WARNING_OFFSET_CAL_SPAD_COUNT_TOO_LOW    ((VL53L1_Error) - 34)
	/*!< Thrown when VL53L1_run_offset_calibration() when one of stages
	     range has less that 5.0 effective SPADS. This is the recommended
	     min value to yield a stable offset */


#define VL53L1_WARNING_ZONE_CAL_MISSING_SAMPLES       ((VL53L1_Error) - 35)
	/*!<  Thrown if one of more of the zones have less than
	      the requested number of valid samples */
#define VL53L1_WARNING_ZONE_CAL_SIGMA_TOO_HIGH        ((VL53L1_Error) - 36)
	/*!<  Thrown if one or more zones have sigma estimate value greater
	 *    than 8.0 mm. This is the recommended min value to yield a stable
	 *    offset measurement */
#define VL53L1_WARNING_ZONE_CAL_RATE_TOO_HIGH         ((VL53L1_Error) - 37)
	/*!< Thrown if one of more zones have  peak rate higher than
	      that 50.0Mcps. This is the recommended  max rate to avoid
	     pile-up influencing the offset measurement */


#define VL53L1_WARNING_XTALK_MISSING_SAMPLES             ((VL53L1_Error) - 38)
	/*!< Thrown to notify that some of the xtalk samples did not yield
	 * valid ranging pulse data while attempting to measure
	 * the xtalk signal in vl53l1_run_xtalk_extract(). This can signify any of
	 * the zones are missing samples, for further debug information the
	 * xtalk_results struct should be referred to. This warning is for
	 * notification only, the xtalk pulse and shape have still been generated
	 */
#define VL53L1_WARNING_XTALK_NO_SAMPLES_FOR_GRADIENT     ((VL53L1_Error) - 39)
	/*!< Thrown to notify that some of teh xtalk samples used for gradient
	 * generation did not yield valid ranging pulse data while attempting to
	 * measure the xtalk signal in vl53l1_run_xtalk_extract(). This can signify
	 * that any one of the zones 0-3 yielded no successful samples. The
	 * xtalk_results struct should be referred to for further debug info.
	 * This warning is for notification only, the xtalk pulse and shape
	 * have still been generated.
	 */
#define VL53L1_WARNING_XTALK_SIGMA_LIMIT_FOR_GRADIENT    ((VL53L1_Error) - 40)
/*!< Thrown to notify that some of the xtalk samples used for gradient
	 * generation did not pass the sigma limit check  while attempting to
	 * measure the xtalk signal in vl53l1_run_xtalk_extract(). This can signify
	 * that any one of the zones 0-3 yielded an avg sigma_mm value > the limit.
	 * The xtalk_results struct should be referred to for further debug info.
	 * This warning is for notification only, the xtalk pulse and shape
	 * have still been generated.
	 */

#define VL53L1_ERROR_NOT_IMPLEMENTED                   ((VL53L1_Error) - 41)
	/*!< Tells requested functionality has not been implemented yet or
	 * not compatible with the device */
#define VL53L1_ERROR_PLATFORM_SPECIFIC_START           ((VL53L1_Error) - 60)
	/*!< Tells the starting code for platform */
/** @} VL53L1_define_Error_group */




const uint8_t VL51L1X_DEFAULT_CONFIGURATION[] = {
	0x00, /* 0x2d : set bit 2 and 5 to 1 for fast plus mode (1MHz I2C), else don't touch */
	0x01, /* 0x2e : bit 0 if I2C pulled up at 1.8V, else set bit 0 to 1 (pull up at AVDD) */
	0x01, /* 0x2f : bit 0 if GPIO pulled up at 1.8V, else set bit 0 to 1 (pull up at AVDD) */
	0x01, /* 0x30 : set bit 4 to 0 for active high interrupt and 1 for active low (bits 3:0 must be 0x1), use SetInterruptPolarity() */
	0x02, /* 0x31 : bit 1 = interrupt depending on the polarity, use CheckForDataReady() */
	0x00, /* 0x32 : not user-modifiable */
	0x02, /* 0x33 : not user-modifiable */
	0x08, /* 0x34 : not user-modifiable */
	0x00, /* 0x35 : not user-modifiable */
	0x08, /* 0x36 : not user-modifiable */
	0x10, /* 0x37 : not user-modifiable */
	0x01, /* 0x38 : not user-modifiable */
	0x01, /* 0x39 : not user-modifiable */
	0x00, /* 0x3a : not user-modifiable */
	0x00, /* 0x3b : not user-modifiable */
	0x00, /* 0x3c : not user-modifiable */
	0x00, /* 0x3d : not user-modifiable */
	0xff, /* 0x3e : not user-modifiable */
	0x00, /* 0x3f : not user-modifiable */
	0x0F, /* 0x40 : not user-modifiable */
	0x00, /* 0x41 : not user-modifiable */
	0x00, /* 0x42 : not user-modifiable */
	0x00, /* 0x43 : not user-modifiable */
	0x00, /* 0x44 : not user-modifiable */
	0x00, /* 0x45 : not user-modifiable */
	0x20, /* 0x46 : interrupt configuration 0->level low detection, 1-> level high, 2-> Out of window, 3->In window, 0x20-> New sample ready , TBC */
	0x0b, /* 0x47 : not user-modifiable */
	0x00, /* 0x48 : not user-modifiable */
	0x00, /* 0x49 : not user-modifiable */
	0x02, /* 0x4a : not user-modifiable */
	0x0a, /* 0x4b : not user-modifiable */
	0x21, /* 0x4c : not user-modifiable */
	0x00, /* 0x4d : not user-modifiable */
	0x00, /* 0x4e : not user-modifiable */
	0x05, /* 0x4f : not user-modifiable */
	0x00, /* 0x50 : not user-modifiable */
	0x00, /* 0x51 : not user-modifiable */
	0x00, /* 0x52 : not user-modifiable */
	0x00, /* 0x53 : not user-modifiable */
	0xc8, /* 0x54 : not user-modifiable */
	0x00, /* 0x55 : not user-modifiable */
	0x00, /* 0x56 : not user-modifiable */
	0x38, /* 0x57 : not user-modifiable */
	0xff, /* 0x58 : not user-modifiable */
	0x01, /* 0x59 : not user-modifiable */
	0x00, /* 0x5a : not user-modifiable */
	0x08, /* 0x5b : not user-modifiable */
	0x00, /* 0x5c : not user-modifiable */
	0x00, /* 0x5d : not user-modifiable */
	0x01, /* 0x5e : not user-modifiable */
	0xdb, /* 0x5f : not user-modifiable */
	0x0f, /* 0x60 : not user-modifiable */
	0x01, /* 0x61 : not user-modifiable */
	0xf1, /* 0x62 : not user-modifiable */
	0x0d, /* 0x63 : not user-modifiable */
	0x01, /* 0x64 : Sigma threshold MSB (mm in 14.2 format for MSB+LSB), use SetSigmaThreshold(), default value 90 mm  */
	0x68, /* 0x65 : Sigma threshold LSB */
	0x00, /* 0x66 : Min count Rate MSB (MCPS in 9.7 format for MSB+LSB), use SetSignalThreshold() */
	0x80, /* 0x67 : Min count Rate LSB */
	0x08, /* 0x68 : not user-modifiable */
	0xb8, /* 0x69 : not user-modifiable */
	0x00, /* 0x6a : not user-modifiable */
	0x00, /* 0x6b : not user-modifiable */
	0x00, /* 0x6c : Intermeasurement period MSB, 32 bits register, use SetIntermeasurementInMs() */
	0x00, /* 0x6d : Intermeasurement period */
	0x0f, /* 0x6e : Intermeasurement period */
	0x89, /* 0x6f : Intermeasurement period LSB */
	0x00, /* 0x70 : not user-modifiable */
	0x00, /* 0x71 : not user-modifiable */
	0x00, /* 0x72 : distance threshold high MSB (in mm, MSB+LSB), use SetD:tanceThreshold() */
	0x00, /* 0x73 : distance threshold high LSB */
	0x00, /* 0x74 : distance threshold low MSB ( in mm, MSB+LSB), use SetD:tanceThreshold() */
	0x00, /* 0x75 : distance threshold low LSB */
	0x00, /* 0x76 : not user-modifiable */
	0x01, /* 0x77 : not user-modifiable */
	0x0f, /* 0x78 : not user-modifiable */
	0x0d, /* 0x79 : not user-modifiable */
	0x0e, /* 0x7a : not user-modifiable */
	0x0e, /* 0x7b : not user-modifiable */
	0x00, /* 0x7c : not user-modifiable */
	0x00, /* 0x7d : not user-modifiable */
	0x02, /* 0x7e : not user-modifiable */
	0xc7, /* 0x7f : ROI center, use SetROI() */
	0xff, /* 0x80 : XY ROI (X=Width, Y=Height), use SetROI() */
	0x9B, /* 0x81 : not user-modifiable */
	0x00, /* 0x82 : not user-modifiable */
	0x00, /* 0x83 : not user-modifiable */
	0x00, /* 0x84 : not user-modifiable */
	0x01, /* 0x85 : not user-modifiable */
	0x00, /* 0x86 : clear interrupt, use ClearInterrupt() */
	0x00  /* 0x87 : start ranging, use StartRanging() or StopRanging(), If you want an automatic start after VL53L1X_init() call, put 0x40 in location 0x87 */
};


#endif // VL53L1X_REGISTERS_H
