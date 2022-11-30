#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define STACK_SIZE 500
#define PRIORITY 5

typedef struct k_mutex mutex;
typedef struct k_thread thread;

mutex mx;

int data = 0;

K_THREAD_STACK_DEFINE(thread_a_stack_area, STACK_SIZE);
static thread thread_a;

K_THREAD_STACK_DEFINE(thread_b_stack_area, STACK_SIZE);
static thread thread_b;

void mtx_func() {
    char* this_thread_name;
    thread *current_thread;
    current_thread = k_current_get();
    this_thread_name = k_thread_name_get(current_thread);

    while(1) {
        if(k_mutex_lock(&mx, K_MSEC(2000)) == 0) {
            data = data + 1;
            printk("%s %d\n", this_thread_name, data);
            k_sleep(K_MSEC(1000));
            k_mutex_unlock(&mx);
        }
    }
}

void thread_a_entry(void *nothing_0, void *nothing_1, void *nothing_2) {
    mtx_func();
}

void thread_b_entry(void *nothing_0, void *nothing_1, void *nothing_2) {
    mtx_func();
}

void main() {
    k_mutex_init(&mx);

    k_thread_create(&thread_a, thread_a_stack_area,
                    K_THREAD_STACK_SIZEOF(thread_a_stack_area),
                    thread_a_entry, NULL, NULL, NULL,
                    PRIORITY, 0, K_FOREVER);
    k_thread_name_set(&thread_a, "thread0");

    k_thread_create(&thread_b, thread_b_stack_area,
                    K_THREAD_STACK_SIZEOF(thread_b_stack_area),
                    thread_b_entry, NULL, NULL, NULL,
                    PRIORITY, 0, K_FOREVER);
    k_thread_name_set(&thread_b, "thread1");

    k_thread_start(&thread_a);
    k_thread_start(&thread_b);
}
