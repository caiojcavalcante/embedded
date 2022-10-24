/* main.c - Hello World demo */

/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>

/*
 * The hello world demo has two threads that utilize semaphores and sleeping
 * to take turns printing a greeting message at a controlled rate. The demo
 * shows both the static and dynamic approaches for spawning a thread; a real
 * world application would likely use the static approach for both threads.
 */

#define PIN_THREADS (IS_ENABLED(CONFIG_SMP)		  \
		     && IS_ENABLED(CONFIG_SCHED_CPU_MASK) \
		     && (CONFIG_MP_NUM_CPUS > 1))

/* size of stack area used by each thread */
#define STACKSIZE 1024

/* scheduling priority used by each thread */
#define PRIORITY 7

/* delay between greetings (in ms) */
#define SLEEPTIME 500

#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)

//define os leds
static struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {0});
static struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led1), gpios, {0});

gpio_pin_toggle()

/*
 * @param my_name      thread identification string
 * @param my_sem       thread's own semaphore
 * @param other_sem    other thread's semaphore
 */
void helloLoop(const char *my_name, struct k_sem *my_sem, struct k_sem *other_sem, struct gpio_dt_spec *led)
{
	const char *tname;
	uint8_t cpu = 0;
	struct k_thread *current_thread;

	while (1) {
		/* take my semaphore */
		k_sem_take(my_sem, K_FOREVER);

		current_thread = k_current_get();
		tname = k_thread_name_get(current_thread);

		/* toggle led */
		gpio_pin_toggle(led);

		/* wait a while, then let other thread have a turn */
		k_busy_wait(100000);
		k_msleep(SLEEPTIME);
		k_sem_give(other_sem);
	}
}

/* define semaphores */

K_SEM_DEFINE(threadA_sem, 1, 1);	/* starts off "available" */
K_SEM_DEFINE(threadB_sem, 0, 1);	/* starts off "not available" */


/* threadB is a dynamic thread that is spawned by threadA */

void threadB(void *dummy1, void *dummy2, void *dummy3)
{	/* invoke routine to ping-pong hello messages with threadA */
	helloLoop(__func__, &threadB_sem, &threadA_sem, &led1);
}

K_THREAD_STACK_DEFINE(threadA_stack_area, STACKSIZE);
static struct k_thread threadA_data;

K_THREAD_STACK_DEFINE(threadB_stack_area, STACKSIZE);
static struct k_thread threadB_data;

/* threadA is a static thread that is spawned automatically */

void threadA(void *dummy1, void *dummy2, void *dummy3)
{
	/* invoke routine to ping-pong hello messages with threadB */
	helloLoop(__func__, &threadA_sem, &threadB_sem, &led0);
}

void main(void)
{
    int ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
    if (ret < 0)
        printk("Error %d: failed to configure LED0 pin %d", ret, led0.pin);

    int ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
    if (ret < 0)
        printk("Error %d: failed to configure LED0 pin %d", ret, led0.pin);

	k_thread_create(&threadA_data, threadA_stack_area,
			K_THREAD_STACK_SIZEOF(threadA_stack_area),
			threadA, NULL, NULL, NULL,
			PRIORITY, 0, K_FOREVER);
	k_thread_name_set(&threadA_data, "thread_a");
#if PIN_THREADS
	k_thread_cpu_pin(&threadA_data, 0);
#endif

	k_thread_create(&threadB_data, threadB_stack_area,
			K_THREAD_STACK_SIZEOF(threadB_stack_area),
			threadB, NULL, NULL, NULL,
			PRIORITY, 0, K_FOREVER);
	k_thread_name_set(&threadB_data, "thread_b");
#if PIN_THREADS
	k_thread_cpu_pin(&threadB_data, 1);
#endif

	k_thread_start(&threadA_data);
	k_thread_start(&threadB_data);
}
