#include "system.h"
#include "altera_up_avalon_accelerometer_spi.h"
#include "altera_avalon_timer_regs.h"
#include "altera_avalon_timer.h"
#include "altera_avalon_pio_regs.h"
#include "sys/alt_irq.h"
#include <stdlib.h>

#define OFFSET -32
#define PWM_PERIOD 16
#define TAPS 5 // 5-tap filter

alt_8 pwm = 0;
alt_u8 led;
int level;

// Simple 5-tap moving average filter
float LPF(alt_32 acc_read[TAPS]) {
    float avg = 0;
    float coeff = 0.2; // Coefficients all (0.2)

    // Calculate the moving average
    for (int i = 0; i < TAPS; i++) {
        avg += acc_read[i] * coeff;
    }

    // Return the filtered value
    return avg;
}

void led_write(alt_u8 led_pattern) {
    IOWR(LED_BASE, 0, led_pattern);
}

void convert_read(alt_32 acc_read, int * level, alt_u8 * led) {
    acc_read += OFFSET;
    alt_u8 val = (acc_read >> 6) & 0x07;
    *led = (8 >> val) | (8 << (8 - val));
    *level = (acc_read >> 1) & 0x1f;
}

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

void timer_init(void * isr) {
    IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_BASE, 0x0003);
    IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_BASE, 0);
    IOWR_ALTERA_AVALON_TIMER_PERIODL(TIMER_BASE, 0x0900);
    IOWR_ALTERA_AVALON_TIMER_PERIODH(TIMER_BASE, 0x0000);
    alt_irq_register(TIMER_IRQ, 0, isr);
    IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_BASE, 0x0007);
}

int main() {
    int i = 0;
    alt_32 x_read[TAPS] = {0};  // Initialise array to hold 5 accelerometer readings

    alt_up_accelerometer_spi_dev * acc_dev;
    acc_dev = alt_up_accelerometer_spi_open_dev("/dev/accelerometer_spi");
    if (acc_dev == NULL) {
        return 1;
    }

    timer_init(sys_timer_isr);

    while (1) {
        if (i == TAPS) {
            i = 0;  // Reset index
        }

        alt_up_accelerometer_spi_read_x_axis(acc_dev, &x_read[i]);

        // Apply the LPF
        float filtered_value = LPF(x_read);

        convert_read(filtered_value, &level, &led);

        i++;
    }

    return 0;
}
