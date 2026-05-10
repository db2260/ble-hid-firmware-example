#include "ble_hid.h"

#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/services/hids.h>

LOG_MODULE_REGISTER(ble_hid, LOG_LEVEL_INF);

#define INPUT_REPORT_ID_KEYBOARD 1
#define OUTPUT_REPORT_ID_KEYBOARD 1

static struct bt_conn *current_conn;
static struct bt_hids hids;

static const uint8_t hid_report_map[] = {
    0x05, 0x01,       /* Usage Page Generic Desktop */
    0x09, 0x06,       /* Usage Keyboard */
    0xA1, 0x01,       /* Collection Application */

    0x85, INPUT_REPORT_ID_KEYBOARD,
    0x05, 0x07,       /* Usage Page Keyboard */
    0x19, 0xE0,
    0x29, 0xE7,
    0x15, 0x00,
    0x25, 0x01,
    0x75, 0x01,
    0x95, 0x08,
    0x81, 0x02,       /* Modifier byte */

    0x95, 0x01,
    0x75, 0x08,
    0x81, 0x01,       /* Reserved byte */

    0x95, 0x06,
    0x75, 0x08,
    0x15, 0x00,
    0x25, 0x65,
    0x05, 0x07,
    0x19, 0x00,
    0x29, 0x65,
    0x81, 0x00,       /* Key array */

    0x85, OUTPUT_REPORT_ID_KEYBOARD,
    0x95, 0x05,
    0x75, 0x01,
    0x05, 0x08,
    0x19, 0x01,
    0x29, 0x05,
    0x91, 0x02,       /* LED output report */

    0x95, 0x01,
    0x75, 0x03,
    0x91, 0x01,

    0xC0
};

static void keyboard_leds_written(struct bt_hids_rep *rep,
                                  struct bt_conn *conn,
                                  bool write)
{
    ARG_UNUSED(rep);
    ARG_UNUSED(conn);
    ARG_UNUSED(write);

    LOG_DBG("Keyboard LED state changed");
}

int ble_hid_init(void)
{
    static struct bt_hids_inp_rep input_reports[] = {
        {
            .id = INPUT_REPORT_ID_KEYBOARD,
            .size = 8,
        },
    };

    static struct bt_hids_outp_feat_rep output_reports[] = {
        {
            .id = OUTPUT_REPORT_ID_KEYBOARD,
            .size = 1,
            .handler = keyboard_leds_written,
        },
    };

    struct bt_hids_init_param init = {0};

    init.rep_map.data = hid_report_map;
    init.rep_map.size = sizeof(hid_report_map);
    init.info.bcd_hid = 0x0111;
    init.info.b_country_code = 0x00;
    init.info.flags = BT_HIDS_REMOTE_WAKE | BT_HIDS_NORMALLY_CONNECTABLE;
    init.is_kb = true;
    init.is_mouse = false;
    init.inp_rep_group_init.reports = input_reports;
    init.inp_rep_group_init.cnt = ARRAY_SIZE(input_reports);
    init.outp_feat_rep_group_init.reports = output_reports;
    init.outp_feat_rep_group_init.cnt = ARRAY_SIZE(output_reports);

    return bt_hids_init(&hids, &init);
}

void ble_hid_connected(struct bt_conn *conn)
{
    current_conn = bt_conn_ref(conn);
}

void ble_hid_disconnected(void)
{
    if (current_conn) {
        bt_conn_unref(current_conn);
        current_conn = NULL;
    }
}

static int send_keyboard_report(uint8_t modifier, uint8_t keycode)
{
    uint8_t report[8] = {
        modifier,
        0x00,
        keycode,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    };

    if (!current_conn) {
        return -ENOTCONN;
    }

    return bt_hids_inp_rep_send(&hids, current_conn,
                                INPUT_REPORT_ID_KEYBOARD,
                                report, sizeof(report), NULL);
}

int ble_hid_send_key_click(uint8_t keycode)
{
    int err;

    err = send_keyboard_report(0x00, keycode);
    if (err) {
        return err;
    }

    k_msleep(15);

    return send_keyboard_report(0x00, 0x00);
}

int ble_hid_send_key_state(uint8_t keycode, bool pressed)
{
    return send_keyboard_report(0x00, pressed ? keycode : 0x00);
}
