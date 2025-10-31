#ifndef SF20C_H
#define SF20C_H


#include <voxl_io/i2c.h>
#include <stdint.h>


#define SF20C_TOF_DEFAULT_ADDR 0x66

void sf20c_set_en_debug(int en);

int sf20c_start_ranging(void);

int sf20c_stop_ranging(void);

int sf20c_clear_interrupt(void);

int sf20c_check_for_data_ready(uint8_t *isDataReady);

int sf20c_get_distance_mm(int* dist_mm, int* sd_mm);

int sf20c_set_bus_to_default_slave_address(void);

int sf20c_check_whoami(int quiet);

int sf20c_init(float fov_deg, int TimingBudgetInMs);

int sf20c_wait_for_data(void);

#endif // end #define SF20C_H