#include "power.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/pm/pm.h>

LOG_MODULE_REGISTER(power, LOG_LEVEL_INF);

void power_init(void)
{
    LOG_INF("Power policy initialized");
}

void power_on_connected(void)
{
    LOG_INF("Connected power profile active");
}

void power_on_disconnected(void)
{
    LOG_INF("Advertising power profile active");
}

void power_maintenance_run(void)
{
    /*
     * Put board-specific work here: quiesce unused sensors, lower scan rates,
     * sample battery less often while idle, and gate external regulators.
     */
}
