#include "system.h"
#include "altera_up_avalon_accelerometer_spi.h"
#include "altera_avalon_timer_regs.h"
#include "altera_avalon_timer.h"
#include "altera_avalon_pio_regs.h"
#include "sys/alt_irq.h"
#include <stdlib.h>
#include "alt_types.h"
#include "sys/times.h"

#define OFFSET -32
#define PWM_PERIOD 16
#define TAPS 51         // Number of FIR filter taps
#define SCALE_FACTOR 256 // Scaling factor for fixed-point representation

alt_8 pwm = 0;
alt_u8 led;
int level;

// FIR Filter Coefficients (Fixed-Point Scaled from MATLAB)
const alt_32 coeffs[TAPS] = {
    1, 2, -1, -2, 1, 0, -3, 1, 1, -4, 1, 3, -5, 0, 6, -6, -3, 10, -7, -8, 19,
    -7, -31, 74, 165, 74, -31, -7, 19, -8, -7, 10, -3, -6, 6, 0, -5, 3, 1, -4,
    1, 1, -3, 0, 1, -2, -1, 2, 1
};

alt_32 x_read[TAPS] = {0}; // Circular buffer for input samples

// LED Write Function
void led_write(alt_u8 led_pattern) {
    IOWR(LED_BASE, 0, led_pattern);
}

// Convert Raw Accelerometer Readings
void convert_read(alt_32 acc_read, int *level, alt_u8 *led) {
    acc_read += OFFSET;
    alt_u8 val = (acc_read >> 6) & 0x07;
    *led = (8 >> val) | (8 << (8 - val));
    *level = (acc_read >> 1) & 0x1F;
}

// Fixed-Point FIR Low-Pass Filter
alt_32 LPF(alt_32 *acc_read) {
    alt_32 output = 0;
    for (int i = 0; i < TAPS; i++) {
        output += acc_read[i] * coeffs[i];
    }
    return output >> 8;  // Scaling down the result to match the original signal
}

// System Timer ISR (For LED Updates)
void sys_timer_isr() {
    IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_BASE, 0);

    if (pwm < abs(level)) {
        if (level < 0) {
            led_write(led << 1);
        } else {
            led_write(led >> 1);
        }
    } else {
        led_write(led);
    }

    if (pwm > PWM_PERIOD) {
        pwm = 0;
    } else {
        pwm++;
    }
}

// Timer Initialization
void timer_init(void *isr) {
    IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_BASE, 0x0003);
    IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_BASE, 0);
    IOWR_ALTERA_AVALON_TIMER_PERIODL(TIMER_BASE, 0x0900);
    IOWR_ALTERA_AVALON_TIMER_PERIODH(TIMER_BASE, 0x0000);
    alt_irq_register(TIMER_IRQ, 0, isr);
    IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_BASE, 0x0007);
}

// Main Function
int main() {
    int i = 0;  // Circular buffer index
    alt_up_accelerometer_spi_dev *acc_dev;

    //Timing vars
    clock_t exec_t1, exec_t2;

    // Open Accelerometer SPI Device
    acc_dev = alt_up_accelerometer_spi_open_dev("/dev/accelerometer_spi");
    if (acc_dev == NULL) {
        return 1;  // Error opening accelerometer
    }

    timer_init(sys_timer_isr);

    // Main Loop
    while (1) {
    	exec_t1 = times(NULL); // Start timing before reading the accelerometer data
        // Read Accelerometer X-Axis Data
        alt_up_accelerometer_spi_read_x_axis(acc_dev, &x_read[i]);

        alt_32 filtered_output = LPF(x_read);

        convert_read(filtered_output, &level, &led);

        i = (i + 1) % TAPS;

        exec_t2 = times(NULL); //End time after processing

        printf(" proc time = %ld ticks \n", (long)(exec_t2-exec_t1));
    }

    return 0;
}
