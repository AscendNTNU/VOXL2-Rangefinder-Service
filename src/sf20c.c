
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <voxl_io/i2c.h>
#include "sf20c.h"
#include "config_file.h" // for bus global variable
#include "vl53l1x_registers.h"

#define SF20C_LOWEST_ACCEPTABLE_SIGNAL 5

static int en_debug = 0;


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



static int sf20c_write_reg_byte(uint16_t reg, uint8_t data)
{
	return voxl_i2c_reg16_write_bytes(bus, _reverse_lsb_msb_16(reg), 1, &data);
}

static int sf20c_write_reg_word(uint16_t reg, uint16_t data)
{
	uint8_t buf[2];
	buf[0] = data >> 8;
	buf[1] = data & 0x00FF;
	return voxl_i2c_reg16_write_bytes(bus, _reverse_lsb_msb_16(reg), 2, buf);
}

static int sf20c_write_reg_int(uint16_t reg, uint32_t data)
{
	uint8_t buf[4];
	buf[0] = (data >> 24) & 0xFF;
	buf[1] = (data >> 16) & 0xFF;
	buf[2] = (data >> 8)  & 0xFF;
	buf[3] = (data >> 0)  & 0xFF;
	return voxl_i2c_reg16_write_bytes(bus, _reverse_lsb_msb_16(reg), 4, buf);
}


static int sf20c_read_reg_bytes(uint16_t reg, uint8_t* data, int bytes)
{
	int ret;
	ret = voxl_i2c_reg16_read_bytes(bus, _reverse_lsb_msb_16(reg), bytes, data);
	if(ret!=bytes) return -1;
	return 0;
}

static int sf20c_read_reg_byte(uint16_t reg, uint8_t* data)
{
	int ret;
	ret = voxl_i2c_reg16_read_bytes(bus, _reverse_lsb_msb_16(reg), 1, data);
	if(ret!=1) return -1;
	return 0;
}


static int sf20c_read_reg_word(uint16_t reg, uint16_t* data)
{
	int ret;
	uint8_t buf[2];
	ret = voxl_i2c_reg16_read_bytes(bus, _reverse_lsb_msb_16(reg), 2, buf);
	if(ret!=2) return -1;
	*data = (buf[0] << 8) + buf[1];
	return 0;
}


static int sf20c_set_address(uint8_t addr)
{
	return sf20c_write_reg_byte(VL53L1_I2C_SLAVE__DEVICE_ADDRESS, addr);
}


int sf20c_start_ranging(void)
{
	return sf20c_write_reg_byte(SYSTEM__MODE_START, 0x40); /* Enable VL53L1X */
}

int vl53l1x_stop_ranging(void)
{
	return sf20c_write_reg_byte(SYSTEM__MODE_START, 0x00); /* Disable VL53L1X */
}

int sf20c_clear_interrupt(void)
{
	return sf20c_write_reg_byte(SYSTEM__INTERRUPT_CLEAR, 0x01);
}

