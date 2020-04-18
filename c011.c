#include <bcm2835.h>
#include <time.h>
#include "pins.h"
#include "c011.h"
#include <stdio.h>


#define TCSLCSH (60)
#define TCSHCSL (50)
#define TCSLDrV (50)

static uint32_t bits=0;

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

void set_gpio_bit(uint8_t pin, uint8_t on) {
    if (on) {
        bits |= 1<<pin;
    } else {
        bits &= ~(1<<pin);
    }
}

void gpio_commit(void) {
    bcm2835_peri_write (bcm2835_regbase(BCM2835_REGBASE_GPIO) + BCM2835_GPCLR0/4, ~bits);
    bcm2835_peri_write (bcm2835_regbase(BCM2835_REGBASE_GPIO) + BCM2835_GPSET0/4, bits);
}
        
void c011_init(void) {
    bcm2835_init();
    set_control_output_pins();
    set_gpio_bit(ANALYSE, LOW);
    gpio_commit();
}

void c011_reset(void) {
    set_gpio_bit(ANALYSE, LOW);
    gpio_commit();
    //TN29 states "Recommended pulse width is 5 ms, with a delay of 5 ms before sending anything down a link."
    set_gpio_bit(RESET, LOW);
    gpio_commit();
    set_gpio_bit(RESET, HIGH);
    gpio_commit();
    bcm2835_delayMicroseconds (5*1000);
    set_gpio_bit(RESET, LOW);
    gpio_commit();
    bcm2835_delayMicroseconds (5*1000);
    //The whitecross HSL takes some time to cascade reset
    bcm2835_delay(600);
}

void c011_analyse(void) {
    set_gpio_bit(ANALYSE, LOW);
    gpio_commit();
    bcm2835_delayMicroseconds (5*1000);
    set_gpio_bit(ANALYSE, HIGH);
    gpio_commit();
    bcm2835_delayMicroseconds (5*1000);
    set_gpio_bit(RESET, HIGH);
    gpio_commit();
    bcm2835_delayMicroseconds (5*1000);
    set_gpio_bit(RESET, LOW);
    gpio_commit();
    bcm2835_delayMicroseconds (5*1000);
    set_gpio_bit(ANALYSE, LOW);
    gpio_commit();
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
    set_gpio_bit (RS1,0);
    set_gpio_bit (RS0,1);
    set_gpio_bit (RW,0);
    set_gpio_bit (CS,1);
    gpio_commit();
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
    set_gpio_bit (D7,d7);
    set_gpio_bit (D6,d6);
    set_gpio_bit (D5,d5);
    set_gpio_bit (D4,d4);
    set_gpio_bit (D3,d3);
    set_gpio_bit (D2,d2);
    set_gpio_bit (D1,d1);
    set_gpio_bit (D0,d0);
    gpio_commit();
    //CS=0
    set_gpio_bit(CS, LOW);
    gpio_commit();
    sleep_ns (TCSLCSH);
    //CS=1
    set_gpio_bit(CS, HIGH);
    gpio_commit();
    sleep_ns (TCSHCSL);
    return 0;
}

static uint8_t read_c011(void) {
    set_data_input_pins();
    set_gpio_bit(CS, LOW);
    gpio_commit();
    //must allow time for data valid after !CS
    sleep_ns (TCSLDrV);
    uint32_t reg = bcm2835_peri_read (bcm2835_regbase(BCM2835_REGBASE_GPIO) + BCM2835_GPLEV0/4);
    uint8_t byte;
    reg >>= 2;
    byte = reg & 0xFF;
    set_gpio_bit(CS, HIGH);
    gpio_commit();
    sleep_ns (TCSHCSL);
    return byte;
}

uint8_t c011_read_input_status(void) {
    uint8_t byte;
    set_gpio_bit (RS1,1);
    set_gpio_bit (RS0,0);
    set_gpio_bit (RW,1);
    set_gpio_bit (CS,1);
    gpio_commit();
    byte = read_c011();
    return byte;
}

uint8_t c011_read_output_status(void) {
    uint8_t byte;
    set_gpio_bit (RS1,1);
    set_gpio_bit (RS0,1);
    set_gpio_bit (RW,1);
    set_gpio_bit (CS,1);
    gpio_commit();
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
    set_gpio_bit (RS1,0);
    set_gpio_bit (RS0,0);
    set_gpio_bit (RW,1);
    set_gpio_bit (CS,1);
    gpio_commit();
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

