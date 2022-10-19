/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   100

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define LED3_NODE DT_ALIAS(led3)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);

void main(void)
{
	int ret, iterator = 0;

	if (!device_is_ready(led0.port) || !device_is_ready(led1.port) || !device_is_ready(led2.port) || !device_is_ready(led3.port)) {
		return;
	}

	ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) return;

	ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) return;

	ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) return;

	ret = gpio_pin_configure_dt(&led3, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) return;


	// gpio_dt_spec arr[4] = {&led0, &led1, &led2, &led3}

	while (1) {
		
		switch(iterator++ % 4)
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
		// ret0 = gpio_pin_toggle_dt(arr[(iterator++) % 4]);
		k_msleep(SLEEP_TIME_MS);
	}
}
