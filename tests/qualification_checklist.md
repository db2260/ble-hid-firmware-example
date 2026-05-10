# BLE HID Qualification Checklist

Run this checklist on the final PCB and release candidate firmware.

## BLE and Pairing

- Fresh pair on Windows, macOS, iOS, Android, and Linux.
- Reconnect after host sleep, device sleep, and RF loss.
- Bond persists across device reset and battery removal.
- Factory reset clears bonds and allows pairing to a new host.
- Privacy enabled; address rotates while preserving bonded reconnect.

## HID Behavior

- Press and release reports are always paired.
- No stuck keys after disconnect, reconnect, low battery, or reset.
- Rollover behavior matches the declared report descriptor.
- Host LED output reports are handled without crashing.
- Input debounce prevents double events across the full voltage range.

## Power and Reliability

- Advertising current, connected idle current, and active typing current measured.
- Watchdog reset recovery verified with an injected main-loop stall.
- Battery service reports stable values under key activity and radio activity.
- Brownout and low-battery behavior verified.
- Long idle and rapid input stress tests run for at least 24 hours.

## OTA/DFU

- Signed upgrade accepted.
- Unsigned or incorrectly signed upgrade rejected.
- Interrupted upload resumes or fails safely.
- Confirmed image survives reboot.
- Rollback path tested.

## Compliance Prep

- GAP appearance and device name match product requirements.
- Connection parameters satisfy latency and current targets.
- GATT database reviewed against HID over GATT requirements.
- RF pre-scan completed before formal regulatory testing.