int sf20c_check_for_data_ready(uint8_t *isDataReady)
{
	uint8_t Temp;

	if(sf20c_read_reg_byte(GPIO__TIO_HV_STATUS, &Temp)){
		return -1;
	}

	if(Temp & 1){
		*isDataReady = 1;
	}
	else{
		*isDataReady = 0;
	}
	return 0;
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

	// one-shot read of all data
	static const uint16_t base = VL53L1_RESULT__INTERRUPT_STATUS;
	static const uint8_t n_bytes = 16; // up to the corrected range_mm register
	uint8_t all_data[n_bytes];
	if(sf20c_read_reg_bytes(VL53L1_RESULT__INTERRUPT_STATUS, all_data, n_bytes)){
		fprintf(stderr, "ERROR bulk reading status\n");
		return -1;
	}

	// first check raw status register
	uint8_t status_raw = all_data[VL53L1_RESULT__RANGE_STATUS-base] & 0x1F;
	// convert from raw register to a real status code.
	uint8_t status = 255;
	static const uint8_t status_rtn[24] = {\
				255, 255, 255, 5, 2, 4, 1, 7, 3, 0,
				255, 255, 9, 13, 255, 255, 255, 255, 10, 6,
				255, 255, 11, 12 };
	if(status_raw < 24) status = status_rtn[status_raw];

	// range
	int offset = VL53L1_RESULT__FINAL_CROSSTALK_CORRECTED_RANGE_MM_SD0-base;
	uint16_t dist_mm_raw = (all_data[offset]<<8) + all_data[offset+1];


	// experiment applying gain factor (made it worse)
	// int32_t range_mm = tmp;
	// range_mm *= 2011;
	// range_mm += 0x0400;
	// range_mm /= 0x0800;
	// *dist_mm = range_mm;

	// read signal strength
	offset = VL53L1_RESULT__PEAK_SIGNAL_COUNT_RATE_MCPS_SD0 - base;
	uint16_t signal = (all_data[offset]<<8) + all_data[offset+1];

	offset = VL53L1_RESULT__SIGMA_SD0 - base;
	uint16_t sigma_mm = (all_data[offset]<<8) + all_data[offset+1];
	sigma_mm /= 4; // register is in 14.2 fixed point so discard the fraction bits


	if(en_debug){
		printf("mm:%5d ", dist_mm_raw);
		printf("signal:%6d ", signal);
		printf("SD:%5d ", sigma_mm);
		_print_status(status);
	}


	// allow "good" and "low signal" readings through, we check signal strength ourselves
	if(status!=0 && status!=2) return 0;

	// Sensor reads uint16_max sometimes on bad readings, discard these too
	if(dist_mm_raw > 8000) return 0;

	// signal == 0 is definitely a bad reading. also drop borderline values
	if(signal < VL53L1X_LOWEST_ACCEPTABLE_SIGNAL) return 0;

	*dist_mm = dist_mm_raw;
	*sd = sigma_mm;

	return 0;
}


int sf20c_check_whoami(int quiet)
{
	//read WHOAMI register
	uint16_t id;
	int ret = sf20c_read_reg_word(VL53L1_IDENTIFICATION__MODEL_ID, &id);
	if(ret<0){
		if(!quiet){
			fprintf(stderr, "ERROR in %s, failed to read whoami register\n", __FUNCTION__);
		}
		return -2;
	}
	if(en_debug){
		printf("read whoami reg 0x%04x = 0x%04x\n", VL53L1_IDENTIFICATION__MODEL_ID, id);
	}
	if(id != 0xEACC){
		fprintf(stderr, "ERROR in %s, invalid whoami register\n", __FUNCTION__);
		fprintf(stderr, "read 0x%X, expected 0xEACC\n", id);
		return -1;
	}
	return 0;
}



