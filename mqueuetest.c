/* main.c - Hello World demo */

/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>


#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)

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

//create leds
static struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {0});
static struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led1), gpios, {0});

//create data structure for message queue
struct data_item_type{
	uint32_t field1;
	uint32_t field2;
	uint32_t field3;
};

//create message queue
K_MSGQ_DEFINE(my_msgq, sizeof(struct data_item_type), 10, 4);


void sender(int value)
{
	struct data_item_type data;

	while(1)
	{
		data.field1 = value;

	while(k_msgq_put(&my_msgq, &data, K_NO_WAIT) != 0)
	{
		k_msgq_purge(&my_msgq);
	}

	printk("data was successfuly added to message queue\n");
	return;
	}
}

int reader(void)
{
	struct data_item_type data;

	while(1)
	{
		//get the item data
		k_msgq_get(&my_msgq, &data, K_FOREVER);

		//process data
		printk("message received with content %d\n", data.field1);
		return data.field1;
	}
}

void mqueue_loop(const char *my_name, struct gpio_dt_spec *led)
{
	const char *tname;
	uint8_t cpu;
	struct k_thread *current_thread;

	while (1) {
		gpio_pin_toggle_dt(led);

		current_thread = k_current_get();
		tname = k_thread_name_get(current_thread);

        if(strcmp(tname, "thread_a") == 0)
        {
            //se for a thread a
        	sender(1);
        }
        else
        {
            //se for a thread b
        	reader();
        }

		/* wait a while, then let other thread have a turn */
		k_busy_wait(100000);
		k_msleep(SLEEPTIME);
		k_sem_give(other_sem);
	}
}

/* threadB is a dynamic thread that is spawned by threadA */

void threadB(void)
{
	mequeue_loop(__func__, &led1);
}

K_THREAD_STACK_DEFINE(threadA_stack_area, STACKSIZE);
static struct k_thread threadA_data;

K_THREAD_STACK_DEFINE(threadB_stack_area, STACKSIZE);
static struct k_thread threadB_data;

/* threadA is a static thread that is spawned automatically */

void threadA(void)
{
	mequeue_loop(__func__, &led0);
}

void main(void)
{
    // configure leds
	gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
	gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);

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