# BLE HID Keyboard Example

This is a Zephyr / nRF Connect SDK example for a BLE HID keyboard-style device.

## Features

- BLE HID over GATT keyboard report map
- Bonding, settings persistence, and BLE privacy
- Battery service integration
- Connection callbacks and advertising restart
- Explicit press/release HID reports
- Modular source layout for HID and battery logic

## Layout

```text
ble-hid-firmware-example/
  CMakeLists.txt
  prj.conf
  README.md
  src/
    main.c
    ble_hid.c
    ble_hid.h
    battery.c
    battery.h
```

## Build

From an initialized Zephyr or nRF Connect SDK environment:

```sh
west build -b nrf52840dk_nrf52840 .
west flash
```

Adjust the board name for your target.

## Production Notes

Before shipping, add your real input scanner, hardware-specific power management,
factory reset bond clearing, watchdog feeding policy, OTA/DFU, and qualification
testing for your target host platforms.
