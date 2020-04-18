#include <bcm2835.h>
#include <time.h>
#include "pins.h"
#include "c011.h"
#include <stdio.h>


#define TCSLCSH (60)
#define TCSHCSL (50)
#define TCSLDrV (50)

static void sleep_ns(int ns) {
    struct timespec s = {0,ns};
    int ret = nanosleep(&s,NULL);
    if (ret != 0) {
        printf ("nanosleep(%d) failed\n", ret);
    }
}

static void set_control_output_pins(void) {
    bcm2835_gpio_fsel(RS0, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(RS1, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(RESET, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(CS, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(RW, BCM2835_GPIO_FSEL_OUTP);
}

static void set_data_output_pins(void) {
    //bits 9-0 output (001)
    //%00001001001001001001001001001001
    //    0   9   2   4   9   2   4   9
    uint32_t word = 0x09249249;
    bcm2835_peri_write (bcm2835_regbase(BCM2835_REGBASE_GPIO) + BCM2835_GPFSEL0/4, word);
}

static void set_data_input_pins(void) {
    //bits 9-0 input (000)
    uint32_t word = 0;
    bcm2835_peri_write (bcm2835_regbase(BCM2835_REGBASE_GPIO) + BCM2835_GPFSEL0/4, word);
}
        
void c011_init(void) {
    bcm2835_init();
    set_control_output_pins();
    bcm2835_gpio_write(ANALYSE, LOW);
}

void c011_reset(void) {
    bcm2835_gpio_write(ANALYSE, LOW);
    //TN29 states "Recommended pulse width is 5 ms, with a delay of 5 ms before sending anything down a link."
    bcm2835_gpio_write(RESET, LOW);
    bcm2835_gpio_write(RESET, HIGH);
    bcm2835_delayMicroseconds (5*1000);
    bcm2835_gpio_write(RESET, LOW);
    bcm2835_delayMicroseconds (5*1000);
    //The whitecross HSL takes some time to cascade reset
    bcm2835_delay(600);
}

void c011_analyse(void) {
    bcm2835_gpio_write(ANALYSE, LOW);
    bcm2835_delayMicroseconds (5*1000);
    bcm2835_gpio_write(ANALYSE, HIGH);
    bcm2835_delayMicroseconds (5*1000);
    bcm2835_gpio_write(RESET, HIGH);
    bcm2835_delayMicroseconds (5*1000);
    bcm2835_gpio_write(RESET, LOW);
    bcm2835_delayMicroseconds (5*1000);
    bcm2835_gpio_write(ANALYSE, LOW);
    bcm2835_delayMicroseconds (5*1000);
}

int c011_write_byte(uint8_t byte, uint32_t timeout) {
    //wait for output ready
    uint64_t timeout_us = timeout*1000;
    while ((c011_read_output_status() & 0x01) != 0x01 && timeout_us>0) {
        bcm2835_delayMicroseconds(1);
        timeout_us--;
    }
    if (timeout_us == 0) {
        return -1;
    }
    //RS1=0, RS0=1
    //RW=0
    //CS=1
    set_data_output_pins();
    bcm2835_gpio_write_mask (0<<RS1 | 1<<RS0 | 0<<RW | 1<<CS,
                             1<<RS1 | 1<<RS0 | 1<<RW | 1<<CS);
    //D0-D7
    uint8_t d7,d6,d5,d4,d3,d2,d1,d0;
    d7=(byte&0x80) >> 7;
    d6=(byte&0x40) >> 6;
    d5=(byte&0x20) >> 5;
    d4=(byte&0x10) >> 4;
    d3=(byte&0x08) >> 3;
    d2=(byte&0x04) >> 2;
    d1=(byte&0x02) >> 1;
    d0=(byte&0x01) >> 0;
    bcm2835_gpio_write_mask (d7<<D7 | d6<<D6 | d5<<D5 | d4<<D4 | d3<<D3 | d2<<D2 | d1<<D1 | d0<<D0,
                             1<<D7 | 1<<D6 | 1<<D5 | 1<<D4 | 1<<D3 | 1<<D2 | 1<<D1 | 1<<D0);
    //CS=0
    bcm2835_gpio_write(CS, LOW);
    sleep_ns (TCSLCSH);
    //CS=1
    bcm2835_gpio_write(CS, HIGH);
    sleep_ns (TCSHCSL);
    return 0;
}

static uint8_t read_c011(void) {
    set_data_input_pins();
    bcm2835_gpio_write(CS, LOW);
    //must allow time for data valid after !CS
    sleep_ns (TCSLDrV);
    uint32_t reg = bcm2835_peri_read (bcm2835_regbase(BCM2835_REGBASE_GPIO) + BCM2835_GPLEV0/4);
    uint8_t byte;
    reg >>= 2;
    byte = reg & 0xFF;
    bcm2835_gpio_write(CS, HIGH);
    sleep_ns (TCSHCSL);
    return byte;
}

uint8_t c011_read_input_status(void) {
    uint8_t byte;
    bcm2835_gpio_write_mask (1<<RS1 | 0<<RS0 | 1<<RW | 1<<CS,
                             1<<RS1 | 1<<RS0 | 1<<RW | 1<<CS);
    byte = read_c011();
    return byte;
}

uint8_t c011_read_output_status(void) {
    uint8_t byte;
    bcm2835_gpio_write_mask (1<<RS1 | 1<<RS0 | 1<<RW | 1<<CS,
                             1<<RS1 | 1<<RS0 | 1<<RW | 1<<CS);
    byte = read_c011();
    return byte;
}

int c011_read_byte(uint8_t *byte, uint32_t timeout) {
    uint64_t timeout_us = timeout*1000;
    while ((c011_read_input_status() & 0x01) == 0x00 && timeout_us>0) {
        bcm2835_delayMicroseconds(1);
        timeout_us--;
    }
    if (timeout_us == 0) {
        return -1;
    }
    bcm2835_gpio_write_mask (0<<RS1 | 0<<RS0 | 1<<RW | 1<<CS,
                             1<<RS1 | 1<<RS0 | 1<<RW | 1<<CS);
    *byte = read_c011();
    return 0;
}

uint32_t c011_write_bytes (uint8_t *bytes, uint32_t num, uint32_t timeout) {
    uint32_t i;
    for (i=0; i < num; i++) {
        int ret = c011_write_byte(bytes[i], timeout);
        if (ret == -1) {
            break;
        }
    }
    return i;
}

uint32_t c011_read_bytes (uint8_t *bytes, uint32_t num, uint32_t timeout) {
    uint32_t i;
    for (i=0; i < num; i++) {
        int ret = c011_read_byte(&bytes[i], timeout);
        if (ret == -1) {
            break;
        }
    }
    return i;
}

