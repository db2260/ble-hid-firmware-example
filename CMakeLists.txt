cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(prod_ble_hid_keyboard)

target_sources(app PRIVATE
  src/main.c
  src/ble_hid.c
  src/battery.c
  src/input_scanner.c
  src/factory_reset.c
  src/power.c
  src/app_watchdog.c
)
