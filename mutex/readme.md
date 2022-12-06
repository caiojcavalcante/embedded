# Why to use a mutex? 
When using threads, sometimes, the threads can try to access the same data at the same time, which can cause a deadlock, for that reason, there is a tool called mutex *(short for mutual exclusion)*, which guarantees only one thread have access to certain data.

#How does it work?

Before a thread can access the data, the thread has to lock the mutex, if the mutex is already locked, the thread will not access it, if the thread can successfuly lock the mutex, the thread can proceed to work and then unlock it. 


# Mutex sample

##### Let's have some dummy data and define threads that will manipulate that data 
```c
    int data = 0;

    typedef struct k_thread thread;

    static thread thread_a;
    static thread thread_b;

    K_THREAD_STACK_DEFINE(thread_a_stack_area, STACK_SIZE);
    K_THREAD_STACK_DEFINE(thread_b_stack_area, STACK_SIZE);

```
##### Define a function which the threads will run
```c
    void mtx_func() {
        char* this_thread_name;
        thread *current_thread;
        current_thread = k_current_get();
        this_thread_name = k_thread_name_get(current_thread);

        while(1) {
            data++;
            printk("%s %d\n", this_thread_name, data);
            k_sleep(K_MSEC(1000));
        }
    }
```
##### If we stay like this, both threads will try to manipulate the data at the same time, to fix this, we can use a mutex.
```c
typedef struct k_mutex mutex;
mutex mx;
/*supressed code*/
void main() {
    k_mutex_init(&mx);
}
```
##### Now let's make the mtx_func() lock the mutex before manipulating the data
```c
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
```
##### Now let's set the threads entries and start the threads
```c
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
```
