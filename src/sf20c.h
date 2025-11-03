#ifndef SF20C_H
#define SF20C_H


#include <voxl_io/i2c.h>
#include <stdint.h>


#define SF20C_TOF_DEFAULT_ADDR 0x66

void sf20c_set_en_debug(int en);

int sf20c_get_distance_mm(int* dist_mm, int* sd_mm);

int sf20c_set_bus_to_default_slave_address(void);

int sf20c_init(void);

int sf20c_set_distance_output(uint32_t bits);


#endif // end #define SF20C_H