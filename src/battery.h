#ifndef BATTERY_H_
#define BATTERY_H_

#include <stdint.h>

void battery_service_init(void);
uint8_t battery_level_get(void);
void battery_level_update(void);

#endif
