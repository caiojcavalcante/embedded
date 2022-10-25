/* main.c - Hello World demo */

/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <string.h>


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
struct data_item_type {
    uint32_t field1;
};

//create message queue
K_MSGQ_DEFINE(my_msgqA, sizeof(struct data_item_type), 10, 4);
K_MSGQ_DEFINE(my_msgqB, sizeof(struct data_item_type), 10, 4);

K_THREAD_STACK_DEFINE(threadA_stack_area, STACKSIZE);
static struct k_thread threadA_data;

K_THREAD_STACK_DEFINE(threadB_stack_area, STACKSIZE);
static struct k_thread threadB_data;

/* threadA is a static thread that is spawned automatically */

void threadA(void *dummy1, void *dummy2, void *dummy3)
{
    struct data_item_type data;
    while(1)
    {
        //get the item data
        k_msgq_get(&my_msgqA, &data, K_FOREVER);
        k_msleep(1000);

        //process data
        printk("%s: message received with content\n", "Thread A");

        k_msgq_put(&my_msgqB, &data, K_NO_WAIT);
    }

}

void threadB(void *dummy1, void *dummy2, void *dummy3)
{
    uint8_t data;
    while(1)
    {
        //get the item data
        k_msgq_get(&my_msgqB, &data, K_FOREVER);

        k_msleep(1000);
        //process data
        printk("%s: message received with content\n", "Thread B");
        k_msgq_put(&my_msgqA, &data, K_NO_WAIT);
    }
}

void main(void)
{
    struct data_item_type data;

    data.field1 = 1;


    // configure leds
    gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);

    k_thread_create(&threadA_data, threadA_stack_area,
                    K_THREAD_STACK_SIZEOF(threadA_stack_area),
                    threadA, NULL, NULL, NULL,
                    PRIORITY, 0, K_FOREVER);
    k_thread_name_set(&threadA_data, "thread_a");

    k_thread_create(&threadB_data, threadB_stack_area,
                    K_THREAD_STACK_SIZEOF(threadB_stack_area),
                    threadB, NULL, NULL, NULL,
                    PRIORITY, 0, K_FOREVER);
    k_thread_name_set(&threadB_data, "thread_b");

    k_thread_start(&threadA_data);
    k_thread_start(&threadB_data);

    k_msgq_put(&my_msgqB, &data, K_NO_WAIT);

    while (1){
        k_msleep(100);
    }
}