# What is uart? 
Universal asynchronous receiver/transmitter

[Wikipedia](https://pt.wikipedia.org/wiki/Universal_asynchronous_receiver/transmitter)

It is a protocol for serial communication between systems, it is based on receivers and transmitters often called
rx and tx, very useful when synchronization or data sharing is needed between systems

# How it works?

We must have a message queue, a device with transmitter tx, a device with receiver rx, and a callback function which will
triggered once a message is put in queue.

### Initializing rx, tx and message queue

```c

#define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)

#define MSG_SIZE 32

K_MSGQ_DEFINE(uart_msgq, MSG_SIZE, 10, 4);

static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

static char rx_buf[MSG_SIZE];
static int rx_buf_pos;

void main(void)
{
    char tx_buf[MSG_SIZE];

    if (!device_is_ready(uart_dev)) {
        printk("UART device not found!");
        return;
    }

    uart_irq_callback_user_data_set(uart_dev, serial_cb, NULL);
    uart_irq_rx_enable(uart_dev);
}
```

### Creating callback function

```c
void serial_cb(const struct device *dev, void *user_data)
{
    uint8_t c;

    if (!uart_irq_update(uart_dev)) {
        return;
    }

    while (uart_irq_rx_ready(uart_dev)) {

        uart_fifo_read(uart_dev, &c, 1);

        if ((c == '\n' || c == '\r') && rx_buf_pos > 0) {
            /* terminate string */
            rx_buf[rx_buf_pos] = '\0';

            /* if queue is full, message is silently dropped */
            k_msgq_put(&uart_msgq, &rx_buf, K_NO_WAIT);

            /* reset the buffer (it was copied to the msgq) */
            rx_buf_pos = 0;
        }
        else if (rx_buf_pos < (sizeof(rx_buf) - 1)) {
            rx_buf[rx_buf_pos++] = c;
        }
        /* else: characters beyond buffer size are dropped */
    }
}
```
### Create a function to send a uart message
```c
void print_uart(char *buf)
{
    int msg_len = strlen(buf);

    for (int i = 0; i < msg_len; i++) {
        uart_poll_out(uart_dev, buf[i]);
    }
}
```
### Prompt the other system and echo it's message to itself
```c
void main(void)
{
    char tx_buf[MSG_SIZE];

    if (!device_is_ready(uart_dev)) {
        printk("UART device not found!");
        return;
    }

    uart_irq_callback_user_data_set(uart_dev, serial_cb, NULL);
    uart_irq_rx_enable(uart_dev);

    print_uart("Hello! I'm your echo bot.\r\n");
    print_uart("Tell me something and press enter:\r\n");

    while (k_msgq_get(&uart_msgq, &tx_buf, K_FOREVER) == 0) {
        print_uart("Echo: ");
        print_uart(tx_buf);
        print_uart("\r\n");
    }
}
```