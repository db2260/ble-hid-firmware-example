#include "app_watchdog.h"

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/watchdog.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app_watchdog, LOG_LEVEL_INF);

#define WATCHDOG_TIMEOUT_MS 4000

#if DT_NODE_HAS_STATUS(DT_ALIAS(watchdog0), okay)
static const struct device *const wdt = DEVICE_DT_GET(DT_ALIAS(watchdog0));
#elif DT_HAS_COMPAT_STATUS_OKAY(nordic_nrf_wdt)
static const struct device *const wdt =
    DEVICE_DT_GET(DT_COMPAT_GET_ANY_STATUS_OKAY(nordic_nrf_wdt));
#else
static const struct device *const wdt;
#endif

static int channel_id = -1;

int app_watchdog_init(void)
{
    struct wdt_timeout_cfg cfg = {
        .window = {
            .min = 0,
            .max = WATCHDOG_TIMEOUT_MS,
        },
        .callback = NULL,
        .flags = WDT_FLAG_RESET_SOC,
    };
    int err;

    if (!wdt || !device_is_ready(wdt)) {
        LOG_WRN("No ready watchdog device; watchdog disabled");
        return 0;
    }

    channel_id = wdt_install_timeout(wdt, &cfg);
    if (channel_id < 0) {
        LOG_ERR("Watchdog timeout install failed: %d", channel_id);
        return channel_id;
    }

    err = wdt_setup(wdt, 0);
    if (err) {
        LOG_ERR("Watchdog setup failed: %d", err);
        return err;
    }

    LOG_INF("Watchdog enabled with %d ms timeout", WATCHDOG_TIMEOUT_MS);
    return 0;
}

void app_watchdog_feed(void)
{
    int err;

    if (!wdt || channel_id < 0) {
        return;
    }

    err = wdt_feed(wdt, channel_id);
    if (err) {
        LOG_WRN("Watchdog feed failed: %d", err);
    }
}
