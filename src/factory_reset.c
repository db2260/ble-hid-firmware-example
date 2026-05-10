#include "factory_reset.h"

#include <errno.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/reboot.h>

LOG_MODULE_REGISTER(factory_reset, LOG_LEVEL_INF);

#define FACTORY_RESET_HOLD_MS 8000
#define FACTORY_RESET_POLL_MS 50

#if DT_NODE_HAS_STATUS(DT_ALIAS(sw0), okay)
static const struct gpio_dt_spec reset_button =
    GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
#endif

int factory_reset_check_boot_request(void)
{
#if DT_NODE_HAS_STATUS(DT_ALIAS(sw0), okay)
    int err;
    int held_ms = 0;

    if (!gpio_is_ready_dt(&reset_button)) {
        LOG_WRN("Factory-reset GPIO is not ready");
        return -ENODEV;
    }

    err = gpio_pin_configure_dt(&reset_button, GPIO_INPUT);
    if (err) {
        LOG_WRN("Factory-reset GPIO configure failed: %d", err);
        return err;
    }

    while (gpio_pin_get_dt(&reset_button) > 0) {
        if (held_ms >= FACTORY_RESET_HOLD_MS) {
            LOG_WRN("Factory reset requested; clearing BLE bonds");
            err = bt_unpair(BT_ID_DEFAULT, NULL);
            if (err) {
                LOG_ERR("Bond clear failed: %d", err);
                return err;
            }

            k_sleep(K_MSEC(250));
            sys_reboot(SYS_REBOOT_COLD);
        }

        k_sleep(K_MSEC(FACTORY_RESET_POLL_MS));
        held_ms += FACTORY_RESET_POLL_MS;
    }

    return 0;
#else
    LOG_INF("No sw0 alias; factory reset button disabled");
    return 0;
#endif
}
