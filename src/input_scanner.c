#include "input_scanner.h"

#include <errno.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(input_scanner, LOG_LEVEL_INF);

#define DEBOUNCE_MS 20

struct input_button {
    const struct gpio_dt_spec spec;
    struct gpio_callback callback;
    struct k_work_delayable debounce_work;
    uint8_t hid_keycode;
    bool stable_pressed;
    bool configured;
};

static input_scanner_event_handler_t event_handler;

#define BUTTON_COUNT                                                   \
    (DT_NODE_HAS_STATUS(DT_ALIAS(sw0), okay) +                         \
     DT_NODE_HAS_STATUS(DT_ALIAS(sw1), okay) +                         \
     DT_NODE_HAS_STATUS(DT_ALIAS(sw2), okay) +                         \
     DT_NODE_HAS_STATUS(DT_ALIAS(sw3), okay))

#define BUTTON_FROM_ALIAS(alias_name, keycode)                          \
    COND_CODE_1(DT_NODE_HAS_STATUS(DT_ALIAS(alias_name), okay),          \
        ({                                                              \
            .spec = GPIO_DT_SPEC_GET(DT_ALIAS(alias_name), gpios),       \
            .hid_keycode = keycode,                                     \
        },),                                                            \
        ())

#if BUTTON_COUNT > 0
static struct input_button buttons[] = {
    BUTTON_FROM_ALIAS(sw0, 0x04)
    BUTTON_FROM_ALIAS(sw1, 0x05)
    BUTTON_FROM_ALIAS(sw2, 0x06)
    BUTTON_FROM_ALIAS(sw3, 0x07)
};
#endif

static bool button_is_pressed(const struct input_button *button)
{
#if BUTTON_COUNT > 0
    int value = gpio_pin_get_dt(&button->spec);

    if (value < 0) {
        LOG_WRN("GPIO read failed: %d", value);
        return button->stable_pressed;
    }

    return value > 0;
#else
    ARG_UNUSED(button);
    return false;
#endif
}

static void debounce_work_handler(struct k_work *work)
{
    struct k_work_delayable *delayable = k_work_delayable_from_work(work);
    struct input_button *button = CONTAINER_OF(delayable,
                                               struct input_button,
                                               debounce_work);
    bool pressed = button_is_pressed(button);

    if (pressed == button->stable_pressed) {
        return;
    }

    button->stable_pressed = pressed;

    if (event_handler) {
        event_handler(button->hid_keycode, pressed);
    }
}

static void gpio_callback_handler(const struct device *dev,
                                  struct gpio_callback *cb,
                                  uint32_t pins)
{
    struct input_button *button = CONTAINER_OF(cb, struct input_button, callback);

    ARG_UNUSED(dev);
    ARG_UNUSED(pins);

    k_work_reschedule(&button->debounce_work, K_MSEC(DEBOUNCE_MS));
}

int input_scanner_init(input_scanner_event_handler_t handler)
{
    int err;

    event_handler = handler;

#if BUTTON_COUNT == 0
    LOG_WRN("No sw0-sw3 devicetree aliases found; input scanner idle");
    return 0;
#else
    if (ARRAY_SIZE(buttons) == 0) {
        LOG_WRN("No sw0-sw3 devicetree aliases found; input scanner idle");
        return 0;
    }

    for (size_t i = 0; i < ARRAY_SIZE(buttons); i++) {
        struct input_button *button = &buttons[i];

        if (!gpio_is_ready_dt(&button->spec)) {
            LOG_ERR("GPIO device is not ready");
            return -ENODEV;
        }

        err = gpio_pin_configure_dt(&button->spec, GPIO_INPUT);
        if (err) {
            LOG_ERR("GPIO configure failed: %d", err);
            return err;
        }

        button->stable_pressed = button_is_pressed(button);
        k_work_init_delayable(&button->debounce_work, debounce_work_handler);

        gpio_init_callback(&button->callback, gpio_callback_handler,
                           BIT(button->spec.pin));

        err = gpio_add_callback(button->spec.port, &button->callback);
        if (err) {
            LOG_ERR("GPIO callback add failed: %d", err);
            return err;
        }

        err = gpio_pin_interrupt_configure_dt(&button->spec,
                                              GPIO_INT_EDGE_BOTH);
        if (err) {
            LOG_ERR("GPIO interrupt configure failed: %d", err);
            return err;
        }

        button->configured = true;
    }

    LOG_INF("Input scanner ready with %u button(s)",
            (unsigned int)ARRAY_SIZE(buttons));
    return 0;
#endif
}
