# BLE HID Keyboard Example

This is a Zephyr / nRF Connect SDK example for a BLE HID keyboard-style device.

## Features

- BLE HID over GATT keyboard report map
- Bonding, settings persistence, and BLE privacy
- Battery service integration
- Connection callbacks and advertising restart
- Explicit press/release HID reports
- Debounced GPIO input scanner using `sw0`-`sw3` devicetree aliases
- Long-press factory reset path for clearing BLE bonds
- Hardware watchdog setup and feed policy
- Power management hooks
- MCUmgr/SMP over BLE OTA/DFU configuration
- Qualification checklist for host interoperability and reliability testing

## Layout

```text
ble-hid-firmware-example/
  CMakeLists.txt
  prj.conf
  README.md
  boards/
    nrf52840dk_nrf52840.overlay
  src/
    main.c
    ble_hid.c
    ble_hid.h
    battery.c
    battery.h
    input_scanner.c
    input_scanner.h
    factory_reset.c
    factory_reset.h
    power.c
    power.h
    app_watchdog.c
    app_watchdog.h
  tests/
    qualification_checklist.md
```

## Build

From an initialized Zephyr or nRF Connect SDK environment:

```sh
west build -b nrf52840dk_nrf52840 .
west flash
```

Adjust the board name for your target.

## Hardware Assumptions

The input scanner uses Zephyr devicetree aliases:

- `sw0`: sends keyboard `A`, also acts as factory-reset button at boot
- `sw1`: sends keyboard `B`
- `sw2`: sends keyboard `C`
- `sw3`: sends keyboard `D`

Boards without those aliases still build; missing buttons are skipped. For a
keyboard matrix, replace `input_scanner.c` with row/column scanning while keeping
the same `input_scanner_event_handler_t` boundary.

## Factory Reset

Hold `sw0` during boot for 8 seconds. The firmware clears all BLE bonds for the
default identity and reboots. In a shipping product, combine this with an LED or
haptic acknowledgement so users know the reset was accepted.

## OTA/DFU

This template enables MCUboot and MCUmgr image upload over BLE. A production
product also needs:

- Signed images and key management
- Partition layout reviewed for the exact flash size
- Upgrade, rollback, and interrupted-transfer testing
- A host-side release process using `mcumgr` or your mobile app

Example host flow after building a signed image:

```sh
mcumgr --conntype ble --connstring peer_name="ProdBLE Keyboard" image upload build/zephyr/app_update.bin
mcumgr --conntype ble --connstring peer_name="ProdBLE Keyboard" image list
mcumgr --conntype ble --connstring peer_name="ProdBLE Keyboard" image confirm
mcumgr --conntype ble --connstring peer_name="ProdBLE Keyboard" reset
```

## Production Notes

Before shipping, replace placeholder battery measurement with calibrated ADC
logic, review power states against your board schematic, tune connection
parameters against latency/current targets, and run the qualification checklist in
`tests/qualification_checklist.md`.