// argument is the index of this sensor in the enabled_sensors array
int sf20c_init(float fov_deg, int TimingBudgetInMs)
{
	if(sf20c_check_whoami(0)){
		fprintf(stderr, "ERROR in %s, failed to verify whoami\n", __FUNCTION__);
		return -1;
	}
	if(en_debug){
		printf("initializing a sensor\n");
	}

	// load in default settings
	uint8_t Addr = 0x00;
	for (Addr = 0x2D; Addr <= 0x87; Addr++){
		vl53l1x_write_reg_byte(Addr, VL51L1X_DEFAULT_CONFIGURATION[Addr - 0x2D]);
	}

	// set to long distance mode
	sf20c_write_reg_byte(PHASECAL_CONFIG__TIMEOUT_MACROP, 0x0A);
	sf20c_write_reg_byte(RANGE_CONFIG__VCSEL_PERIOD_A, 0x0F);
	sf20c_write_reg_byte(RANGE_CONFIG__VCSEL_PERIOD_B, 0x0D);
	sf20c_write_reg_byte(RANGE_CONFIG__VALID_PHASE_HIGH, 0xB8);
	sf20c_write_reg_word(SD_CONFIG__WOI_SD0, 0x0F0D);
	sf20c_write_reg_word(SD_CONFIG__INITIAL_PHASE_SD0, 0x0E0E);


	switch(TimingBudgetInMs)
	{
		case 20:
			sf20c_write_reg_word(RANGE_CONFIG__TIMEOUT_MACROP_A_HI,0x001E);
			sf20c_write_reg_word(RANGE_CONFIG__TIMEOUT_MACROP_B_HI,0x0022);
			break;
		case 33:
			sf20c_write_reg_word(RANGE_CONFIG__TIMEOUT_MACROP_A_HI,0x0060);
			sf20c_write_reg_word(RANGE_CONFIG__TIMEOUT_MACROP_B_HI,0x006E);
			break;
		case 50:
			sf20c_write_reg_word(RANGE_CONFIG__TIMEOUT_MACROP_A_HI,0x00AD);
			sf20c_write_reg_word(RANGE_CONFIG__TIMEOUT_MACROP_B_HI,0x00C6);
			break;
		case 100:
			sf20c_write_reg_word(RANGE_CONFIG__TIMEOUT_MACROP_A_HI,0x01CC);
			sf20c_write_reg_word(RANGE_CONFIG__TIMEOUT_MACROP_B_HI,0x01EA);
			break;
		case 200:
			sf20c_write_reg_word(RANGE_CONFIG__TIMEOUT_MACROP_A_HI,0x02D9);
			sf20c_write_reg_word(RANGE_CONFIG__TIMEOUT_MACROP_B_HI,0x02F8);
			break;
		case 500:
			sf20c_write_reg_word(RANGE_CONFIG__TIMEOUT_MACROP_A_HI,0x048F);
			sf20c_write_reg_word(RANGE_CONFIG__TIMEOUT_MACROP_B_HI,0x04A4);
			break;
		default:
			fprintf(stderr, "invalid timing budget\n");
			return -1;
	}

	// set optical center to the middle
	sf20c_write_reg_byte(ROI_CONFIG__USER_ROI_CENTRE_SPAD, 199);

	// pick correct SPAD size between 4x4 to 16x16 for desired fov
	// also set the FOV that will actually be set in the enabled_sensors struct
	// so that this number will be sent out the pipe instead of what was
	// in the config file
	uint8_t pads = 4;
	if(fov_deg >= 26.125f){
		fov_deg = 27.0f;
		pads = 16;
	}
	else if(fov_deg >= 24.375f){
		fov_deg = 25.25f;
		pads = 14;
	}
	else if(fov_deg >= 22.625f){
		fov_deg = 23.5f;
		pads = 12;
	}
	else if(fov_deg >= 20.875f){
		fov_deg = 21.75f;
		pads = 10;
	}
	else if(fov_deg >= 18.75f){
		fov_deg = 20.0f;
		pads = 8;
	}
	else if(fov_deg >= 16.25f){
		fov_deg = 17.5f;
		pads = 6;
	}
	else{
		fov_deg = 15.0f;
		pads = 4;
	}

	if(en_debug){
		printf("using %2d pads, for a diagonal fov of %6.1f deg\n", pads, (double)fov_deg);
	}

	sf20c_write_reg_byte(ROI_CONFIG__USER_ROI_REQUESTED_GLOBAL_XY_SIZE,  (pads-1)<<4 | (pads-1));


	// stuff for automatic intermeasurement period, not used here
	uint16_t ClockPLL;
	uint16_t intermeasurement_time_ms = 30;
	sf20c_read_reg_word(VL53L1_RESULT__OSC_CALIBRATE_VAL, &ClockPLL);
	ClockPLL = ClockPLL & 0x3FF;
	sf20c_write_reg_int(VL53L1_SYSTEM__INTERMEASUREMENT_PERIOD,
				   (uint32_t)(ClockPLL * intermeasurement_time_ms * 1.075));

	if(en_debug){
		printf("done initializing a sensor\n");
	}

	return 0;
}


int sf20c_wait_for_data(void)
{
	for(int i=0; i<20; i++){
		uint8_t isDataReady = 0;
		if(sf20c_check_for_data_ready(&isDataReady)){
			fprintf(stderr, "failed to check data ready\n");
			return -1;
		}
		if(en_debug) printf("data ready: %d i=%d\n", isDataReady, i);
		if(isDataReady) return 0;
		usleep(5000);
	}

	// timeout
	return -1;
}


// this assumes mux is off and we can only see one sensor
int sf20c_set_bus_to_default_slave_address(void)
{
	// check whoami at default address first
	if(voxl_i2c_set_device_address(bus, SF20C_TOF_DEFAULT_ADDR)){
		fprintf(stderr, "failed to set i2c slave config on bus %d, address %d\n",
											bus, SF20C_TOF_DEFAULT_ADDR);
		return -1;
	}
	return 0;
}


