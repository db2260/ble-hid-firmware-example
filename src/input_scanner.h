#ifndef INPUT_SCANNER_H_
#define INPUT_SCANNER_H_

#include <stdbool.h>
#include <stdint.h>

typedef void (*input_scanner_event_handler_t)(uint8_t hid_keycode, bool pressed);

int input_scanner_init(input_scanner_event_handler_t handler);

#endif
