#include "battery.h"
#include "ble_hid.h"
#include "app_watchdog.h"
#include "factory_reset.h"
#include "input_scanner.h"
#include "power.h"

#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/services/bas.h>
#include <zephyr/bluetooth/services/hids.h>

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL,
        BT_UUID_16_ENCODE(BT_UUID_HIDS_VAL),
        BT_UUID_16_ENCODE(BT_UUID_BAS_VAL)),
};

static const struct bt_data sd[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME,
            sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

static void advertising_start(void)
{
    int err = bt_le_adv_start(
        BT_LE_ADV_CONN_FAST_1,
        ad, ARRAY_SIZE(ad),
        sd, ARRAY_SIZE(sd));

    if (err && err != -EALREADY) {
        LOG_ERR("Advertising failed: %d", err);
    } else {
        LOG_INF("Advertising");
    }
}

static void connected(struct bt_conn *conn, uint8_t err)
{
    if (err) {
        LOG_WRN("Connection failed: 0x%02x", err);
        advertising_start();
        return;
    }

    ble_hid_connected(conn);
    power_on_connected();
    LOG_INF("Connected");
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    ARG_UNUSED(conn);

    LOG_INF("Disconnected: 0x%02x", reason);
    ble_hid_disconnected();
    power_on_disconnected();
    advertising_start();
}

static bool le_param_req(struct bt_conn *conn, struct bt_le_conn_param *param)
{
    ARG_UNUSED(conn);

    param->interval_min = 6;
    param->interval_max = 12;
    param->latency = 30;
    param->timeout = 400;

    return true;
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
    .le_param_req = le_param_req,
};

static void auth_cancel(struct bt_conn *conn)
{
    ARG_UNUSED(conn);
    LOG_WRN("Pairing cancelled");
}

static void pairing_complete(struct bt_conn *conn, bool bonded)
{
    ARG_UNUSED(conn);
    LOG_INF("Pairing complete, bonded=%d", bonded);
}

static void pairing_failed(struct bt_conn *conn, enum bt_security_err reason)
{
    ARG_UNUSED(conn);
    LOG_WRN("Pairing failed: %d", reason);
}

static struct bt_conn_auth_cb auth_cb = {
    .cancel = auth_cancel,
};

static struct bt_conn_auth_info_cb auth_info_cb = {
    .pairing_complete = pairing_complete,
    .pairing_failed = pairing_failed,
};

static void input_event_handler(uint8_t hid_keycode, bool pressed)
{
    int err = ble_hid_send_key_state(hid_keycode, pressed);

    if (err == -ENOTCONN) {
        LOG_DBG("Ignoring input while disconnected");
    } else if (err) {
        LOG_WRN("HID input send failed: %d", err);
    }
}

int main(void)
{
    int err;

    LOG_INF("Booting BLE HID keyboard");

    err = bt_enable(NULL);
    if (err) {
        LOG_ERR("Bluetooth init failed: %d", err);
        return 0;
    }

    if (IS_ENABLED(CONFIG_SETTINGS)) {
        settings_load();
    }

    err = factory_reset_check_boot_request();
    if (err) {
        LOG_WRN("Factory-reset check failed: %d", err);
    }

    bt_conn_auth_cb_register(&auth_cb);
    bt_conn_auth_info_cb_register(&auth_info_cb);

    err = ble_hid_init();
    if (err) {
        LOG_ERR("HID init failed: %d", err);
        return 0;
    }

    battery_service_init();
    power_init();

    err = input_scanner_init(input_event_handler);
    if (err) {
        LOG_ERR("Input scanner init failed: %d", err);
        return 0;
    }

    err = app_watchdog_init();
    if (err) {
        LOG_ERR("Watchdog init failed: %d", err);
        return 0;
    }

    advertising_start();

    while (true) {
        k_sleep(K_SECONDS(1));
        app_watchdog_feed();
        power_maintenance_run();
        battery_level_update();
    }
}
