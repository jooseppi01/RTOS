#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <stdlib.h> 
#include <zephyr/timing/timing.h>

// Thread initializations
#define STACKSIZE 500
#define PRIORITY 5
#define TIME_LEN_ERROR -1
#define COLOR_ERROR -2

// UART initialization
#define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)
static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

void red_task(void *, void *, void*);
void green_task(void *, void *, void*);
void yellow_task(void *, void*, void*);
void debug_task(void *, void *, void*);

K_THREAD_DEFINE(red_thread,STACKSIZE,red_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(green_thread,STACKSIZE,green_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(yellow_thread,STACKSIZE,yellow_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(debug_thread,STACKSIZE,debug_task,NULL,NULL,NULL,PRIORITY,0,0);

static const struct gpio_dt_spec red = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);

K_FIFO_DEFINE(dispatcher_fifo);
K_FIFO_DEFINE(debug_fifo);

K_FIFO_DEFINE(red_fifo);
K_FIFO_DEFINE(green_fifo);
K_FIFO_DEFINE(yellow_fifo);

K_CONDVAR_DEFINE(red_signal);
K_CONDVAR_DEFINE(green_signal);
K_CONDVAR_DEFINE(yellow_signal);

K_CONDVAR_DEFINE(release_signal);

struct k_mutex state_mutex;  // Mutex for state management

int debug_flag = 0;  // 0 = ei debugia, 1 = debug päällä
#define DEBUG_PRINT(fmt, ...) \
	do { if (debug_flag) printk(fmt, ##__VA_ARGS__); } while (0)

struct debug_t {
	void *fifo_reserved;
	uint64_t time;
	char color;
};

int total_time = 0;

// FIFO dispatcher data type
struct data_t {
    void *fifo_reserved;
	char msg[20];
};

// Struct for time FIFO
struct time_t {
    void *fifo_reserved;
    int time;
};

int init_uart(void) {
	// UART initialization
	if (!device_is_ready(uart_dev)) {
		return 1;
	} 
	return 0;
	
}

void init_leds() {
	gpio_pin_configure_dt(&red, GPIO_OUTPUT_ACTIVE);
	gpio_pin_configure_dt(&green, GPIO_OUTPUT_ACTIVE);

	gpio_pin_set_dt(&red, 0);
	gpio_pin_set_dt(&green, 0);
}

int main(void)
{
	int ret = init_uart();
	if (ret != 0) {
		printk("UART initialization failed!\n");
		return ret;
	}

	init_leds();
	timing_init();
	k_mutex_init(&state_mutex);

	printk("Program started..\n");

	return 0;
}

static void uart_task(void *unused1, void *unused2, void *unused3)
{
	// Received character from UART
	char rc=0;
	// Message from UART
	char uart_msg[20];
	memset(uart_msg,0,20);
	int uart_msg_cnt = 0;

	while (true) {

		// Ask UART if data available
		if (uart_poll_in(uart_dev,&rc) == 0) {

			// If character is not newline, add to UART message buffer
			if (rc != 'X') {
				uart_msg[uart_msg_cnt] = rc;
				uart_msg_cnt++;

			// Character is newline, copy dispatcher data and put to FIFO buffer
			// HOX viesti pitää olla muodossa esim. r1000/n g2000/n y3000
			} else {				
				DEBUG_PRINT("UART msg: %s\n", uart_msg);
				// Allocate memory for dispatcher data
				struct data_t *buf = k_malloc(sizeof(struct data_t));
				if (buf == NULL) {
					return;
				}
				// Copy UART message to dispatcher data
				snprintf(buf->msg, 20, "%s", uart_msg);

				// Put dispatcher data to FIFO buffer
				k_fifo_put(&dispatcher_fifo, buf);
	
				// Clear UART receive buffer
				uart_msg_cnt = 0;
				memset(uart_msg,0,20);
			}
		}
		k_yield();
		k_msleep(10);
	}
	return 0;
}

int time_parse(char *time) {

    // how many seconds, default returns error
    int seconds = TIME_LEN_ERROR;

    // Check that string is not null
    if (time == NULL) {
        return TIME_LEN_ERROR;
    }

	// Check that string length is 6
	if (strlen(time) != 6) {
		return TIME_LEN_ERROR;
	}

	// check that the string is not empty
	if (time[0] == '\0') {
		return TIME_LEN_ERROR;
	}

	// test that the string only contains numbers, using isdigit()
	if (isdigit(time[0]) == 0 || isdigit(time[1]) == 0 || isdigit(time[2]) == 0 || isdigit(time[3]) == 0 || isdigit(time[4]) == 0 || isdigit(time[5]) == 0) {
		return TIME_LEN_ERROR;
	}

    // For example: 124033 -> 12hour 40min 33sec
    int values[3];
    values[2] = atoi(time + 4); // seconds
    time[4] = 0;
    values[1] = atoi(time + 2); // minutes
    time[2] = 0;
    values[0] = atoi(time); // hours
    // Now you have:
    // values[0] hour
    // values[1] minute
    // values[2] second

    if (values[0] < 0 || values[0] > 23 || values[1] < 0 || values[1] > 59 || values[2] < 0 || values[2] > 59) {
        return TIME_LEN_ERROR;
    }

    // Calculate return value from the parsed minutes and seconds
    seconds = (values[1] * 60) + values[2];

	// test if value is 0
	if (seconds == 0) {
		return TIME_LEN_ERROR;
	}

    return seconds;
}

struct UartParser
{
	char color;
	int time;
};

struct UartParser uart_parse(char *input)
{
    // example input: r1000
    struct UartParser result;

    // check that the string length is 5
    if (strlen(input) != 5) {
        result.color = COLOR_ERROR;
        result.time = TIME_LEN_ERROR;
        return result;
    }

    //check that the first character is r, g, or y
    if (input[0] != 'r' && input[0] != 'g' && input[0] != 'y') {
        result.color = COLOR_ERROR;
        result.time = TIME_LEN_ERROR;
        return result;
    }

    // check that the last 4 characters are numbers
    if (isdigit(input[1]) == 0 || isdigit(input[2]) == 0 || isdigit(input[3]) == 0 || isdigit(input[4]) == 0) {
        result.color = COLOR_ERROR;
        result.time = TIME_LEN_ERROR;
        return result;
    }

    char color = input[0];
    int time = atoi(input + 1);

    result.color = color;
    result.time = time;

    return result;
}


static void dispatcher_task(void *unused1, void *unused2, void *unused3)
{
	while (true) {
		// Receive dispatcher data from uart_task fifo
		struct data_t *rec_item = k_fifo_get(&dispatcher_fifo, K_FOREVER);
		char sequence[20];
		memcpy(sequence, rec_item->msg,20);
		k_free(rec_item);

		DEBUG_PRINT("Dispatcher: %s\n", sequence);

		// debug print on or off
		if (sequence[0] == 'd') {
			if (debug_flag == 0) {
				debug_flag = 1;
				printk("Debug print on\n");
			} else {
				debug_flag = 0;
				printk("Debug print off\n");
			}
			k_condvar_signal(&release_signal);
			continue;
		}

		// KÄYTETÄÄN TIME_PARSEA AJASTINKESKEYTYKSEEN... OHJELMA PYSÄHTYY ANNETTUN AJAN JONKA JÄLKEEN PUNAINEN MENEE PÄÄLLE ANNETUN AJAN VERRAN
		if (sequence[0] == '0') {
			int time = time_parse(sequence);
			printk("%dX", time);
			if (time == TIME_LEN_ERROR) {
				continue;
			}
			DEBUG_PRINT("entered time_parse\n");
			int ms_time = time * 1000;
			DEBUG_PRINT("Paused for %d seconds: \n", time);
			k_msleep(ms_time);
			
			DEBUG_PRINT("Red on for the time given\n");
			DEBUG_PRINT("Time in ms: %d\n", ms_time);
			struct time_t *red_item = k_malloc(sizeof(struct time_t));
			if (red_item == NULL) {
				return;
			}

			red_item->time = ms_time;
			k_fifo_put(&red_fifo, red_item);
			k_condvar_signal(&red_signal);
			if (k_condvar_signal(&red_signal) == 0) {
				DEBUG_PRINT("red signal send\n");
			}
			k_condvar_wait(&release_signal, &state_mutex, K_FOREVER);
		}

		else {
			// KÄYTEÄÄN UART_PARSERIA UARTISTA SAADUN DATAN KÄSITTELYYN
			struct UartParser uartti = uart_parse(sequence);
			DEBUG_PRINT("uart_parse vari: %c\n", uartti.color);
			DEBUG_PRINT("uart_parse aika: %d\n", uartti.time);

			char color = uartti.color;
			int time = uartti.time;

			if (color == COLOR_ERROR || time == TIME_LEN_ERROR) {
				DEBUG_PRINT("Invalid color or time format\n");
				continue;
			}

			DEBUG_PRINT("Data: %c %d\n", color, time);	

			// jos aika on 0, ei tehdä mitään ja jatketaan jotta ei jää jumiin		
			if (time == 0) {
				DEBUG_PRINT("No time given \n");
				continue;
			}

			// jos ensimmäinen merkki on r, laita red_fifoon
			else if (color == 'r') {
				struct time_t *red_item = k_malloc(sizeof(struct time_t));
				if (red_item == NULL) {
					return;
				}
				red_item->time = time;
				k_fifo_put(&red_fifo, red_item);
				k_condvar_signal(&red_signal);

			// jos ensimmäinen merkki on g, laita green_fifoon
			} else if (color == 'g') {
				struct time_t *green_item = k_malloc(sizeof(struct time_t));
				if (green_item == NULL) {
					return;
				}
				green_item->time = time;
				k_fifo_put(&green_fifo, green_item);
				k_condvar_signal(&green_signal);
			}

			else if (color == 'y'){
				struct time_t *yellow_item = k_malloc(sizeof(struct time_t));
				if (yellow_item == NULL) {
					return;
				}
				yellow_item->time = time;
				k_fifo_put(&yellow_fifo, yellow_item);
				k_condvar_signal(&yellow_signal);
			}

			else {
				printk("Invalid color\n");
			}

			// Use release signal to control sequence or k_yield
			k_msleep(10);

			k_condvar_wait(&release_signal, &state_mutex, K_FOREVER);
			DEBUG_PRINT("Released\n");
		}
	}
}

void red_task(void *, void *, void *) {
    while (true) {
		// wait for signal
        k_condvar_wait(&red_signal, &state_mutex, K_FOREVER);
		DEBUG_PRINT("red signal got \n");

		// Start timing
		timing_start();
		timing_t start_time = timing_counter_get();

        // get time from red_fifo
        struct time_t *red_item = k_fifo_get(&red_fifo, K_FOREVER);   
        int timee;
		timee = red_item->time;
        k_free(red_item);
		DEBUG_PRINT("Red time: %d\n", timee);

        // Red on
        gpio_pin_set_dt(&red, 1);
		DEBUG_PRINT("Red on\n");
	
        k_msleep(timee);

        gpio_pin_set_dt(&red, 0);
		DEBUG_PRINT("Red off\n");
		
		struct debug_t *buf = k_malloc(sizeof(struct debug_t));
		if (buf == NULL) {
			return;
		}
        
		timing_stop();
		timing_t end_time = timing_counter_get();
        uint64_t diff = timing_cycles_to_ns(timing_cycles_get(&start_time, &end_time));

		buf->time = diff;
		buf->color = 'r';
		k_fifo_put(&debug_fifo, buf);

		// statemutex off
		

        k_condvar_signal(&release_signal);
    }
}

void green_task(void *, void *, void *) {
	while (true) {
		// wait for signal
		k_condvar_wait(&green_signal, &state_mutex, K_FOREVER);
		
		timing_start();
		timing_t start_time = timing_counter_get();

		// get time from green_fifo
		struct time_t *green_item = k_fifo_get(&green_fifo, K_FOREVER);   
		int timee;
		timee = green_item->time;
		k_free(green_item);

		// Green on
		DEBUG_PRINT("Green on\n");
		gpio_pin_set_dt(&green, 1);
		k_msleep(timee);
		
		// set green off
		gpio_pin_set_dt(&green, 0);
		DEBUG_PRINT("Green off\n");

		struct debug_t *buf = k_malloc(sizeof(struct debug_t));
		if (buf == NULL) {
			return;
		}

		timing_stop();
		timing_t end_time = timing_counter_get();
		uint64_t diff = timing_cycles_to_ns(timing_cycles_get(&start_time, &end_time));

		buf->time = diff;
		buf->color = 'g';
		k_fifo_put(&debug_fifo, buf);

		//signal back to dispatcher
		k_condvar_signal(&release_signal);
	}
}

void yellow_task(void *, void *, void *) {
	while (true) {
		// wait for signal
		k_condvar_wait(&yellow_signal, &state_mutex, K_FOREVER);

		timing_start();
		timing_t start_time = timing_counter_get();

		// get time from yellow_fifo
		struct time_t *yellow_item = k_fifo_get(&yellow_fifo, K_FOREVER);   
		int timee;
		timee = yellow_item->time;
		k_free(yellow_item);

		// Yellow on

		DEBUG_PRINT("Yellow on\n");
		gpio_pin_set_dt(&green, 1);
		gpio_pin_set_dt(&red, 1);
		k_msleep(timee);
		
		// set yellow off
		gpio_pin_set_dt(&green, 0);
		gpio_pin_set_dt(&red, 0);
		DEBUG_PRINT("Yellow off\n");

		timing_stop();
		timing_t end_time = timing_counter_get();
		uint64_t diff = timing_cycles_to_ns(timing_cycles_get(&start_time, &end_time));

		struct debug_t *buf = k_malloc(sizeof(struct debug_t));
		if (buf == NULL) {
			return;
		}
		buf->time = diff;
		buf->color = 'y';
		k_fifo_put(&debug_fifo, buf);

		//signal back to dispatcher
		k_condvar_signal(&release_signal);
	}
}

void debug_task(void *, void *, void *)
{
	while (true) {
		
		k_msleep(1000);
		// Store received data
		struct debug_t *received;
		received = k_fifo_get(&debug_fifo, K_FOREVER);

		//time from ns to ms
		int time = received->time / 1000000;
		// totaltime from ns to ms
		total_time = total_time + time;

		// print data
		//printk("Color: %c, Time: %d ms, Total time: %d ms\n", received->color, time, total_time);
		k_free(received);

		k_yield();
	}
}

K_THREAD_DEFINE(dis_thread,STACKSIZE,dispatcher_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(uart_thread,STACKSIZE,uart_task,NULL,NULL,NULL,PRIORITY,0,0);
