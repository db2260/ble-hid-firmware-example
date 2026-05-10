#ifndef BLE_HID_H_
#define BLE_HID_H_

#include <stdint.h>
#include <zephyr/bluetooth/conn.h>

#define BLE_HID_KEY_A_USAGE 0x04

int ble_hid_init(void);
void ble_hid_connected(struct bt_conn *conn);
void ble_hid_disconnected(void);
int ble_hid_send_key_click(uint8_t keycode);

#endif
