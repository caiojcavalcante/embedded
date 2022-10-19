/*
 * Copyright (c) 2016 Open-RnD Sp. z o.o.
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>

#define SLEEP_TIME_MS	200
#define DEBOUNCE_TIME_MS 10

/*
 * Get button configuration from the devicetree sw0 alias. This is mandatory.
 */
#define SW0_NODE	DT_ALIAS(sw0)
#if !DT_NODE_HAS_STATUS(SW0_NODE, okay)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});
static struct gpio_callback button_cb_data;

/*
 * The led0 devicetree alias is optional. If present, we'll use it
 * to turn on the LED whenever the button is pressed.
 */
static struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {0});
static struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led1), gpios, {0});
static struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led2), gpios, {0});
static struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led3), gpios, {0});

void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
}

void toggle()
{
	static int state = 0;
	state = !state;
	gpio_pin_set_dt(&led0, state);
}

void toggle_led_party(int *is_party_on)
{
	static int state = 0;
	state = !state;
	*is_party_on = state;
}

bool is_pressed()
{
	return gpio_pin_get_dt(&button) == 0;
}

void main(void)
{
	int ret;

	if (!device_is_ready(led0.port) || !device_is_ready(led1.port) || !device_is_ready(led2.port) || !device_is_ready(led3.port)) {
		return;
	}
	
	ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_LOW);
	if (ret < 0) return;

	ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_LOW);
	if (ret < 0) return;

	ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_LOW);
	if (ret < 0) return;

	ret = gpio_pin_configure_dt(&led3, GPIO_OUTPUT_LOW);
	if (ret < 0) return;

	if (!device_is_ready(button.port)) {
		printk("Error: button device %s is not ready\n",
		       button.port->name);
		return;
	}

	
	gpio_pin_set_dt(&led0, 0);
	gpio_pin_set_dt(&led1, 0);
	gpio_pin_set_dt(&led2, 0);
	gpio_pin_set_dt(&led3, 0);

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);

	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n",
		       ret, button.port->name, button.pin);
		return;
	}

	ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, button.port->name, button.pin);
		return;
	}

	gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
	gpio_add_callback(button.port, &button_cb_data);

	// if (led.port && !device_is_ready(led.port)) {
	// 	printk("Error %d: LED device %s is not ready; ignoring it\n",
	// 	       ret, led.port->name);
	// 	led.port = NULL;
	// }
	
	// if (led.port) {
	// 	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT);
	// 	if (ret != 0) {
	// 		printk("Error %d: failed to configure LED device %s pin %d\n",
	// 		       ret, led.port->name, led.pin);
	// 		led.port = NULL;
	// 	} else {
	// 		printk("Set up LED at %s pin %d\n", led.port->name, led.pin);
	// 	}
	// }


	printk("Press the button\n");

	bool party_on = false;

	bool button_last_state = 0;

	short iterator = 0;
	while (1) {
		/* If we have an LED, match its state to the button's. */
		bool button_state = is_pressed();
		bool button1 = gpio_pin_get_dt(&button1) == 0;

		if (party_on)
		{
			iterator = (iterator + 1) % 4;
			switch(iterator)
			{
				case 0:
					gpio_pin_toggle_dt(&led0);
					break;
				case 1:
					gpio_pin_toggle_dt(&led1);
					break;
				case 2:
					gpio_pin_toggle_dt(&led3);
					break;
				case 3:
					gpio_pin_toggle_dt(&led2);
					break;
				default:
					break;
			}
			k_sleep(K_MSEC(100));
		}
		else
		{
			gpio_pin_set_dt(&led0, 0);
			gpio_pin_set_dt(&led1, 0);
			gpio_pin_set_dt(&led2, 0);
			gpio_pin_set_dt(&led3, 0);
		}

		if (button_state && button_state != button_last_state) {
			//DEBOUNCE
			k_msleep(DEBOUNCE_TIME_MS);
			if(is_pressed())
			{
				// toggle(); //toggle led0
				toggle_led_party(&party_on);
			}
		}
		button_last_state = button_state;

	}
}
