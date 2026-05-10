#include "battery.h"

#include <zephyr/bluetooth/services/bas.h>

static uint8_t battery_level = 100;

void battery_service_init(void)
{
    bt_bas_set_battery_level(battery_level);
}

uint8_t battery_level_get(void)
{
    return battery_level;
}

void battery_level_update(void)
{
    /*
     * Replace this placeholder with a calibrated ADC measurement and filtering.
     * Keep the value stable to avoid noisy BLE notifications.
     */
    bt_bas_set_battery_level(battery_level);
}
